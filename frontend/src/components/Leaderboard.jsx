import { useEffect, useState } from 'react';
import './Leaderboard.css';

function Leaderboard({ socket, sendMessage, onClose }) {
  const [players, setPlayers] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    // Request leaderboard data
    if (socket) {
      socket.on('server-message', (data) => {
        if (data.cmd === 'LEADERBOARD') {
          setPlayers(data.payload.players || []);
          setLoading(false);
        }
      });

      // Request data
      sendMessage({
        cmd: 'LEADERBOARD',
        payload: {}
      });
    }

    return () => {
      if (socket) {
        socket.off('server-message');
      }
    };
  }, [socket, sendMessage]);

  const getMedalEmoji = (rank) => {
    if (rank === 1) return '🥇';
    if (rank === 2) return '🥈';
    if (rank === 3) return '🥉';
    return `#${rank}`;
  };

  const getEloColor = (elo) => {
    if (elo >= 1200) return '#ffd700'; // Gold
    if (elo >= 1000) return '#c0c0c0'; // Silver
    if (elo >= 850) return '#cd7f32'; // Bronze
    return '#94a3b8'; // Default
  };

  return (
    <div className="leaderboard-overlay" onClick={onClose}>
      <div className="leaderboard-modal" onClick={(e) => e.stopPropagation()}>
        <div className="leaderboard-header">
          <h2>🏆 Bảng Xếp Hạng</h2>
          <button className="close-button" onClick={onClose}>✕</button>
        </div>

        {loading ? (
          <div className="loading-container">
            <div className="loading-spinner"></div>
            <p>Đang tải dữ liệu...</p>
          </div>
        ) : (
          <div className="leaderboard-content">
            {players.length === 0 ? (
              <div className="no-data">
                <p>Chưa có dữ liệu xếp hạng</p>
              </div>
            ) : (
              <div className="leaderboard-table">
                <div className="table-header">
                  <div className="col-rank">Hạng</div>
                  <div className="col-player">Người chơi</div>
                  <div className="col-elo">ELO</div>
                  <div className="col-games">Trận</div>
                  <div className="col-wins">Thắng</div>
                  <div className="col-winrate">Tỷ lệ</div>
                </div>
                <div className="table-body">
                  {players.map((player) => (
                    <div 
                      key={player.rank} 
                      className={`table-row ${player.rank <= 3 ? 'top-rank' : ''}`}
                    >
                      <div className="col-rank">
                        <span className={`rank-badge rank-${player.rank}`}>
                          {getMedalEmoji(player.rank)}
                        </span>
                      </div>
                      <div className="col-player">
                        <span className="player-name">{player.username}</span>
                      </div>
                      <div className="col-elo">
                        <span 
                          className="elo-badge" 
                          style={{ color: getEloColor(player.elo) }}
                        >
                          ⭐ {player.elo}
                        </span>
                      </div>
                      <div className="col-games">{player.games}</div>
                      <div className="col-wins">{player.wins}</div>
                      <div className="col-winrate">
                        <span className={`winrate ${player.winrate >= 50 ? 'positive' : 'negative'}`}>
                          {player.winrate.toFixed(1)}%
                        </span>
                      </div>
                    </div>
                  ))}
                </div>
              </div>
            )}
          </div>
        )}

        <div className="leaderboard-footer">
          <button className="refresh-btn" onClick={() => {
            setLoading(true);
            sendMessage({ cmd: 'LEADERBOARD', payload: {} });
          }}>
            🔄 Làm mới
          </button>
          <button className="close-btn" onClick={onClose}>Đóng</button>
        </div>
      </div>
    </div>
  );
}

export default Leaderboard;
