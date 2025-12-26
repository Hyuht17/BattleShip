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
  const [matchReadyStatus, setMatchReadyStatus] = useState({ me: false, opponent: false }); // Track ready status
  const [isMatching, setIsMatching] = useState(false);
  const [notification, setNotification] = useState(null); // {title: string, message: string, type: 'info'|'error'|'success'|'warning'}
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

  // Handle tab close/refresh - send LOGOUT
  useEffect(() => {
    const handleBeforeUnload = (e) => {
      if (user && socketRef.current?.connected) {
        // Send LOGOUT synchronously
        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/api/logout', false); // synchronous
        xhr.setRequestHeader('Content-Type', 'application/json');
        
        // Also try via socket (best effort)
        socketRef.current.emit('client-message', {
          cmd: 'LOGOUT',
          payload: {}
        });
        
        console.log('[LOGOUT] Sent logout on page unload');
      }
    };

    window.addEventListener('beforeunload', handleBeforeUnload);
    return () => {
      window.removeEventListener('beforeunload', handleBeforeUnload);
    };
  }, [user]);

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
      setNotification({
        title: 'Lỗi Server',
        message: data.error,
        type: 'error'
      });
    });

    // Game events - Define handler inline to avoid stale closure
    const handleMessage = (data) => {
      const { cmd, payload } = data;

      switch (cmd) {
        case 'LOGIN_SUCCESS':
          console.log('[LOGIN SUCCESS] User:', payload.username, 'ELO:', payload.elo);
          const userData = { 
            username: payload.username, 
            elo: payload.elo || 800,
            sessionToken: payload.sessionToken
          };
          setUser(userData);
          // Save to localStorage
          localStorage.setItem('battleship_user', JSON.stringify(userData));
          if (payload.sessionToken) {
            localStorage.setItem('battleship_session', payload.sessionToken);
          }
          setScreen('lobby');
          break;

        case 'REGISTER_SUCCESS':
          console.log('[REGISTER SUCCESS]');
          setNotification({
            title: 'Đăng ký thành công',
            message: 'Đăng ký thành công! Vui lòng đăng nhập.',
            type: 'success'
          });
          break;

        case 'GAME_START':
          console.log('[GAME_START] Opponent:', payload.opponent, 'Your turn:', payload.your_turn);
          opponentRef.current = payload.opponent; // Save opponent name
          setMatchFound(null); // Close match found modal
          setMatchReadyStatus({ me: false, opponent: false }); // Reset ready status
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
            setNotification({
              title: 'Thông báo',
              message: payload.message,
              type: 'error'
            });
          }
          break;

        case 'DRAW_OFFER':
          console.log('[DRAW_OFFER] Received from:', payload.from);
          setDrawRequest({ from: payload.from });
          break;

        case 'DRAW_REJECTED':
          console.log('[DRAW_REJECTED]');
          setNotification({
            title: 'Đề nghị hòa bị từ chối',
            message: 'Đối thủ đã từ chối đề nghị hòa của bạn.',
            type: 'warning'
          });
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
          setMatchReadyStatus({ me: false, opponent: false }); // Reset ready status
          break;

        case 'OPPONENT_READY':
          console.log('[OPPONENT_READY]', payload);
          setMatchReadyStatus(prev => ({ ...prev, opponent: true }));
          break;

        case 'WAITING_OPPONENT':
          console.log('[WAITING_OPPONENT]', payload);
          // Already showing in modal
          break;

        case 'MATCH_DECLINED':
          console.log('[MATCH_DECLINED]', payload);
          setMatchFound(null);
          setIsMatching(false);
          setMatchReadyStatus({ me: false, opponent: false }); // Reset ready status
          setNotification({
            title: 'Trận đấu bị từ chối',
            message: payload.message || 'Đối thủ đã từ chối trận đấu.',
            type: 'warning'
          });
          break;

        case 'MATCHING_CANCELLED':
          console.log('[MATCHING_CANCELLED]', payload);
          setIsMatching(false);
          break;

        case 'LOGOUT_SUCCESS':
          console.log('[LOGOUT_SUCCESS]');
          localStorage.removeItem('battleship_user');
          localStorage.removeItem('battleship_session');
          setUser(null);
          setScreen('login');
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
        setNotification({
          title: 'Tàu bị đánh chìm!',
          message: `${ship_sunk} đã bị đánh chìm!`,
          type: 'info'
        });
      }
    };

    const handleGameEndInline = (payload) => {
      // Show game result modal instead of alert
      const opponentName = opponentRef.current || payload.opponent || 'Không rõ';
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
      } else if (payload.reason === 'OPPONENT_LEFT_SETUP') {
        reason = 'Đối thủ đã thoát trong lúc đặt thuyền';
      } else if (payload.reason === 'SURRENDER') {
        reason = 'Đối thủ đã đầu hàng';
      } else if (payload.reason === 'DRAW_ACCEPTED') {
        reason = 'Hòa - Cả hai đồng ý';
      } else if (payload.reason === 'OPPONENT_TIMEOUT') {
        reason = 'Đối thủ không kết nối lại (timeout 60s)';
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
    setMatchReadyStatus(prev => ({ ...prev, me: true })); // Mark myself as ready
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
    setMatchReadyStatus({ me: false, opponent: false }); // Reset ready status
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
    
    // Request player list and leaderboard when returning to lobby for rematch
    if (socketRef.current && socketRef.current.connected) {
      console.log('[REMATCH] Requesting player list and leaderboard');
      socketRef.current.emit('client-message', {
        cmd: 'PLAYER_LIST',
        payload: {}
      });
      socketRef.current.emit('client-message', {
        cmd: 'LEADERBOARD',
        payload: {}
      });
    }
  };

  const handleBackToLobby = () => {
    setGameResult(null);
    setScreen('lobby');
    setGameState(null);
    
    // Request player list and leaderboard when returning to lobby
    if (socketRef.current && socketRef.current.connected) {
      console.log('[BACK_TO_LOBBY] Requesting player list and leaderboard');
      socketRef.current.emit('client-message', {
        cmd: 'PLAYER_LIST',
        payload: {}
      });
      socketRef.current.emit('client-message', {
        cmd: 'LEADERBOARD',
        payload: {}
      });
    }
  };

  const handleLogout = () => {
    // Send LOGOUT to server
    if (socketRef.current && socketRef.current.connected) {
      sendMessage({
        cmd: 'LOGOUT',
        payload: {}
      });
    }
    
    // Clear localStorage
    localStorage.removeItem('battleship_user');
    localStorage.removeItem('battleship_session');
    
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
    <div className="w-full min-h-screen bg-white">
      {screen !== 'lobby' && (
      <header className="w-full max-w-7xl mx-auto px-5 py-4 bg-gradient-to-r from-purple-600 to-purple-800 rounded-lg mb-5 mt-5 flex justify-between items-center shadow-md">
        <h1 className="text-2xl font-bold text-white">BattleShip Network Game</h1>
        <div className="flex items-center gap-4">
          <div className={`px-4 py-2 rounded-full text-sm font-semibold ${
            connected 
              ? 'bg-green-100 text-green-800 border-2 border-green-500' 
              : 'bg-red-100 text-red-800 border-2 border-red-500'
          }`}>
            {connected ? 'Connected' : 'Disconnected'}
          </div>
          {user && (
            <div className="flex items-center gap-3 text-white">
              <span className="font-semibold">{user.username}</span>
              {user.elo !== undefined && (
                <span className="bg-white/20 px-3 py-1 rounded-full text-sm font-bold text-yellow-300">
                  {user.elo} ELO
                </span>
              )}
              <button 
                className="px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg font-semibold text-sm transition-colors"
                onClick={handleLogout}
              >
                Đăng xuất
              </button>
            </div>
          )}
        </div>
      </header>
      )}

      <main className="w-full">
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
            onLogout={handleLogout}
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
        <div className="fixed inset-0 bg-black/40 backdrop-blur-md flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-xl p-8 max-w-md w-full mx-4 animate-fadeIn">
            <div className="text-center">
              {/* Icon */}
              <div className="w-16 h-16 mx-auto mb-6 bg-gray-800 rounded-full flex items-center justify-center">
                <svg viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="2" className="w-8 h-8">
                  <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2" />
                  <circle cx="9" cy="7" r="4" />
                  <path d="M23 21v-2a4 4 0 0 0-3-3.87M16 3.13a4 4 0 0 1 0 7.75" />
                </svg>
              </div>
              
              <h2 className="text-2xl font-bold text-gray-800 mb-6">Tìm thấy đối thủ!</h2>
              
              {/* Opponent Info */}
              <div className="bg-gray-50 border border-gray-200 rounded-lg p-6 mb-6">
                <div className="flex items-center justify-center gap-3 mb-4">
                  <div className="w-12 h-12 rounded-full bg-gray-700 flex items-center justify-center text-white font-bold text-lg">
                    {matchFound.opponent?.[0]?.toUpperCase() || 'O'}
                  </div>
                  <div className="text-left">
                    <div className="font-semibold text-gray-800 text-lg">{matchFound.opponent}</div>
                    <div className="text-gray-500 text-sm">{matchFound.elo} ELO</div>
                  </div>
                </div>
              </div>

              {/* Ready Status */}
              <div className="bg-gray-50 border border-gray-200 rounded-lg p-4 mb-6">
                <div className="space-y-3">
                  {/* My Status */}
                  <div className="flex items-center justify-between">
                    <span className="text-gray-700 font-medium">{user?.username || 'Bạn'}</span>
                    <div className="flex items-center gap-2">
                      {matchReadyStatus.me ? (
                        <>
                          <div className="w-3 h-3 rounded-full bg-green-500"></div>
                          <span className="text-green-600 font-semibold text-sm">Sẵn sàng</span>
                        </>
                      ) : (
                        <>
                          <div className="w-3 h-3 rounded-full bg-gray-400"></div>
                          <span className="text-gray-500 text-sm">Chưa sẵn sàng</span>
                        </>
                      )}
                    </div>
                  </div>
                  
                  {/* Opponent Status */}
                  <div className="flex items-center justify-between">
                    <span className="text-gray-700 font-medium">{matchFound.opponent}</span>
                    <div className="flex items-center gap-2">
                      {matchReadyStatus.opponent ? (
                        <>
                          <div className="w-3 h-3 rounded-full bg-green-500"></div>
                          <span className="text-green-600 font-semibold text-sm">Sẵn sàng</span>
                        </>
                      ) : (
                        <>
                          <div className="w-3 h-3 rounded-full bg-gray-400 animate-pulse"></div>
                          <span className="text-gray-500 text-sm">Đang chờ...</span>
                        </>
                      )}
                    </div>
                  </div>
                </div>
              </div>
              
              <p className="text-gray-600 mb-6">Bạn có muốn chơi với đối thủ này không?</p>
              
              {/* Buttons */}
              <div className="flex gap-4">
                <button 
                  className={`flex-1 px-6 py-3 rounded-lg font-semibold transition-colors ${
                    matchReadyStatus.me 
                      ? 'bg-green-500 hover:bg-green-600 text-white' 
                      : 'bg-gray-800 hover:bg-gray-700 text-white'
                  }`}
                  onClick={handleMatchReady}
                  disabled={matchReadyStatus.me}
                >
                  {matchReadyStatus.me ? '✓ Đã sẵn sàng' : 'Sẵn sàng'}
                </button>
                <button 
                  className="flex-1 px-6 py-3 bg-gray-100 hover:bg-gray-200 text-gray-800 rounded-lg font-semibold transition-colors"
                  onClick={handleMatchDecline}
                >
                  Từ chối
                </button>
              </div>
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

      {/* Notification Modal */}
      {notification && (
        <div className="fixed inset-0 bg-black bg-opacity-30 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-xl p-6 max-w-md w-full mx-4 animate-fadeIn">
            <div className="text-center">
              <div className={`w-16 h-16 mx-auto mb-4 rounded-full flex items-center justify-center ${
                notification.type === 'error' ? 'bg-red-100' :
                notification.type === 'success' ? 'bg-green-100' :
                notification.type === 'warning' ? 'bg-yellow-100' :
                'bg-blue-100'
              }`}>
                {notification.type === 'error' && (
                  <svg className="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M6 18L18 6M6 6l12 12" />
                  </svg>
                )}
                {notification.type === 'success' && (
                  <svg className="w-8 h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
                  </svg>
                )}
                {notification.type === 'warning' && (
                  <svg className="w-8 h-8 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
                  </svg>
                )}
                {notification.type === 'info' && (
                  <svg className="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                )}
              </div>
              <h3 className="text-xl font-bold text-gray-800 mb-2">{notification.title}</h3>
              <p className="text-gray-600 mb-6">{notification.message}</p>
              <button
                onClick={() => setNotification(null)}
                className="px-6 py-2 bg-gray-800 hover:bg-gray-700 text-white rounded-lg font-semibold transition-colors"
              >
                Đóng
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;
