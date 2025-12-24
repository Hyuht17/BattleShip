import { useEffect, useState } from 'react';

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

  const getResultClass = (result) => {
    switch (result) {
      case 'WIN':
        return 'text-green-600';
      case 'LOSE':
        return 'text-red-600';
      case 'DRAW':
        return 'text-yellow-600';
      default:
        return 'text-gray-600';
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

  const getResultBadgeColor = (result) => {
    switch (result) {
      case 'WIN':
        return 'bg-green-100 text-green-800 border-green-300';
      case 'LOSE':
        return 'bg-red-100 text-red-800 border-red-300';
      case 'DRAW':
        return 'bg-yellow-100 text-yellow-800 border-yellow-300';
      default:
        return 'bg-gray-100 text-gray-800 border-gray-300';
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
    <div className="fixed inset-0 bg-black bg-opacity-30 flex items-center justify-center z-50" onClick={onClose}>
      <div className="bg-white rounded-lg shadow-xl w-full max-w-2xl mx-4 max-h-[85vh] flex flex-col animate-fadeIn" onClick={(e) => e.stopPropagation()}>
        {/* Header */}
        <div className="flex justify-between items-center p-6 border-b border-gray-200">
          <h2 className="text-2xl font-bold text-gray-800">Lịch Sử Thi Đấu</h2>
          <button 
            className="w-8 h-8 rounded-full bg-gray-100 hover:bg-gray-200 flex items-center justify-center transition-colors"
            onClick={onClose}
          >
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" className="w-5 h-5 text-gray-600">
              <path d="M18 6L6 18M6 6l12 12" />
            </svg>
          </button>
        </div>

        {loading ? (
          <div className="flex items-center justify-center py-20">
            <div className="w-8 h-8 border-2 border-gray-300 border-t-gray-800 rounded-full animate-spin"></div>
            <span className="ml-3 text-gray-600">Đang tải...</span>
          </div>
        ) : (
          <>
            {/* Stats */}
            <div className="p-6 bg-gray-50 border-b border-gray-200">
              <div className="grid grid-cols-2 md:grid-cols-5 gap-4">
                <div className="text-center">
                  <div className="text-2xl font-bold text-gray-800 mb-1">{matches.length}</div>
                  <div className="text-sm text-gray-500 uppercase tracking-wide">Tổng trận</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-bold text-green-600 mb-1">{stats.wins}</div>
                  <div className="text-sm text-gray-500 uppercase tracking-wide">Thắng</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-bold text-red-600 mb-1">{stats.losses}</div>
                  <div className="text-sm text-gray-500 uppercase tracking-wide">Thua</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-bold text-yellow-600 mb-1">{stats.draws}</div>
                  <div className="text-sm text-gray-500 uppercase tracking-wide">Hòa</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-bold text-gray-800 mb-1">{winRate}%</div>
                  <div className="text-sm text-gray-500 uppercase tracking-wide">Tỷ lệ thắng</div>
                </div>
              </div>
            </div>

            {/* Matches List */}
            <div className="flex-1 overflow-y-auto p-6">
              {matches.length === 0 ? (
                <div className="text-center py-20 text-gray-500">
                  <p className="text-lg mb-2">Chưa có trận đấu nào</p>
                  <p className="text-sm text-gray-400">Hãy thách đấu người chơi khác để bắt đầu!</p>
                </div>
              ) : (
                <div className="space-y-3">
                  {matches.map((match, index) => (
                    <div 
                      key={index} 
                      className="bg-gray-50 border border-gray-200 rounded-lg p-4 hover:border-gray-300 transition-colors"
                    >
                      <div className="flex items-center justify-between">
                        <div className="flex items-center gap-4 flex-1">
                          {/* Result Badge */}
                          <span className={`px-3 py-1 rounded font-semibold text-sm border ${getResultBadgeColor(match.result)}`}>
                            {getResultText(match.result)}
                          </span>
                          
                          {/* Opponent */}
                          <div className="flex-1">
                            <div className="text-sm text-gray-500 mb-1">Đối thủ</div>
                            <div className="font-semibold text-gray-800">{match.opponent}</div>
                          </div>
                          
                          {/* Time */}
                          <div className="text-right">
                            <div className="text-sm text-gray-500 mb-1">Thời gian</div>
                            <div className="text-sm text-gray-600">{formatDate(match.timestamp)}</div>
                          </div>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </>
        )}
      </div>
    </div>
  );
}

export default MatchHistory;
