import 'dotenv/config';
import express from 'express';
import http from 'http';
import { Server } from 'socket.io';
import net from 'net';

const app = express();
const httpServer = http.createServer(app);

const io = new Server(httpServer, {
  pingTimeout: 60000,
  pingInterval: 25000,
  connectTimeout: 45000
});

// C Client configuration (localhost only)
// Usage: C_CLIENT_PORT=9001 node server.js
const C_CLIENT_HOST = 'localhost';
const C_CLIENT_PORT = parseInt(process.env.C_CLIENT_PORT || 9000);
const NODE_SERVER_PORT = parseInt(process.env.NODE_SERVER_PORT || 3000);

console.log(`🔌 Connecting to C Client: ${C_CLIENT_HOST}:${C_CLIENT_PORT}`);
console.log(`🌐 Node Server Port: ${NODE_SERVER_PORT}`);

// Store active TCP connections to C client
const tcpClients = new Map();
const messageBuffers = new Map();

// Socket.IO connection handling
io.on('connection', (socket) => {
  console.log(`Browser client ${socket.id} connected`);

  // Create TCP connection to C client
  const tcpClient = new net.Socket();
  tcpClients.set(socket.id, tcpClient);
  messageBuffers.set(socket.id, '');

  tcpClient.connect(C_CLIENT_PORT, C_CLIENT_HOST, () => {
    console.log(`TCP connected to C client for ${socket.id}`);
  });

  // Nhận dữ liệu từ C client (forwarded from C++ server)
  tcpClient.on('data', (data) => {
    const dataStr = data.toString();
    let buffer = messageBuffers.get(socket.id) + dataStr;
    
    // Split by newlines
    const messages = buffer.split('\n');
    messageBuffers.set(socket.id, messages.pop() || '');
    
    // Process complete messages
    messages.forEach((msg) => {
      if (msg.trim()) {
        try {
          const jsonMsg = JSON.parse(msg);
          console.log(`[${socket.id}] C++ → Browser:`, jsonMsg.cmd || 'DATA');
          socket.emit('server-message', jsonMsg);
        } catch (e) {
          console.error(`[${socket.id}] JSON parse error:`, msg);
        }
      }
    });
  });

  // TCP connection closed
  tcpClient.on('close', () => {
    console.log(`TCP connection closed for ${socket.id}`);
    socket.emit('server-disconnected', { message: 'Connection to game server closed' });
  });

  // TCP connection error
  tcpClient.on('error', (err) => {
    console.error(`TCP error for ${socket.id}:`, err.message);
    socket.emit('server-error', { error: err.message });
  });

  // Handle messages from browser → C client → C++ server
  socket.on('client-message', (data) => {
    console.log(`[${socket.id}] Browser → C++:`, data.cmd || JSON.stringify(data).substring(0, 50));
    
    if (tcpClient && !tcpClient.destroyed) {
      const message = JSON.stringify(data) + '\n';
      tcpClient.write(message);
    } else {
      console.error(`TCP client not available for ${socket.id}`);
      socket.emit('server-message', {
        cmd: 'ERROR',
        payload: { message: 'Not connected to server' }
      });
    }
  });

  // Browser disconnect
  socket.on('disconnect', () => {
    console.log(`Browser client disconnected: ${socket.id}`);
    
    const client = tcpClients.get(socket.id);
    if (client && !client.destroyed) {
      client.destroy();
    }
    
    tcpClients.delete(socket.id);
    messageBuffers.delete(socket.id);
  });
});

// Start the server
httpServer.listen(NODE_SERVER_PORT, 'localhost', () => {
  console.log(`Node.js server running on http://localhost:${NODE_SERVER_PORT}`);
  console.log(`Connecting to C Client: ${C_CLIENT_HOST}:${C_CLIENT_PORT}`);
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\n\nShutting down gracefully...');
  tcpClients.forEach((client) => {
    if (!client.destroyed) {
      client.destroy();
    }
  });
  httpServer.close(() => {
    console.log('Server closed');
    process.exit(0);
  });
});
