import { useEffect, useState } from 'react';
import './LobbyScreen.css';
import MatchHistory from './MatchHistory';

function LobbyScreen({ socket, sendMessage, user, isMatching }) {
  const [players, setPlayers] = useState([]);
  const [refreshing, setRefreshing] = useState(false);
  const [showHistory, setShowHistory] = useState(false);

  useEffect(() => {
    // Request player list on mount
    refreshPlayerList();

    // Listen for player list updates
    if (socket) {
      socket.on('server-message', (data) => {
        if (data.cmd === 'PLAYER_LIST') {
          setPlayers(data.payload.players || []);
          setRefreshing(false);
        }
      });
    }

    // Auto-refresh every 5 seconds
    const interval = setInterval(() => {
      refreshPlayerList();
    }, 5000);

    return () => clearInterval(interval);
  }, [socket]);

  const refreshPlayerList = () => {
    setRefreshing(true);
    sendMessage({
      cmd: 'PLAYER_LIST',
      payload: {}
    });
  };

  const handleChallenge = (targetUsername) => {
    if (window.confirm(`Challenge ${targetUsername} to a game?`)) {
      console.log('[LOBBY] Sending challenge to:', targetUsername);
      sendMessage({
        cmd: 'CHALLENGE',
        payload: { target_username: targetUsername }
      });
      console.log('[LOBBY] Challenge sent');
    }
  };

  const handleStartMatching = () => {
    console.log('[LOBBY] Starting matching...');
    sendMessage({
      cmd: 'START_MATCHING',
      payload: {}
    });
  };

  const handleCancelMatching = () => {
    console.log('[LOBBY] Cancelling matching...');
    sendMessage({
      cmd: 'CANCEL_MATCHING',
      payload: {}
    });
  };

  return (
    <div className="lobby-screen">
      <div className="lobby-container">
        <h2>🎮 Game Lobby</h2>
        
        <div className="lobby-header">
          <h3>Available Players</h3>
          <div className="lobby-actions">
            <button 
              onClick={() => setShowHistory(true)}
              className="history-button"
            >
              📊 Lịch sử
            </button>
            {isMatching ? (
              <button 
                onClick={handleCancelMatching}
                className="matching-button active"
              >
                ⏸️ Hủy tìm trận
              </button>
            ) : (
              <button 
                onClick={handleStartMatching}
                className="matching-button"
              >
                🎯 Tìm trận (±100 ELO)
              </button>
            )}
            <button 
              onClick={refreshPlayerList} 
              className="refresh-button"
              disabled={refreshing}
            >
              {refreshing ? '🔄 Refreshing...' : '🔄 Refresh'}
            </button>
          </div>
        </div>

        {isMatching && (
          <div className="matching-indicator">
            <div className="matching-spinner"></div>
            <span>Đang tìm đối thủ có ELO tương đương (±100)...</span>
          </div>
        )}

        <div className="players-list">
          {players.length === 0 ? (
            <div className="no-players">
              <p>No other players online</p>
              <p className="hint">Waiting for opponents to join...</p>
            </div>
          ) : (
            <div className="players-grid">
              {players.map((player, index) => (
                <div key={index} className="player-card">
                  <div className="player-info">
                    <span className="player-icon">👤</span>
                    <div className="player-details">
                      <span className="player-name">{player.username}</span>
                      {player.elo !== undefined && (
                        <span className="player-elo">⭐ {player.elo} ELO</span>
                      )}
                    </div>
                    <span className={`player-status status-${player.status}`}>
                      {player.status === 1 ? '🟢 Online' : 
                       player.status === 2 ? '🟡 In Lobby' : '🔴 In Game'}
                    </span>
                  </div>
                  <button
                    onClick={() => handleChallenge(player.username)}
                    className="challenge-button"
                    disabled={player.status === 3}
                  >
                    ⚔️ Challenge
                  </button>
                </div>
              ))}
            </div>
          )}
        </div>

        <div className="lobby-info">
          <p>💡 Click "Challenge" to start a game with another player</p>
          <p>💡 You will be notified when someone challenges you</p>
        </div>
      </div>
      
      {showHistory && (
        <MatchHistory 
          socket={socket} 
          user={user} 
          onClose={() => setShowHistory(false)} 
        />
      )}
    </div>
  );
}

export default LobbyScreen;
