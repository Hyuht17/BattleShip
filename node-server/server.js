const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');
const net = require('net');

const app = express();
const httpServer = http.createServer(app);
const io = new Server(httpServer, {
  cors: {
    origin: [
      "http://localhost:5173",     // Vite frontend
      "http://127.0.0.1:5173",     // Vite alternative
      "http://localhost:5500",     // Live Server
      "http://127.0.0.1:5500",     // Live Server alternative
      "http://localhost:3001",     // Development
      "null"                       // For file:// protocol
    ],
    methods: ["GET", "POST"],
    credentials: true
  }
});

app.use(cors({
  origin: [
    "http://localhost:5173",
    "http://127.0.0.1:5173",
    "http://localhost:5500",
    "http://127.0.0.1:5500",
    "http://localhost:3001"
  ],
  credentials: true
}));
app.use(express.json());

const CPP_SERVER_HOST = 'localhost';
const CPP_SERVER_PORT = 8080;
const NODE_SERVER_PORT = 3000;

// Store active connections
const connections = new Map();
const messageBuffers = new Map(); // Buffer for incomplete messages

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({ status: 'ok', message: 'Node.js server is running' });
});

// Socket.IO connection handling
io.on('connection', (socket) => {
  console.log(`[${new Date().toISOString()}] Client connected: ${socket.id}`);

  // Create TCP connection to C++ server
  const cppClient = new net.Socket();
  connections.set(socket.id, cppClient);
  messageBuffers.set(socket.id, '');

  // Set keepalive to detect disconnections
  cppClient.setKeepAlive(true, 5000);
  
  // Connect to C++ server
  cppClient.connect(CPP_SERVER_PORT, CPP_SERVER_HOST, () => {
    console.log(`[${new Date().toISOString()}] TCP connected to C++ server for client ${socket.id}`);
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
    console.error(`[${new Date().toISOString()}] C++ server error for client ${socket.id}:`, err.message);
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

// Start server
httpServer.listen(NODE_SERVER_PORT, () => {
  console.log('╔═══════════════════════════════════════╗');
  console.log('║   BattleShip Node.js Server Started! ║');
  console.log(`║   Port: ${NODE_SERVER_PORT}                           ║`);
  console.log(`║   C++ Server: ${CPP_SERVER_HOST}:${CPP_SERVER_PORT}           ║`);
  console.log('╚═══════════════════════════════════════╝');
  console.log(`Frontend URL: http://localhost:5173`);
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
