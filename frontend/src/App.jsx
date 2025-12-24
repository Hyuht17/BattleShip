import { useEffect, useState, useRef } from 'react';
import { io } from 'socket.io-client';
import './App.css';
import LoginScreen from './components/LoginScreen';
import LobbyScreen from './components/LobbyScreen';
import GameScreen from './components/GameScreen';
import { SOCKET_SERVER_URL } from './config';

function App() {
  const [screen, setScreen] = useState('login'); // 'login', 'lobby', 'game'
  const [connected, setConnected] = useState(false);
  const [user, setUser] = useState(null);
  const [gameState, setGameState] = useState(null);
  const [challengeRequest, setChallengeRequest] = useState(null); // {challenger: string}
  const [gameResult, setGameResult] = useState(null); // {result: 'WIN'|'LOSE', reason: string, opponent: string}
  const [drawRequest, setDrawRequest] = useState(null); // {from: username}
  const [matchFound, setMatchFound] = useState(null); // {opponent: string, elo: number}
  const [isMatching, setIsMatching] = useState(false);
  const socketRef = useRef(null);
  const opponentRef = useRef(null); // Store opponent name to avoid stale closure

  // Load saved user from localStorage on mount
  useEffect(() => {
    const savedUser = localStorage.getItem('battleship_user');
    if (savedUser) {
      try {
        const userData = JSON.parse(savedUser);
        setUser(userData);
        setScreen('lobby');
      } catch (e) {
        console.error('Failed to parse saved user:', e);
        localStorage.removeItem('battleship_user');
      }
    }
  }, []);

  useEffect(() => {
    console.log('Connecting to:', SOCKET_SERVER_URL);
    
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
      
      // Auto-login if user exists in localStorage
      const savedUser = localStorage.getItem('battleship_user');
      if (savedUser) {
        try {
          const userData = JSON.parse(savedUser);
          console.log('Auto-login with saved user:', userData.username);
          // Send LOGIN directly via socket
          socketRef.current.emit('client-message', {
            cmd: 'LOGIN',
            payload: { username: userData.username }
          });
        } catch (e) {
          console.error('Failed to auto-login:', e);
        }
      }
    });

    socketRef.current.on('server-connected', (data) => {
      console.log('Connected to C++ game server:', data);
    });

    socketRef.current.on('disconnect', () => {
      console.log('Disconnected from server');
      setConnected(false);
    });

    socketRef.current.on('server-error', (data) => {
      console.error('Server error:', data);
      alert('Server error: ' + data.error);
    });

    // Game events - Define handler inline to avoid stale closure
    const handleMessage = (data) => {
      const { cmd, payload } = data;

      switch (cmd) {
        case 'LOGIN_SUCCESS':
          console.log('[LOGIN SUCCESS] User:', payload.username, 'ELO:', payload.elo);
          const userData = { username: payload.username, elo: payload.elo || 800 };
          setUser(userData);
          // Save to localStorage
          localStorage.setItem('battleship_user', JSON.stringify(userData));
          setScreen('lobby');
          break;

        case 'REGISTER_SUCCESS':
          console.log('[REGISTER SUCCESS]');
          alert('Registration successful! Please login.');
          break;

        case 'GAME_START':
          console.log('[GAME_START] Opponent:', payload.opponent, 'Your turn:', payload.your_turn);
          opponentRef.current = payload.opponent; // Save opponent name
          setMatchFound(null); // Close match found modal
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
          handleMoveResultInline(payload);
          break;

        case 'TURN_CHANGE':
          setGameState(prev => ({ ...prev, yourTurn: payload.your_turn }));
          break;

        case 'GAME_END':
          handleGameEndInline(payload);
          break;

        case 'CHALLENGE':
          // Set challenge request to show custom modal
          setChallengeRequest({
            challenger: payload.challenger
          });
          
          // Try to focus window and play notification sound
          window.focus();
          try {
            const audio = new Audio('data:audio/wav;base64,UklGRnoGAABXQVZFZm10IBAAAAABAAEAQB8AAEAfAAABAAgAZGF0YQoGAACBhYqFbF1fdJivrJBhNjVgodDbq2EcBj+a2/LDciUFLIHO8tiJNwgZaLvt559NEAxQp+PwtmMcBjiR1/LMeSwFJHfH8N2QQAoUXrTp66hVFApGn+DyvmwhBTGH0fPTgjMGHm7A7+OZURE');
            audio.play().catch(() => {});
          } catch (e) {}
          break;

        case 'SYSTEM_MSG':
          if (payload.code !== 200) {
            alert(payload.message);
          }
          break;

        case 'DRAW_OFFER':
          console.log('[DRAW_OFFER] Received from:', payload.from);
          setDrawRequest({ from: payload.from });
          break;

        case 'DRAW_REJECTED':
          console.log('[DRAW_REJECTED]');
          alert('Đối thủ từ chối đề nghị hòa');
          break;

        case 'MATCHING_STARTED':
          console.log('[MATCHING_STARTED]', payload);
          setIsMatching(true);
          break;

        case 'MATCH_FOUND':
          console.log('[MATCH_FOUND]', payload);
          setIsMatching(false);
          setMatchFound({
            opponent: payload.opponent,
            elo: payload.elo
          });
          break;

        case 'OPPONENT_READY':
          console.log('[OPPONENT_READY]', payload);
          // Could show a notification
          break;

        case 'WAITING_OPPONENT':
          console.log('[WAITING_OPPONENT]', payload);
          // Already showing in modal
          break;

        case 'MATCH_DECLINED':
          console.log('[MATCH_DECLINED]', payload);
          setMatchFound(null);
          setIsMatching(false);
          alert(payload.message || 'Đối thủ đã từ chối trận đấu');
          break;

        case 'MATCHING_CANCELLED':
          console.log('[MATCHING_CANCELLED]', payload);
          setIsMatching(false);
          break;

        default:
          console.log('Unhandled command:', cmd, payload);
      }
    };

    const handleMoveResultInline = (payload) => {
      const { coord, result, ship_sunk, is_your_shot } = payload;
      const col = parseInt(coord.substring(1));
      const row = coord.charCodeAt(0) - 65;

      setGameState(prev => {
        if (!prev) return prev;
        const newState = { 
          ...prev,
          myShips: prev.myShips || [], // Ensure myShips is preserved
          opponentShips: prev.opponentShips || []
        };
        
        // Use is_your_shot field from server to determine which board to update
        if (is_your_shot === true || is_your_shot === undefined) {
          // This is your shot - update opponent board (tracking board)
          newState.opponentBoard = prev.opponentBoard.map(r => [...r]);
          if (result === 'HIT' || result === 'ALREADY_HIT') {
            newState.opponentBoard[row][col] = 2; // Hit
          } else if (result === 'MISS') {
            newState.opponentBoard[row][col] = 3; // Miss
          }
        } else {
          // This is opponent's shot - update your board (defensive board)
          newState.myBoard = prev.myBoard.map(r => [...r]);
          if (result === 'HIT' || result === 'ALREADY_HIT') {
            // Mark as hit (only if there's a ship there)
            if (newState.myBoard[row][col] === 1) {
              newState.myBoard[row][col] = 2; // Hit on ship
            } else {
              newState.myBoard[row][col] = 2; // Hit (already marked)
            }
          } else if (result === 'MISS') {
            // Mark as miss on your board
            if (newState.myBoard[row][col] === 0) {
              newState.myBoard[row][col] = 3; // Miss
            }
          }
        }
        return newState;
      });

      if (ship_sunk) {
        alert(`${ship_sunk} was sunk!`);
      }
    };

    const handleGameEndInline = (payload) => {
      // Show game result modal instead of alert
      const opponentName = opponentRef.current || 'Không rõ';
      console.log('[GAME_END] Opponent name:', opponentName);
      console.log('[GAME_END] Payload:', payload);
      
      // Update user ELO if provided
      if (payload.elo !== undefined) {
        setUser(prev => {
          const newUser = { ...prev, elo: payload.elo };
          // Update localStorage with new ELO
          localStorage.setItem('battleship_user', JSON.stringify(newUser));
          return newUser;
        });
      }
      
      let reason = payload.reason;
      if (payload.reason === 'OPPONENT_DISCONNECT') {
        reason = 'Đối thủ đã ngắt kết nối';
      } else if (payload.reason === 'SURRENDER') {
        reason = 'Đối thủ đã đầu hàng';
      } else if (payload.reason === 'DRAW_ACCEPTED') {
        reason = 'Hòa - Cả hai đồng ý';
      }

      setGameResult({
        result: payload.result,
        reason: reason,
        opponent: opponentName,
        elo: payload.elo // Add ELO to result modal
      });
    };

    socketRef.current.on('server-message', handleMessage);

    return () => {
      if (socketRef.current) {
        socketRef.current.off('server-message', handleMessage);
        socketRef.current.disconnect();
      }
    };
  }, []);

  // These functions are no longer needed as they're defined inline in useEffect

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

  const handleChallengeResponse = (accept) => {
    if (!challengeRequest) return;
    
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', {
        cmd: 'CHALLENGE_REPLY',
        payload: {
          challenger_username: challengeRequest.challenger,
          status: accept ? 'ACCEPT' : 'REJECT'
        }
      });
    }
    
    setChallengeRequest(null);
  };

  const handleDrawReply = (accept) => {
    if (!drawRequest) return;
    
    console.log('[DRAW_REPLY] Sending:', accept ? 'accept' : 'reject');
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', {
        cmd: 'DRAW_REPLY',
        payload: {
          status: accept ? 'accept' : 'reject'
        }
      });
    }
    
    setDrawRequest(null);
  };

  const handleMatchReady = () => {
    console.log('[MATCH_READY] Sending ready confirmation');
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', {
        cmd: 'MATCH_READY',
        payload: {}
      });
    }
  };

  const handleMatchDecline = () => {
    console.log('[MATCH_DECLINE] Declining match');
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', {
        cmd: 'MATCH_DECLINE',
        payload: {}
      });
    }
    setMatchFound(null);
    setIsMatching(false);
  };

  const handleRematch = () => {
    if (!gameResult) return;
    
    console.log('[REMATCH] Sending challenge to:', gameResult.opponent);
    
    // Send challenge to the same opponent
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', {
        cmd: 'CHALLENGE',
        payload: {
          target_username: gameResult.opponent
        }
      });
    }
    
    // Close result modal and return to lobby
    setGameResult(null);
    setScreen('lobby');
    setGameState(null);
    
    // Request player list when returning to lobby for rematch
    if (socketRef.current && socketRef.current.connected) {
      console.log('[REMATCH] Requesting player list');
      socketRef.current.emit('client-message', {
        cmd: 'PLAYER_LIST',
        payload: {}
      });
    }
  };

  const handleBackToLobby = () => {
    setGameResult(null);
    setScreen('lobby');
    setGameState(null);
    
    // Request player list when returning to lobby
    if (socketRef.current && socketRef.current.connected) {
      console.log('[BACK_TO_LOBBY] Requesting player list');
      socketRef.current.emit('client-message', {
        cmd: 'PLAYER_LIST',
        payload: {}
      });
    }
  };

  const handleLogout = () => {
    // Clear localStorage
    localStorage.removeItem('battleship_user');
    
    // Reset state
    setUser(null);
    setScreen('login');
    setGameState(null);
    setChallengeRequest(null);
    setGameResult(null);
    setDrawRequest(null);
    
    console.log('Logged out');
  };

  return (
    <div className="app">
      <header className="app-header">
        <h1>🚢 BattleShip Network Game</h1>
        <div className={`status ${connected ? 'connected' : 'disconnected'}`}>
          {connected ? '🟢 Connected' : '🔴 Disconnected'}
        </div>
        {user && (
          <div className="user-info">
            <span className="username">👤 {user.username}</span>
            {user.elo !== undefined && (
              <span className="user-elo">⭐ {user.elo} ELO</span>
            )}
            <button className="logout-btn" onClick={handleLogout}>
              Đăng xuất
            </button>
          </div>
        )}
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
            isMatching={isMatching}
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

      {/* Draw offer modal - Highest priority */}
      {drawRequest && (
        <div className="modal-overlay">
          <div className="draw-modal">
            <h3>🤝 Đề nghị hòa</h3>
            <p><strong>{drawRequest.from}</strong> muốn hòa trận đấu này</p>
            <div className="modal-actions">
              <button className="accept-btn" onClick={() => handleDrawReply(true)}>
                Đồng ý
              </button>
              <button className="reject-btn" onClick={() => handleDrawReply(false)}>
                Từ chối
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Match Found Modal */}
      {matchFound && (
        <div className="modal-overlay match-overlay">
          <div className="match-modal">
            <div className="match-icon">🎯</div>
            <h2>Tìm thấy đối thủ!</h2>
            <div className="match-info">
              <div className="opponent-info-card">
                <span className="opponent-name">👤 {matchFound.opponent}</span>
                <span className="opponent-elo">⭐ {matchFound.elo} ELO</span>
              </div>
            </div>
            <p className="match-instruction">Bạn có muốn chơi với đối thủ này không?</p>
            <div className="match-buttons">
              <button className="ready-button" onClick={handleMatchReady}>
                ✓ Sẵn sàng
              </button>
              <button className="decline-button" onClick={handleMatchDecline}>
                ✗ Từ chối
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Challenge Modal - Always on top */}
      {challengeRequest && (
        <div className="challenge-modal-overlay">
          <div className="challenge-modal">
            <h2 className="textcolor">⚔️ Challenge Request!</h2>
            <p className="challenger-name">{challengeRequest.challenger}</p>
            <p>challenges you to a game!</p>
            <div className="challenge-buttons">
              <button 
                className="accept-button"
                onClick={() => handleChallengeResponse(true)}
              >
                ✓ Accept
              </button>
              <button 
                className="reject-button"
                onClick={() => handleChallengeResponse(false)}
              >
                ✗ Reject
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Game Result Modal */}
      {gameResult && (
        <div className="game-result-overlay">
          <div className="game-result-modal">
            <div className={`result-icon ${gameResult.result === 'WIN' ? 'win' : gameResult.result === 'DRAW' ? 'draw' : 'lose'}`}>
              {gameResult.result === 'WIN' ? '🏆' : gameResult.result === 'DRAW' ? '🤝' : '😢'}
            </div>
            <h2 className={`result-title ${gameResult.result === 'WIN' ? 'win' : gameResult.result === 'DRAW' ? 'draw' : 'lose'}`}>
              {gameResult.result === 'WIN' ? 'CHIẾN THẮNG!' : gameResult.result === 'DRAW' ? 'HÒA!' : 'THẤT BẠI'}
            </h2>
            <p className="result-reason">{gameResult.reason}</p>
            <p className="opponent-info">Đối thủ: {gameResult.opponent}</p>
            {gameResult.elo !== undefined && (
              <div className="elo-info">
                <span className="elo-label">ELO mới:</span>
                <span className={`elo-value ${gameResult.result === 'WIN' ? 'win' : 'lose'}`}>
                  ⭐ {gameResult.elo}
                  {gameResult.result === 'WIN' && <span className="elo-change"> (+10)</span>}
                  {gameResult.result === 'LOSE' && <span className="elo-change"> (-10)</span>}
                </span>
              </div>
            )}
            
            <div className="result-buttons">
              <button 
                className="rematch-button"
                onClick={handleRematch}
              >
                ⚔️ Đấu lại
              </button>
              <button 
                className="lobby-button"
                onClick={handleBackToLobby}
              >
                🏠 Về sảnh
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;
