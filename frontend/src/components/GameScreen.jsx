import { useState, useEffect } from 'react';
import './GameScreen.css';
import GameBoard from './GameBoard';
import ShipPlacement from './ShipPlacement';
import ChatBox from './ChatBox';

function GameScreen({ socket, sendMessage, gameState, setGameState, user }) {
  const [selectedCell, setSelectedCell] = useState(null);
  const [chatMessages, setChatMessages] = useState([]);

  useEffect(() => {
    if (socket) {
      socket.on('server-message', (data) => {
        if (data.cmd === 'CHAT') {
          setChatMessages(prev => [...prev, {
            from: data.payload.from,
            message: data.payload.message,
            timestamp: new Date().toLocaleTimeString()
          }]);
        }
      });
    }
  }, [socket]);

  const handlePlaceShips = (ships) => {
    console.log('[PLACE_SHIPS] Sending ships:', ships);
    sendMessage({
      cmd: 'PLACE_SHIPS',
      payload: { ships }
    });
    console.log('[PLACE_SHIPS] Message sent');

    // Update local board
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

  return (
    <div className="game-screen">
      <div className="game-header">
        <h2>⚔️ Battle in Progress</h2>
        <div className="game-info">
          <span className="opponent-info">🎯 vs {gameState.opponent}</span>
          <span className={`turn-indicator ${gameState.yourTurn ? 'your-turn' : 'opponent-turn'}`}>
            {gameState.yourTurn ? '🟢 Your Turn' : '🔴 Opponent\'s Turn'}
          </span>
        </div>
      </div>

      {gameState.phase === 'placing' ? (
        <ShipPlacement onPlaceShips={handlePlaceShips} />
      ) : (
        <div className="game-content">
          <div className="boards-container">
            <div className="board-section">
              <h3>Your Board</h3>
              <GameBoard
                board={gameState.myBoard}
                isOwnBoard={true}
                onCellClick={() => {}}
                disabled={true}
              />
            </div>

            <div className="board-section">
              <h3>Opponent's Board</h3>
              <GameBoard
                board={gameState.opponentBoard}
                isOwnBoard={false}
                onCellClick={handleCellClick}
                disabled={!gameState.yourTurn}
                selectedCell={selectedCell}
              />
            </div>
          </div>

          <ChatBox
            messages={chatMessages}
            onSendMessage={handleSendChat}
            currentUser={user.username}
          />
        </div>
      )}
    </div>
  );
}

export default GameScreen;
