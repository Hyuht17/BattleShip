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
      alert("It's not your turn!");
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
    alert('Đã gửi đề nghị hòa tới đối thủ');
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
    </div>
  );
}

export default GameScreen;
