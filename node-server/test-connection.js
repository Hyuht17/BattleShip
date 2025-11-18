#!/usr/bin/env node

// Test script to check server communication

const io = require('socket.io-client');

const socket = io('http://localhost:3000', {
  transports: ['websocket', 'polling']
});

socket.on('connect', () => {
  console.log('✓ Connected to Node.js server');
  
  // Test register
  console.log('\n📝 Testing REGISTER...');
  socket.emit('client-message', {
    cmd: 'REGISTER',
    payload: {
      username: 'testuser' + Date.now(),
      password: 'test123'
    }
  });
});

socket.on('server-connected', (data) => {
  console.log('✓ Connected to C++ server:', data);
});

socket.on('server-message', (data) => {
  console.log('\n📨 Response from server:');
  console.log(JSON.stringify(data, null, 2));
  
  if (data.cmd === 'REGISTER_SUCCESS') {
    console.log('\n✓ REGISTER successful!');
    
    // Now test login
    console.log('\n🔐 Testing LOGIN...');
    socket.emit('client-message', {
      cmd: 'LOGIN',
      payload: {
        username: 'testuser',
        password: 'test123'
      }
    });
  } else if (data.cmd === 'LOGIN_SUCCESS') {
    console.log('\n✓ LOGIN successful!');
    console.log('Username:', data.payload.username);
    
    // Test getting player list
    console.log('\n📋 Testing PLAYER_LIST...');
    socket.emit('client-message', {
      cmd: 'PLAYER_LIST',
      payload: {}
    });
  } else if (data.cmd === 'PLAYER_LIST') {
    console.log('\n✓ PLAYER_LIST received!');
    console.log('Players:', data.payload.players);
    
    console.log('\n✅ All tests passed!');
    process.exit(0);
  } else if (data.cmd === 'SYSTEM_MSG') {
    console.log('\n⚠️  System message:', data.payload.message);
  }
});

socket.on('server-error', (data) => {
  console.error('\n❌ Server error:', data);
});

socket.on('disconnect', () => {
  console.log('\n❌ Disconnected from server');
});

socket.on('error', (error) => {
  console.error('\n❌ Socket error:', error);
});

// Timeout after 10 seconds
setTimeout(() => {
  console.log('\n⏱️  Timeout - closing connection');
  process.exit(1);
}, 10000);
