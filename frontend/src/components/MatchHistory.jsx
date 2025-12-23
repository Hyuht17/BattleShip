import { useEffect, useState } from 'react';
import './MatchHistory.css';

function MatchHistory({ socket, user, onClose }) {
  const [matches, setMatches] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    if (!socket || !user) return;

    // Request match history
    socket.emit('client-message', {
      cmd: 'MATCH_HISTORY',
      payload: {}
    });

    // Listen for match history response
    const handleMessage = (data) => {
      if (data.cmd === 'MATCH_HISTORY') {
        setMatches(data.payload.matches || []);
        setLoading(false);
      }
    };

    socket.on('server-message', handleMessage);

    return () => {
      socket.off('server-message', handleMessage);
    };
  }, [socket, user]);

  const formatDate = (timestamp) => {
    const date = new Date(timestamp * 1000);
    return date.toLocaleString('vi-VN', {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit'
    });
  };

  const getResultIcon = (result) => {
    switch (result) {
      case 'WIN':
        return '🏆';
      case 'LOSE':
        return '😢';
      case 'DRAW':
        return '🤝';
      default:
        return '❓';
    }
  };

  const getResultClass = (result) => {
    switch (result) {
      case 'WIN':
        return 'win';
      case 'LOSE':
        return 'lose';
      case 'DRAW':
        return 'draw';
      default:
        return '';
    }
  };

  const getResultText = (result) => {
    switch (result) {
      case 'WIN':
        return 'Thắng';
      case 'LOSE':
        return 'Thua';
      case 'DRAW':
        return 'Hòa';
      default:
        return result;
    }
  };

  // Calculate statistics
  const stats = matches.reduce((acc, match) => {
    if (match.result === 'WIN') acc.wins++;
    else if (match.result === 'LOSE') acc.losses++;
    else if (match.result === 'DRAW') acc.draws++;
    return acc;
  }, { wins: 0, losses: 0, draws: 0 });

  const winRate = matches.length > 0 
    ? ((stats.wins / matches.length) * 100).toFixed(1) 
    : 0;

  return (
    <div className="match-history-overlay" onClick={onClose}>
      <div className="match-history-modal" onClick={(e) => e.stopPropagation()}>
        <div className="match-history-header">
          <h2>📊 Lịch Sử Thi Đấu</h2>
          <button className="close-btn" onClick={onClose}>✕</button>
        </div>

        {loading ? (
          <div className="loading">
            <p>Đang tải...</p>
          </div>
        ) : (
          <>
            <div className="match-stats">
              <div className="stat-item">
                <span className="stat-label">Tổng trận:</span>
                <span className="stat-value">{matches.length}</span>
              </div>
              <div className="stat-item win">
                <span className="stat-label">Thắng:</span>
                <span className="stat-value">{stats.wins}</span>
              </div>
              <div className="stat-item lose">
                <span className="stat-label">Thua:</span>
                <span className="stat-value">{stats.losses}</span>
              </div>
              <div className="stat-item draw">
                <span className="stat-label">Hòa:</span>
                <span className="stat-value">{stats.draws}</span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Tỷ lệ thắng:</span>
                <span className="stat-value">{winRate}%</span>
              </div>
            </div>

            <div className="matches-list">
              {matches.length === 0 ? (
                <div className="no-matches">
                  <p>Chưa có trận đấu nào</p>
                  <p className="hint">Hãy thách đấu người chơi khác để bắt đầu!</p>
                </div>
              ) : (
                matches.map((match, index) => (
                  <div key={index} className={`match-item ${getResultClass(match.result)}`}>
                    <div className="match-result">
                      <span className="result-icon">{getResultIcon(match.result)}</span>
                      <span className="result-text">{getResultText(match.result)}</span>
                    </div>
                    <div className="match-details">
                      <div className="match-opponent">
                        <span className="label">Đối thủ:</span>
                        <span className="value">{match.opponent}</span>
                      </div>
                      <div className="match-time">
                        <span className="label">Thời gian:</span>
                        <span className="value">{formatDate(match.timestamp)}</span>
                      </div>
                    </div>
                  </div>
                ))
              )}
            </div>
          </>
        )}
      </div>
    </div>
  );
}

export default MatchHistory;
