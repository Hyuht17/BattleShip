import express from 'express';
import http from 'http';
import { Server } from 'socket.io';
import net from 'net';
import cors from 'cors';
import os from 'os';

const app = express();
const httpServer = http.createServer(app);

// ✅ Lấy IP của server trong LAN
function getLocalIP() {
  const interfaces = os.networkInterfaces();
  for (const devName in interfaces) {
    const iface = interfaces[devName];
    for (let i = 0; i < iface.length; i++) {
      const alias = iface[i];
      if (alias.family === 'IPv4' && !alias.internal) {
        return alias.address;
      }
    }
  }
  return 'localhost';
}

const LOCAL_IP = getLocalIP();
console.log(`🌐 Server LAN IP: ${LOCAL_IP}`);

// CORS: Cho phép browsers từ mọi IP kết nối WebSocket đến Node.js
// Node.js server chỉ kết nối đến 1 C++ server (qua CPP_SERVER_HOST)
// Không có kết nối giữa các Node.js servers với nhau
const io = new Server(httpServer, {
  cors: {
    origin: true,  // Cho phép tất cả origins (dev mode)
    methods: ["GET", "POST"],
    credentials: true
  }
});

app.use(cors({
  origin: true,  // Cho phép tất cả (dev mode)
  credentials: true
}));
app.use(express.json());

// C++ Server configuration
// Nếu Node.js và C++ server cùng máy: dùng 'localhost'
// Nếu C++ server ở máy khác trong LAN: thay 'localhost' bằng IP (ví dụ: '192.168.1.100')
const CPP_SERVER_HOST = process.env.CPP_SERVER_HOST || 'localhost';
const CPP_SERVER_PORT = parseInt(process.env.CPP_SERVER_PORT || '8080');
const NODE_SERVER_PORT = parseInt(process.env.NODE_SERVER_PORT || '3000');

console.log(`📡 C++ Server: ${CPP_SERVER_HOST}:${CPP_SERVER_PORT}`);

// Store active connections
const connections = new Map();
const messageBuffers = new Map(); // Buffer for incomplete messages

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({ status: 'ok', message: 'Node.js server is running' });
});

// Socket.IO connection handling
// MỖI CLIENT tạo 1 WebSocket connection đến Node.js
// Node.js tạo 1 TCP connection tương ứng đến C++ server
// Clients KHÔNG kết nối với nhau, chỉ kết nối đến C++ server qua Node.js
io.on('connection', (socket) => {
  console.log(`[${new Date().toISOString()}] Client ${socket.id} connected from ${socket.handshake.address}`);

  // Create DEDICATED TCP connection to C++ server for THIS client
  // Mỗi client có 1 TCP connection riêng đến C++ server
  const cppClient = new net.Socket();
  connections.set(socket.id, cppClient);
  messageBuffers.set(socket.id, '');

  // Set keepalive to detect disconnections
  cppClient.setKeepAlive(true, 5000);
  
  // Connect to C++ server (all clients connect to SAME C++ server)
  // Tất cả clients đều kết nối đến CÙNG 1 C++ server
  console.log(`[${new Date().toISOString()}] Attempting TCP connection to ${CPP_SERVER_HOST}:${CPP_SERVER_PORT} for client ${socket.id}`);
  
  cppClient.connect(CPP_SERVER_PORT, CPP_SERVER_HOST, () => {
    console.log(`[${new Date().toISOString()}] ✅ TCP connected to C++ server for client ${socket.id}`);
    socket.emit('server-connected', { message: 'Connected to game server' });
  });

  // Handle data from C++ server
  cppClient.on('data', (data) => {
    const dataStr = data.toString();
    console.log(`[${new Date().toISOString()}] Data from C++ server for ${socket.id}:`, dataStr);
    
    // Append to buffer
    let buffer = messageBuffers.get(socket.id) + dataStr;
    
    // Split by newlines to handle multiple messages
    const messages = buffer.split('\n');
    
    // Keep the last incomplete message in buffer
    messageBuffers.set(socket.id, messages.pop() || '');
    
    // Process complete messages
    messages.forEach((msg) => {
      if (msg.trim()) {
        try {
          // Try to parse as JSON
          const jsonMsg = JSON.parse(msg);
          socket.emit('server-message', jsonMsg);
        } catch (e) {
          // If not JSON, send as is
          socket.emit('server-message', { raw: msg });
        }
      }
    });
  });

  // Handle C++ server errors
  cppClient.on('error', (err) => {
    console.error(`[${new Date().toISOString()}] ❌ C++ server error for client ${socket.id}:`);
    console.error(`   Error code: ${err.code}`);
    console.error(`   Error message: ${err.message}`);
    console.error(`   Tried to connect to: ${CPP_SERVER_HOST}:${CPP_SERVER_PORT}`);
    socket.emit('server-error', { error: err.message });
  });

  // Handle C++ server disconnection
  cppClient.on('close', () => {
    console.log(`[${new Date().toISOString()}] C++ server connection closed for client ${socket.id}`);
    socket.emit('server-disconnected', { message: 'Disconnected from game server' });
  });

  // Handle connection timeout
  cppClient.setTimeout(30000); // 30 second timeout
  cppClient.on('timeout', () => {
    console.log(`[${new Date().toISOString()}] Connection timeout for client ${socket.id}`);
    cppClient.destroy();
  });

  // Handle messages from frontend client
  socket.on('client-message', (data) => {
    console.log(`[${new Date().toISOString()}] Message from client ${socket.id}:`, JSON.stringify(data));
    if (cppClient && !cppClient.destroyed) {
      const message = JSON.stringify(data) + '\n';
      cppClient.write(message);
    } else {
      socket.emit('error', { message: 'Not connected to game server' });
    }
  });

  // Handle specific game commands
  socket.on('register', (data) => {
    const msg = { cmd: 'REGISTER', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('login', (data) => {
    const msg = { cmd: 'LOGIN', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('get-players', () => {
    const msg = { cmd: 'PLAYER_LIST', payload: {} };
    socket.emit('client-message', msg);
  });

  socket.on('challenge', (data) => {
    const msg = { cmd: 'CHALLENGE', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('challenge-reply', (data) => {
    const msg = { cmd: 'CHALLENGE_REPLY', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('place-ships', (data) => {
    const msg = { cmd: 'PLACE_SHIPS', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('make-move', (data) => {
    const msg = { cmd: 'MOVE', payload: data };
    socket.emit('client-message', msg);
  });

  socket.on('chat', (data) => {
    const msg = { cmd: 'CHAT', payload: data };
    socket.emit('client-message', msg);
  });

  // Handle client disconnection
  socket.on('disconnect', () => {
    console.log(`[${new Date().toISOString()}] Client disconnected: ${socket.id}`);
    const connection = connections.get(socket.id);
    if (connection && !connection.destroyed) {
      connection.destroy();
    }
    connections.delete(socket.id);
    messageBuffers.delete(socket.id);
  });
});

// Start server - Only listen on localhost (not LAN)
// Chỉ listen localhost, không public ra LAN
httpServer.listen(NODE_SERVER_PORT, 'localhost', () => {
  console.log(`✅ Node.js server listening on:`);
  console.log(`   - Local: http://localhost:${NODE_SERVER_PORT}`);
  console.log(`   📡 Connecting to C++ Server: ${CPP_SERVER_HOST}:${CPP_SERVER_PORT}`);
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\n\nShutting down gracefully...');
  connections.forEach((connection) => {
    if (!connection.destroyed) {
      connection.destroy();
    }
  });
  httpServer.close(() => {
    console.log('Server closed');
    process.exit(0);
  });
});
