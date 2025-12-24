import { useState, useEffect } from 'react';
import './GameScreen.css';
import GameBoard from './GameBoard';
import ShipPlacement from './ShipPlacement';
import ChatBox from './ChatBox';

const SHIPS = [
  { name: 'Carrier', size: 5, symbol: '🛳️' },
  { name: 'Battleship', size: 4, symbol: '🚢' },
  { name: 'Cruiser', size: 3, symbol: '⛴️' },
  { name: 'Submarine', size: 3, symbol: '🚤' },
  { name: 'Destroyer', size: 2, symbol: '⛵' }
];

function GameScreen({ socket, sendMessage, gameState, setGameState, user }) {
  const [selectedCell, setSelectedCell] = useState(null);
  const [chatMessages, setChatMessages] = useState([]);
  const [localNotification, setLocalNotification] = useState(null);
  const [stats, setStats] = useState({
    myShipsRemaining: 5,
    opponentShipsRemaining: 5,
    myHits: 0,
    myMisses: 0,
    opponentHits: 0,
    opponentMisses: 0
  });

  useEffect(() => {
    if (socket) {
      const handleChatMessage = (data) => {
        if (data.cmd === 'CHAT') {
          console.log('[CHAT RECEIVED]', data.payload);
          setChatMessages(prev => [...prev, {
            from: data.payload.from,
            message: data.payload.message,
            timestamp: new Date().toLocaleTimeString()
          }]);
        }
      };

      socket.on('server-message', handleChatMessage);

      // Cleanup function to remove listener when component unmounts
      return () => {
        socket.off('server-message', handleChatMessage);
      };
    }
  }, [socket]);

  const handlePlaceShips = (ships) => {
    console.log('[PLACE_SHIPS] Sending ships:', ships);
    
    // Format ships for server (remove symbol field to match server's expected format)
    const shipsForServer = ships.map(ship => ({
      name: ship.name,
      size: ship.size,
      row: ship.row,
      col: ship.col,
      horizontal: ship.horizontal
    }));
    
    sendMessage({
      cmd: 'PLACE_SHIPS',
      payload: { ships: shipsForServer }
    });
    console.log('[PLACE_SHIPS] Message sent');

    // Update local board with ship symbols (ships already have symbols from ShipPlacement)
    const newBoard = Array(10).fill(null).map(() => Array(10).fill(0));

    ships.forEach(ship => {
      for (let i = 0; i < ship.size; i++) {
        const row = ship.row + (ship.horizontal ? 0 : i);
        const col = ship.col + (ship.horizontal ? i : 0);
        newBoard[row][col] = 1; // ship cell
      }
    });

    setGameState(prev => ({
      ...prev,
      myBoard: newBoard,
      myShips: ships
    }));
  };

  const handleCellClick = (row, col) => {
    if (gameState.phase !== 'playing') return;
    if (!gameState.yourTurn) {
      setLocalNotification({
        title: 'Không phải lượt của bạn',
        message: 'Vui lòng chờ đến lượt của bạn!',
        type: 'warning'
      });
      return;
    }

    // Convert to coordinate (e.g., A5)
    const coord = String.fromCharCode(65 + row) + col;

    sendMessage({
      cmd: 'MOVE',
      payload: { coord }
    });

    setSelectedCell({ row, col });
  };

  const handleSendChat = (message) => {
    console.log('[CHAT SEND]', message, 'from', user.username);
    sendMessage({
      cmd: 'CHAT',
      payload: { message }
    });

    setChatMessages(prev => [...prev, {
      from: user.username,
      message,
      timestamp: new Date().toLocaleTimeString()
    }]);
  };

  const handleSurrender = () => {
    if (window.confirm('Bạn có chắc muốn đầu hàng?')) {
      console.log('[SURRENDER] Sending surrender command');
      sendMessage({
        cmd: 'SURRENDER',
        payload: {}
      });
    }
  };

  const handleDrawOffer = () => {
    console.log('[DRAW_OFFER] Sending draw offer');
    sendMessage({
      cmd: 'DRAW_OFFER',
      payload: {}
    });
    setLocalNotification({
      title: 'Đã gửi đề nghị hòa',
      message: 'Đã gửi đề nghị hòa tới đối thủ. Đang chờ phản hồi...',
      type: 'info'
    });
  };

  // Calculate stats from boards
  useEffect(() => {
    if (gameState.myBoard && gameState.opponentBoard) {
      let myHits = 0, myMisses = 0, oppHits = 0, oppMisses = 0;
      
      gameState.myBoard.forEach(row => {
        row.forEach(cell => {
          if (cell === 2) oppHits++;
          if (cell === 3) oppMisses++;
        });
      });
      
      gameState.opponentBoard.forEach(row => {
        row.forEach(cell => {
          if (cell === 2) myHits++;
          if (cell === 3) myMisses++;
        });
      });
      
      setStats({
        myShipsRemaining: 5 - Math.floor(oppHits / 3), // Assuming avg 3 hits per ship
        opponentShipsRemaining: 5 - Math.floor(myHits / 3),
        myHits,
        myMisses,
        opponentHits: oppHits,
        opponentMisses: oppMisses
      });
    }
  }, [gameState.myBoard, gameState.opponentBoard]);

  return (
    <div className="game-screen">
      <div className="game-header">
        <div className="header-top">
          <h2>⚓ Naval Battle</h2>
          <span className={`turn-indicator ${gameState.yourTurn ? 'your-turn' : 'opponent-turn'}`}>
            {gameState.yourTurn ? '🟢 YOUR TURN' : '🔴 OPPONENT\'S TURN'}
          </span>
        </div>
        
        <div className="game-stats">
          <div className="stat-card player-stat">
            <div className="stat-header">
              <span className="stat-icon">👤</span>
              <span className="stat-player">You</span>
            </div>
            <div className="stat-content">
              <div className="stat-item">
                <span className="stat-label">Ships</span>
                <span className="stat-value">{stats.myShipsRemaining}/5 ⛵</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Hits</span>
                <span className="stat-value hit">{stats.myHits} 💥</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Misses</span>
                <span className="stat-value miss">{stats.myMisses} 💧</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Accuracy</span>
                <span className="stat-value">
                  {stats.myHits + stats.myMisses > 0 
                    ? Math.round((stats.myHits / (stats.myHits + stats.myMisses)) * 100)
                    : 0}%
                </span>
              </div>
            </div>
          </div>

          <div className="vs-divider">
            <span className="vs-text">VS</span>
          </div>

          <div className="stat-card opponent-stat">
            <div className="stat-header">
              <span className="stat-icon">🎯</span>
              <span className="stat-player">{gameState.opponent}</span>
            </div>
            <div className="stat-content">
              <div className="stat-item">
                <span className="stat-label">Ships</span>
                <span className="stat-value">{stats.opponentShipsRemaining}/5 ⛵</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Hits</span>
                <span className="stat-value hit">{stats.opponentHits} 💥</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Misses</span>
                <span className="stat-value miss">{stats.opponentMisses} 💧</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Accuracy</span>
                <span className="stat-value">
                  {stats.opponentHits + stats.opponentMisses > 0 
                    ? Math.round((stats.opponentHits / (stats.opponentHits + stats.opponentMisses)) * 100)
                    : 0}%
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      {gameState.phase === 'placing' ? (
        <ShipPlacement onPlaceShips={handlePlaceShips} />
      ) : (
        <div className="game-content">
          <div className="game-main">
            <div className="boards-container">
              <div className="board-section your-board">
                <div className="board-header">
                  <h3>🛡️ Your Fleet</h3>
                  <div className="board-status">
                    <span className="status-indicator defensive">Defensive</span>
                  </div>
                </div>
                <GameBoard
                  board={gameState.myBoard}
                  isOwnBoard={true}
                  onCellClick={() => {}}
                  disabled={true}
                  ships={gameState.myShips}
                />
              </div>

              <div className="board-section opponent-board">
                <div className="board-header">
                  <h3>⚔️ Enemy Waters</h3>
                  <div className="board-status">
                    <span className={`status-indicator ${gameState.yourTurn ? 'offensive-active' : 'offensive-waiting'}`}>
                      {gameState.yourTurn ? 'Fire at will!' : 'Waiting...'}
                    </span>
                  </div>
                </div>
                <GameBoard
                  board={gameState.opponentBoard}
                  isOwnBoard={false}
                  onCellClick={handleCellClick}
                  disabled={!gameState.yourTurn}
                  selectedCell={selectedCell}
                />
              </div>
            </div>

            <div className="right-panel">
              <div className="game-actions">
                <button className="action-btn surrender-btn" onClick={handleSurrender}>
                  <span className="btn-icon">🏳️</span>
                  <span className="btn-text">Surrender</span>
                </button>
                <button className="action-btn draw-btn" onClick={handleDrawOffer}>
                  <span className="btn-icon">🤝</span>
                  <span className="btn-text">Draw Offer</span>
                </button>
              </div>

              <div className="chat-section">
                <ChatBox
                  messages={chatMessages}
                  onSendMessage={handleSendChat}
                  currentUser={user.username}
                />
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Local Notification Modal */}
      {localNotification && (
        <div className="fixed inset-0 bg-black bg-opacity-30 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-xl p-6 max-w-md w-full mx-4 animate-fadeIn">
            <div className="text-center">
              <div className={`w-16 h-16 mx-auto mb-4 rounded-full flex items-center justify-center ${
                localNotification.type === 'error' ? 'bg-red-100' :
                localNotification.type === 'success' ? 'bg-green-100' :
                localNotification.type === 'warning' ? 'bg-yellow-100' :
                'bg-blue-100'
              }`}>
                {localNotification.type === 'error' && (
                  <svg className="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M6 18L18 6M6 6l12 12" />
                  </svg>
                )}
                {localNotification.type === 'success' && (
                  <svg className="w-8 h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
                  </svg>
                )}
                {localNotification.type === 'warning' && (
                  <svg className="w-8 h-8 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
                  </svg>
                )}
                {localNotification.type === 'info' && (
                  <svg className="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                )}
              </div>
              <h3 className="text-xl font-bold text-gray-800 mb-2">{localNotification.title}</h3>
              <p className="text-gray-600 mb-6">{localNotification.message}</p>
              <button
                onClick={() => setLocalNotification(null)}
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

export default GameScreen;
