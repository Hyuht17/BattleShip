import { useEffect, useState, useRef } from 'react';
import { io } from 'socket.io-client';
import './App.css';
import LoginScreen from './components/LoginScreen';
import LobbyScreen from './components/LobbyScreen';
import GameScreen from './components/GameScreen';

const SOCKET_SERVER_URL = 'http://localhost:3000';

function App() {
  const [screen, setScreen] = useState('login'); // 'login', 'lobby', 'game'
  const [connected, setConnected] = useState(false);
  const [user, setUser] = useState(null);
  const [gameState, setGameState] = useState(null);
  const socketRef = useRef(null);

  useEffect(() => {
    // Initialize socket connection
    socketRef.current = io(SOCKET_SERVER_URL, {
      transports: ['websocket', 'polling'],
      reconnection: true,
      reconnectionDelay: 1000,
      reconnectionAttempts: 5
    });

    // Connection events
    socketRef.current.on('connect', () => {
      console.log('Connected to Node.js server');
      setConnected(true);
    });

    socketRef.current.on('server-connected', (data) => {
      console.log('Connected to C++ game server:', data);
    });

    socketRef.current.on('disconnect', () => {
      console.log('❌ Disconnected from server');
      setConnected(false);
    });

    socketRef.current.on('server-error', (data) => {
      console.error('Server error:', data);
      alert('Server error: ' + data.error);
    });

    // Game events
    socketRef.current.on('server-message', (data) => {
      console.log('Message from server:', data);
      handleServerMessage(data);
    });

    return () => {
      if (socketRef.current) {
        socketRef.current.disconnect();
      }
    };
  }, []);

  const handleServerMessage = (data) => {
    console.log('[SERVER MESSAGE]', data);
    const { cmd, payload } = data;

    switch (cmd) {
      case 'LOGIN_SUCCESS':
        console.log('[LOGIN SUCCESS] User:', payload.username);
        setUser({ username: payload.username });
        setScreen('lobby');
        break;

      case 'REGISTER_SUCCESS':
        console.log('[REGISTER SUCCESS]');
        alert('Registration successful! Please login.');
        break;

      case 'GAME_START':
        console.log('[GAME_START] Opponent:', payload.opponent, 'Your turn:', payload.your_turn);
        setGameState({
          opponent: payload.opponent,
          yourTurn: payload.your_turn,
          phase: 'placing',
          myBoard: Array(10).fill(null).map(() => Array(10).fill(0)),
          opponentBoard: Array(10).fill(null).map(() => Array(10).fill(0)),
          myShips: [],
          opponentShips: []
        });
        console.log('[GAME_START] Switching to game screen');
        setScreen('game');
        break;

      case 'GAME_READY':
        setGameState(prev => ({ ...prev, phase: 'playing' }));
        break;

      case 'MOVE_RESULT':
        handleMoveResult(payload);
        break;

      case 'TURN_CHANGE':
        setGameState(prev => ({ ...prev, yourTurn: payload.your_turn }));
        break;

      case 'GAME_END':
        handleGameEnd(payload);
        break;

      case 'CHALLENGE':
        handleChallenge(payload);
        break;

      case 'SYSTEM_MSG':
        if (payload.code !== 200) {
          alert(payload.message);
        }
        break;

      default:
        console.log('Unhandled command:', cmd, payload);
    }
  };

  const handleMoveResult = (payload) => {
    const { coord, result, ship_sunk } = payload;
    const col = parseInt(coord.substring(1));
    const row = coord.charCodeAt(0) - 65;

    setGameState(prev => {
      const newState = { ...prev };
      if (prev.yourTurn) {
        // Update opponent board
        newState.opponentBoard = prev.opponentBoard.map(r => [...r]);
        newState.opponentBoard[row][col] = result === 'HIT' ? 2 : 3;
      } else {
        // Update my board
        newState.myBoard = prev.myBoard.map(r => [...r]);
        if (newState.myBoard[row][col] === 1) {
          newState.myBoard[row][col] = 2; // hit
        }
      }
      return newState;
    });

    if (ship_sunk) {
      alert(`${ship_sunk} was sunk!`);
    }
  };

  const handleGameEnd = (payload) => {
    const message = payload.result === 'WIN' ? 
      `You won! ${payload.reason}` : 
      `You lost. ${payload.reason}`;
    
    alert(message);
    setScreen('lobby');
    setGameState(null);
  };

  const handleChallenge = (payload) => {
    console.log('[CHALLENGE RECEIVED]', payload);
    const accept = window.confirm(`${payload.challenger} challenges you to a game. Accept?`);
    console.log('[CHALLENGE REPLY]', accept ? 'ACCEPT' : 'REJECT');
    sendMessage({
      cmd: 'CHALLENGE_REPLY',
      payload: {
        challenger_username: payload.challenger,
        status: accept ? 'ACCEPT' : 'REJECT'
      }
    });
  };

  const sendMessage = (message) => {
    console.log('[SEND MESSAGE]', message, 'Connected:', connected, 'Socket:', !!socketRef.current);
    if (socketRef.current) {
      // Check actual socket connection state instead of React state
      if (socketRef.current.connected) {
        socketRef.current.emit('client-message', message);
        console.log('[SEND MESSAGE] Message sent successfully');
      } else {
        console.error('[SEND MESSAGE] Failed - Socket not connected. Actual state:', socketRef.current.connected);
      }
    } else {
      console.error('[SEND MESSAGE] Failed - Socket ref is null');
    }
  };

  const handleLogin = (username, password) => {
    console.log('[LOGIN] Attempting login:', username);
    sendMessage({
      cmd: 'LOGIN',
      payload: { username, password }
    });
  };

  const handleRegister = (username, password) => {
    sendMessage({
      cmd: 'REGISTER',
      payload: { username, password }
    });
  };

  return (
    <div className="app">
      <header className="app-header">
        <h1>🚢 BattleShip Network Game</h1>
        <div className={`status ${connected ? 'connected' : 'disconnected'}`}>
          {connected ? '🟢 Connected' : '🔴 Disconnected'}
        </div>
        {user && <div className="user-info">👤 {user.username}</div>}
      </header>

      <main className="app-main">
        {screen === 'login' && (
          <LoginScreen
            onLogin={handleLogin}
            onRegister={handleRegister}
            connected={connected}
          />
        )}

        {screen === 'lobby' && (
          <LobbyScreen
            socket={socketRef.current}
            sendMessage={sendMessage}
            user={user}
          />
        )}

        {screen === 'game' && gameState && (
          <GameScreen
            socket={socketRef.current}
            sendMessage={sendMessage}
            gameState={gameState}
            setGameState={setGameState}
            user={user}
          />
        )}
      </main>

      <footer className="app-footer">
        <p>Đồ án Lập trình mạng - BattleShip Game</p>
      </footer>
    </div>
  );
}

export default App;
