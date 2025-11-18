#!/usr/bin/env node

// Test full challenge flow: 2 players challenge and start game

const io = require('socket.io-client');

let player1Socket, player2Socket;
let player1Username = 'player_test1_' + Date.now();
let player2Username = 'player_test2_' + Date.now();

console.log('🎮 Testing Challenge Flow\n');

// Player 1
player1Socket = io('http://localhost:3000');

player1Socket.on('connect', () => {
  console.log('✓ Player 1 connected');
  
  // Register and login
  setTimeout(() => {
    console.log('\n[Player 1] Registering...');
    player1Socket.emit('client-message', {
      cmd: 'REGISTER',
      payload: { username: player1Username, password: 'test123' }
    });
  }, 500);
});

player1Socket.on('server-message', (data) => {
  console.log('[Player 1] <-', data.cmd, JSON.stringify(data.payload));
  
  if (data.cmd === 'REGISTER_SUCCESS' && data.payload) {
    console.log('[Player 1] Login...');
    player1Socket.emit('client-message', {
      cmd: 'LOGIN',
      payload: { username: player1Username, password: 'test123' }
    });
  } else if (data.cmd === 'LOGIN_SUCCESS') {
    console.log('[Player 1] ✅ Logged in as:', data.payload.username);
    
    // Start player 2
    startPlayer2();
  } else if (data.cmd === 'CHALLENGE') {
    console.log('[Player 1] 🎯 Received challenge from:', data.payload.challenger);
    
    // Auto-accept the challenge
    setTimeout(() => {
      console.log('[Player 1] ✓ Accepting challenge...');
      player1Socket.emit('client-message', {
        cmd: 'CHALLENGE_REPLY',
        payload: {
          challenger_username: data.payload.challenger,
          status: 'ACCEPT'
        }
      });
    }, 500);
  } else if (data.cmd === 'GAME_START') {
    console.log('[Player 1] 🎮 GAME STARTED!');
    console.log('           Opponent:', data.payload.opponent);
    console.log('           Your turn:', data.payload.your_turn);
    console.log('\n✅ SUCCESS! Challenge flow worked!');
    process.exit(0);
  }
});

function startPlayer2() {
  setTimeout(() => {
    console.log('\n--- Starting Player 2 ---\n');
    
    player2Socket = io('http://localhost:3000');
    
    player2Socket.on('connect', () => {
      console.log('✓ Player 2 connected');
      
      setTimeout(() => {
        console.log('[Player 2] Registering...');
        player2Socket.emit('client-message', {
          cmd: 'REGISTER',
          payload: { username: player2Username, password: 'test123' }
        });
      }, 500);
    });
    
    player2Socket.on('server-message', (data) => {
      console.log('[Player 2] <-', data.cmd, JSON.stringify(data.payload));
      
      if (data.cmd === 'REGISTER_SUCCESS') {
        console.log('[Player 2] Login...');
        player2Socket.emit('client-message', {
          cmd: 'LOGIN',
          payload: { username: player2Username, password: 'test123' }
        });
      } else if (data.cmd === 'LOGIN_SUCCESS') {
        console.log('[Player 2] ✅ Logged in as:', data.payload.username);
        
        // Challenge player 1
        setTimeout(() => {
          console.log('\n[Player 2] 🎯 Sending challenge to Player 1...');
          player2Socket.emit('client-message', {
            cmd: 'CHALLENGE',
            payload: { target_username: player1Username }
          });
        }, 1000);
      } else if (data.cmd === 'CHALLENGE') {
        console.log('[Player 2] 🎯 Received challenge from:', data.payload.challenger);
        
        // Auto-accept after 1 second
        setTimeout(() => {
          console.log('[Player 2] ✓ Accepting challenge...');
          player2Socket.emit('client-message', {
            cmd: 'CHALLENGE_REPLY',
            payload: {
              challenger_username: data.payload.challenger,
              status: 'ACCEPT'
            }
          });
        }, 1000);
      } else if (data.cmd === 'GAME_START') {
        console.log('[Player 2] 🎮 GAME STARTED!');
        console.log('           Opponent:', data.payload.opponent);
        console.log('           Your turn:', data.payload.your_turn);
      }
    });
  }, 2000);
}

// Timeout
setTimeout(() => {
  console.log('\n⏱️  Timeout - Test failed');
  process.exit(1);
}, 15000);
