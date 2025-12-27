import { useEffect, useState, useRef } from 'react';

function AvailablePlayers({ socket, sendMessage, isMatching, onBack }) {
  const [players, setPlayers] = useState([]);
  const [refreshing, setRefreshing] = useState(false);
  const hasRequested = useRef(false);

  useEffect(() => {
    if (!socket || !sendMessage) return;

    console.log('[AVAILABLE_PLAYERS] Component mounted');

    // Gửi request lần đầu khi vào trang
    if (!hasRequested.current) {
      console.log('[AVAILABLE_PLAYERS] Sending initial PLAYER_LIST request...');
      hasRequested.current = true;
      setRefreshing(true);
      sendMessage({
        cmd: 'PLAYER_LIST',
        payload: {}
      });
    }

    // Listen for player list response
    const handleMessage = (data) => {
      if (data.cmd === 'PLAYER_LIST') {
        console.log('[AVAILABLE_PLAYERS] Received PLAYER_LIST:', data.payload);
        setPlayers(data.payload.players || []);
        setRefreshing(false);
      }
    };

    socket.on('server-message', handleMessage);

    // Auto-refresh mỗi 5 giây
    const interval = setInterval(() => {
      console.log('[AVAILABLE_PLAYERS] Auto-refreshing player list...');
      sendMessage({
        cmd: 'PLAYER_LIST',
        payload: {}
      });
    }, 5000);

    return () => {
      console.log('[AVAILABLE_PLAYERS] Component unmounting, cleaning up...');
      clearInterval(interval);
      socket.off('server-message', handleMessage);
    };
  }, [socket, sendMessage]);

  const handleRefresh = () => {
    console.log('[AVAILABLE_PLAYERS] Manual refresh triggered');
    setRefreshing(true);
    sendMessage({
      cmd: 'PLAYER_LIST',
      payload: {}
    });
  };

  const handleChallenge = (targetUsername) => {
    if (window.confirm(`Challenge ${targetUsername} to a game?`)) {
      console.log('[AVAILABLE_PLAYERS] Sending challenge to:', targetUsername);
      sendMessage({
        cmd: 'CHALLENGE',
        payload: { target_username: targetUsername }
      });
    }
  };

  return (
    <div className="w-full max-w-4xl mx-auto px-5 py-10">
      <div className="flex items-center gap-4 mb-8">
        <button 
          className="w-10 h-10 rounded-lg bg-gray-100 hover:bg-gray-200 flex items-center justify-center transition-colors"
          onClick={onBack}
        >
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" className="w-5 h-5 text-gray-800">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
        </button>
        <h2 className="text-3xl font-bold text-gray-800">Available Players</h2>
      </div>

      {isMatching && (
        <div className="flex items-center justify-center gap-4 p-4 mb-6 bg-gray-50 border border-gray-200 rounded-lg text-gray-800 font-medium">
          <div className="w-5 h-5 border-2 border-gray-300 border-t-gray-800 rounded-full animate-spin"></div>
          <span>Đang tìm đối thủ có ELO tương đương (±100)...</span>
        </div>
      )}

      <div className="min-h-[300px] mb-6">
        {players.length === 0 ? (
          <div className="text-center py-20 text-gray-500">
            <p className="text-lg mb-2">No other players online</p>
            <p className="text-sm">Waiting for opponents to join...</p>
          </div>
        ) : (
          <div className="space-y-3">
            {players.map((player, index) => (
              <div key={index} className="bg-gray-50 border border-gray-300 rounded-lg p-4 flex items-center gap-4 hover:border-gray-400 transition-colors">
                {/* Avatar */}
                <div className="w-12 h-12 rounded-full bg-gray-700 flex items-center justify-center text-white font-bold text-lg flex-shrink-0">
                  {player.username?.[0]?.toUpperCase() || 'P'}
                </div>
                
                {/* Player Name */}
                <div className="flex flex-col min-w-0">
                  <span className="text-gray-800 font-semibold text-base truncate">{player.username || 'Unknown'}</span>
                </div>
                
                {/* ELO */}
                <div className="flex flex-col">
                  <span className="text-gray-500 text-lg font-medium">{player.elo !== undefined ? player.elo : '0'}</span>
                  <span className="text-gray-500 text-xs">ELO</span>
                </div>
                
                {/* Status */}
                <span className={`text-xs font-medium px-3 py-1 rounded whitespace-nowrap ${
                  player.status === 1 ? 'bg-green-400 text-white' :
                  player.status === 2 ? 'bg-yellow-400 text-white' :
                  'bg-red-400 text-white'
                }`}>
                  {player.status === 1 ? 'Online' : 
                   player.status === 2 ? 'In Lobby' : 'In Game'}
                </span>
                
                {/* Challenge Button */}
                <button
                  onClick={() => handleChallenge(player.username)}
                  disabled={player.status === 3}
                  className="ml-auto px-4 py-2 bg-gray-700 text-white rounded font-semibold text-sm hover:bg-gray-800 transition-colors disabled:bg-gray-300 disabled:text-gray-500 disabled:cursor-not-allowed"
                >
                  Challenge
                </button>
              </div>
            ))}
          </div>
        )}
      </div>

      <div className="flex justify-center">
        <button 
          onClick={handleRefresh} 
          disabled={refreshing}
          className="px-6 py-2 bg-gray-100 text-gray-800 border border-gray-300 rounded-md font-semibold hover:bg-gray-200 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
        >
          {refreshing ? 'Refreshing...' : 'Refresh'}
        </button>
      </div>
    </div>
  );
}

export default AvailablePlayers;
