import { useEffect, useState } from 'react';
import MatchHistory from './MatchHistory';
import MainMenu from './MainMenu';
import Leaderboard from './Leaderboard';
import AvailablePlayers from './AvailablePlayers';

function LobbyScreen({ socket, sendMessage, user, isMatching, myPing, onLogout }) {
  const [showHistory, setShowHistory] = useState(false);
  const [showPlayerList, setShowPlayerList] = useState(false);
  const [showLeaderboard, setShowLeaderboard] = useState(false);
  const [leaderboardData, setLeaderboardData] = useState([]);
  const [leaderboardLoading, setLeaderboardLoading] = useState(true);

  useEffect(() => {
    if (socket) {
      const handleMessage = (data) => {
        if (data.cmd === 'LEADERBOARD') {
          console.log('[LEADERBOARD] Received data:', data.payload);
          const players = data.payload?.players || [];
          console.log('[LEADERBOARD] Players count:', players.length);
          setLeaderboardData(players);
          setLeaderboardLoading(false);
        }
      };

      socket.on('server-message', handleMessage);

      return () => {
        socket.off('server-message', handleMessage);
      };
    }
  }, [socket]);

  const refreshLeaderboard = () => {
    console.log('[LEADERBOARD] Refreshing leaderboard...');
    setLeaderboardLoading(true);
    sendMessage({
      cmd: 'LEADERBOARD',
      payload: {}
    });
    console.log('[LEADERBOARD] Request sent to server');
  };

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
    <div className="w-full min-h-screen bg-white relative">
      {showPlayerList ? (
        <AvailablePlayers
          socket={socket}
          sendMessage={sendMessage}
          isMatching={isMatching}
          onBack={() => setShowPlayerList(false)}
        />
      ) : (
        <div className="w-full max-w-7xl mx-auto px-5">
          <div className="flex flex-col lg:flex-row gap-6">
            {/* Main Menu */}
            <div className="flex-1">
              <MainMenu
                socket={socket}
                sendMessage={sendMessage}
                user={user}
                isMatching={isMatching}
                onShowHistory={() => setShowHistory(true)}
                onShowPlayerList={() => setShowPlayerList(true)}
                onShowLeaderboard={() => {
                  setShowLeaderboard(true);
                  refreshLeaderboard();
                }}
                onLogout={onLogout}
              />
            </div>
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
