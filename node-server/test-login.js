#!/usr/bin/env node

// Simple login test

const io = require('socket.io-client');

const socket = io('http://localhost:3000');

socket.on('connect', () => {
  console.log('✓ Connected');
  
  // Register first
  console.log('\n1. Registering user "player1"...');
  socket.emit('client-message', {
    cmd: 'REGISTER',
    payload: {
      username: 'player1',
      password: 'pass123'
    }
  });
  
  setTimeout(() => {
    console.log('\n2. Logging in as "player1"...');
    socket.emit('client-message', {
      cmd: 'LOGIN',
      payload: {
        username: 'player1',
        password: 'pass123'
      }
    });
  }, 1000);
});

socket.on('server-message', (data) => {
  console.log('\n📨 Server:', JSON.stringify(data, null, 2));
  
  if (data.cmd === 'LOGIN_SUCCESS') {
    console.log('\n✅ SUCCESS! Login worked!');
    process.exit(0);
  }
});

socket.on('error', console.error);

setTimeout(() => {
  console.log('\n⏱️  Timeout');
  process.exit(1);
}, 5000);
