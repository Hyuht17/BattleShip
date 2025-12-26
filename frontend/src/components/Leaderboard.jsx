import { useEffect, useState } from 'react';

function Leaderboard({ socket, sendMessage, onClose }) {
  const [players, setPlayers] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    // Request leaderboard data
    if (socket) {
      const handleMessage = (data) => {
        console.log('[LEADERBOARD] Received message:', data);
        console.log('[LEADERBOARD] Message type:', typeof data);
        console.log('[LEADERBOARD] Message cmd:', data?.cmd);
        
        // Handle both direct data and string JSON
        let parsedData = data;
        if (typeof data === 'string') {
          try {
            parsedData = JSON.parse(data);
          } catch (e) {
            console.error('[LEADERBOARD] Failed to parse message:', e);
            return;
          }
        }
        
        if (parsedData?.cmd === 'LEADERBOARD') {
          console.log('[LEADERBOARD] Payload:', parsedData.payload);
          console.log('[LEADERBOARD] Players:', parsedData.payload?.players);
          
          let playersList = parsedData.payload?.players || [];
          
          // Ensure it's an array
          if (!Array.isArray(playersList)) {
            console.warn('[LEADERBOARD] Players is not an array:', playersList);
            playersList = [];
          }
          
          console.log('[LEADERBOARD] Players list length:', playersList.length);
          console.log('[LEADERBOARD] First player:', playersList[0]);
          
          setPlayers(playersList);
          setLoading(false);
        }
      };

      socket.on('server-message', handleMessage);

      // Request data
      console.log('[LEADERBOARD] Requesting leaderboard data');
      sendMessage({
        cmd: 'LEADERBOARD',
        payload: {}
      });

      return () => {
        socket.off('server-message', handleMessage);
      };
    }
  }, [socket, sendMessage]);

  const getMedalEmoji = (rank) => {
    if (rank === 1) return '🥇';
    if (rank === 2) return '🥈';
    if (rank === 3) return '🥉';
    return `#${rank}`;
  };

  const getEloColor = (elo) => {
    if (elo >= 1200) return 'text-yellow-600';
    if (elo >= 1000) return 'text-gray-500';
    if (elo >= 850) return 'text-orange-600';
    return 'text-gray-400';
  };

  return (
    <div 
      className="fixed inset-0 bg-black/40 backdrop-blur-md flex items-center justify-center z-50 animate-fadeIn"
      onClick={onClose}
    >
      <div 
        className="bg-white rounded-lg shadow-xl w-[90%] max-w-4xl max-h-[85vh] flex flex-col overflow-hidden animate-slideUp"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Header */}
        <div className="bg-gray-50 border-b border-gray-200 px-6 py-4 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-800">🏆 Bảng Xếp Hạng</h2>
          <button 
            className="w-10 h-10 rounded-full bg-gray-200 hover:bg-gray-300 text-gray-700 flex items-center justify-center transition-colors hover:rotate-90"
            onClick={onClose}
          >
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" className="w-5 h-5">
              <path d="M18 6L6 18M6 6l12 12" />
            </svg>
          </button>
        </div>

        {/* Content */}
        {loading ? (
          <div className="flex flex-col items-center justify-center py-20 px-4">
            <div className="w-12 h-12 border-4 border-gray-300 border-t-gray-800 rounded-full animate-spin mb-4"></div>
            <p className="text-gray-600">Đang tải dữ liệu...</p>
          </div>
        ) : (
          <div className="flex-1 overflow-y-auto p-6">
            {players.length === 0 ? (
              <div className="text-center py-20 text-gray-500">
                <p className="text-lg">Chưa có dữ liệu xếp hạng</p>
                <p className="text-sm mt-2">Số lượng người chơi: {players.length}</p>
              </div>
            ) : (
              <div className="bg-white border border-gray-200 rounded-lg overflow-hidden shadow-md">
                {/* Table Header */}
                <div className="grid grid-cols-[60px_1fr_90px_70px_70px_85px] md:grid-cols-[80px_1fr_120px_80px_80px_100px] gap-2 md:gap-4 px-3 md:px-6 py-3 md:py-4 bg-gray-100 border-b border-gray-200 sticky top-0 z-10">
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide">Hạng</div>
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide">Người chơi</div>
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide">ELO</div>
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide text-center">Trận</div>
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide text-center">Thắng</div>
                  <div className="font-bold text-gray-700 text-xs md:text-sm uppercase tracking-wide text-center">Tỷ lệ</div>
                </div>

                {/* Table Body */}
                <div className="max-h-[450px] overflow-y-auto">
                  {players.map((player) => (
                    <div 
                      key={player.rank} 
                      className={`grid grid-cols-[60px_1fr_90px_70px_70px_85px] md:grid-cols-[80px_1fr_120px_80px_80px_100px] gap-2 md:gap-4 px-3 md:px-6 py-3 md:py-4 border-b border-gray-100 transition-all hover:bg-gray-50 hover:translate-x-1 ${
                        player.rank <= 3 ? 'bg-yellow-50 font-semibold' : ''
                      }`}
                    >
                      <div className="flex items-center">
                        <span className={`${player.rank === 1 ? 'text-xl md:text-2xl' : player.rank === 2 ? 'text-lg md:text-xl' : player.rank === 3 ? 'text-base md:text-lg' : 'text-sm md:text-base'} font-bold`}>
                          {getMedalEmoji(player.rank)}
                        </span>
                      </div>
                      <div className="flex items-center min-w-0">
                        <span className="font-semibold text-gray-800 truncate text-sm md:text-base">{player.username}</span>
                      </div>
                      <div className="flex items-center">
                        <span className={`font-bold text-sm md:text-lg ${getEloColor(player.elo)}`}>
                          ⭐ {player.elo}
                        </span>
                      </div>
                      <div className="flex items-center justify-center text-gray-600 font-medium text-sm md:text-base">
                        {player.games}
                      </div>
                      <div className="flex items-center justify-center text-gray-600 font-medium text-sm md:text-base">
                        {player.wins}
                      </div>
                      <div className="flex items-center justify-center">
                        <span className={`px-2 md:px-3 py-1 rounded-full text-xs md:text-sm font-bold ${
                          player.winrate >= 50 
                            ? 'bg-green-100 text-green-700' 
                            : 'bg-red-100 text-red-700'
                        }`}>
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

        {/* Footer */}
        <div className="bg-gray-50 border-t border-gray-200 px-6 py-4 flex gap-4">
          <button 
            className="flex-1 px-6 py-3 bg-white border border-gray-300 text-gray-800 rounded-lg font-semibold hover:bg-gray-100 hover:-translate-y-0.5 hover:shadow-md transition-all"
            onClick={() => {
              setLoading(true);
              sendMessage({ cmd: 'LEADERBOARD', payload: {} });
            }}
          >
            🔄 Làm mới
          </button>
          <button 
            className="flex-1 px-6 py-3 bg-gray-800 text-white rounded-lg font-semibold hover:bg-gray-700 hover:-translate-y-0.5 hover:shadow-md transition-all"
            onClick={onClose}
          >
            Đóng
          </button>
        </div>
      </div>
    </div>
  );
}

export default Leaderboard;
