import { useEffect, useState } from 'react';
import MatchHistory from './MatchHistory';
import MainMenu from './MainMenu';
import Leaderboard from './Leaderboard';

function LobbyScreen({ socket, sendMessage, user, isMatching, onLogout }) {
  const [players, setPlayers] = useState([]);
  const [refreshing, setRefreshing] = useState(false);
  const [showHistory, setShowHistory] = useState(false);
  const [showPlayerList, setShowPlayerList] = useState(false);
  const [showLeaderboard, setShowLeaderboard] = useState(false);
  const [leaderboardData, setLeaderboardData] = useState([]);
  const [leaderboardLoading, setLeaderboardLoading] = useState(true);

  useEffect(() => {
    console.log('[LOBBY] Component mounted/updated, refreshing data...');
    refreshPlayerList();
    refreshLeaderboard();

    if (socket) {
      const handleMessage = (data) => {
        if (data.cmd === 'PLAYER_LIST') {
          setPlayers(data.payload.players || []);
          setRefreshing(false);
        }
        if (data.cmd === 'LEADERBOARD') {
          console.log('[LEADERBOARD] Received data:', data.payload);
          const players = data.payload?.players || [];
          console.log('[LEADERBOARD] Players count:', players.length);
          setLeaderboardData(players);
          setLeaderboardLoading(false);
        }
      };

      socket.on('server-message', handleMessage);

      const interval = setInterval(() => {
        refreshPlayerList();
      }, 5000);

      const leaderboardInterval = setInterval(() => {
        refreshLeaderboard();
      }, 600000); // Auto refresh leaderboard every 10 minutes (600000ms)

      return () => {
        clearInterval(interval);
        clearInterval(leaderboardInterval);
        socket.off('server-message', handleMessage);
      };
    }
  }, [socket, sendMessage]);

  const refreshPlayerList = () => {
    setRefreshing(true);
    sendMessage({
      cmd: 'PLAYER_LIST',
      payload: {}
    });
  };

  const refreshLeaderboard = () => {
    console.log('[LEADERBOARD] Refreshing leaderboard...');
    setLeaderboardLoading(true);
    sendMessage({
      cmd: 'LEADERBOARD',
      payload: {}
    });
    console.log('[LEADERBOARD] Request sent to server');
  };

  const handleChallenge = (targetUsername) => {
    if (window.confirm(`Challenge ${targetUsername} to a game?`)) {
      console.log('[LOBBY] Sending challenge to:', targetUsername);
      sendMessage({
        cmd: 'CHALLENGE',
        payload: { target_username: targetUsername }
      });
    }
  };

  useEffect(() => {
    if (showPlayerList) {
      refreshPlayerList();
    }
  }, [showPlayerList]);

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
    <div className="w-full min-h-screen bg-white">
      {!showPlayerList ? (
        <div className="w-full max-w-7xl mx-auto px-5 py-10">
          <div className="flex flex-col lg:flex-row gap-6">
            {/* Main Menu - Left Side */}
            <div className="flex-1">
              <MainMenu
                socket={socket}
                sendMessage={sendMessage}
                user={user}
                isMatching={isMatching}
                onShowHistory={() => setShowHistory(true)}
                onShowPlayerList={() => setShowPlayerList(true)}
                onLogout={onLogout}
              />
            </div>

            {/* Leaderboard - Right Side */}
            <div className="w-full lg:w-80 flex-shrink-0">
              <div className="bg-white border border-gray-200 rounded-lg shadow-md sticky top-5">
                {/* Header */}
                <div className="bg-gray-50 border-b border-gray-200 px-4 py-3 flex items-center justify-between">
                  <h3 className="text-lg font-bold text-gray-800">🏆 Bảng Xếp Hạng</h3>
                  <button
                    onClick={(e) => {
                      e.preventDefault();
                      e.stopPropagation();
                      console.log('[LEADERBOARD] Refresh button clicked');
                      refreshLeaderboard();
                    }}
                    disabled={leaderboardLoading}
                    className={`w-8 h-8 rounded-full bg-gray-200 hover:bg-gray-300 active:bg-gray-400 flex items-center justify-center transition-colors ${
                      leaderboardLoading ? 'opacity-50 cursor-not-allowed' : 'cursor-pointer'
                    }`}
                    title="Làm mới"
                    type="button"
                  >
                    <svg 
                      viewBox="0 0 24 24" 
                      fill="none" 
                      stroke="currentColor" 
                      strokeWidth="2" 
                      className={`w-4 h-4 text-gray-700 ${leaderboardLoading ? 'animate-spin' : ''}`}
                    >
                      <path d="M1 4v6h6M23 20v-6h-6M21 12a9 9 0 11-2.636-6.364M3 12a9 9 0 012.636-6.364" />
                    </svg>
                  </button>
                </div>

                {/* Content */}
                {leaderboardLoading ? (
                  <div className="flex flex-col items-center justify-center py-12 px-4">
                    <div className="w-8 h-8 border-2 border-gray-300 border-t-gray-800 rounded-full animate-spin mb-3"></div>
                    <p className="text-sm text-gray-500">Đang tải...</p>
                  </div>
                ) : leaderboardData.length === 0 ? (
                  <div className="text-center py-12 px-4 text-gray-500">
                    <p className="text-sm">Chưa có dữ liệu</p>
                  </div>
                ) : (
                  <div className="max-h-[600px] overflow-y-auto">
                    <div className="divide-y divide-gray-100">
                      {leaderboardData.slice(0, 10).map((player) => (
                        <div
                          key={player.rank}
                          className={`px-4 py-3 hover:bg-gray-50 transition-colors ${
                            player.rank <= 3 ? 'bg-yellow-50' : ''
                          }`}
                        >
                          <div className="flex items-center gap-3">
                            {/* Rank */}
                            <div className="flex-shrink-0 w-8 text-center">
                              <span className={`font-bold ${
                                player.rank === 1 ? 'text-xl' :
                                player.rank === 2 ? 'text-lg' :
                                player.rank === 3 ? 'text-base' :
                                'text-sm text-gray-600'
                              }`}>
                                {getMedalEmoji(player.rank)}
                              </span>
                            </div>

                            {/* Player Info */}
                            <div className="flex-1 min-w-0">
                              <div className="flex items-center justify-between gap-2">
                                <span className="font-semibold text-gray-800 truncate text-sm">
                                  {player.username}
                                </span>
                                <span className={`font-bold text-sm flex-shrink-0 ${getEloColor(player.elo)}`}>
                                  {player.elo}
                                </span>
                              </div>
                              <div className="flex items-center gap-2 mt-1">
                                <span className="text-xs text-gray-500">
                                  {player.wins}W / {player.games}G
                                </span>
                                <span className={`text-xs px-2 py-0.5 rounded-full font-medium ${
                                  player.winrate >= 50
                                    ? 'bg-green-100 text-green-700'
                                    : 'bg-red-100 text-red-700'
                                }`}>
                                  {player.winrate.toFixed(1)}%
                                </span>
                              </div>
                            </div>
                          </div>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>
      ) : (
        <div className="w-full max-w-4xl mx-auto px-5 py-10">
          <div className="flex items-center gap-4 mb-8">
            <button 
              className="w-10 h-10 rounded-lg bg-gray-100 hover:bg-gray-200 flex items-center justify-center transition-colors"
              onClick={() => setShowPlayerList(false)}
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
              onClick={refreshPlayerList} 
              disabled={refreshing}
              className="px-6 py-2 bg-gray-100 text-gray-800 border border-gray-300 rounded-md font-semibold hover:bg-gray-200 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
            >
              {refreshing ? 'Refreshing...' : 'Refresh'}
            </button>
          </div>
        </div>
      )}
      
      {showHistory && (
        <MatchHistory 
          socket={socket} 
          user={user} 
          onClose={() => setShowHistory(false)} 
        />
      )}

      {showLeaderboard && (
        <Leaderboard
          socket={socket}
          sendMessage={sendMessage}
          onClose={() => setShowLeaderboard(false)}
        />
      )}
    </div>
  );
}

export default LobbyScreen;
