import { useState, useEffect } from 'react';

function MainMenu({ socket, sendMessage, user, isMatching, onShowHistory, onShowPlayerList, onLogout, onNotification }) {
  const [localNotification, setLocalNotification] = useState(null);
  const [playerStats, setPlayerStats] = useState({
    wins: 0,
    losses: 0,
    winRate: 0
  });
  const [matchingTime, setMatchingTime] = useState(0);

  useEffect(() => {
    if (user) {
      setPlayerStats({
        wins: 247,
        losses: 189,
        winRate: 56
      });
    }
  }, [user]);

  // Timer for matching
  useEffect(() => {
    let interval = null;
    if (isMatching) {
      setMatchingTime(0);
      interval = setInterval(() => {
        setMatchingTime(prev => prev + 1);
      }, 1000);
    } else {
      setMatchingTime(0);
    }
    return () => {
      if (interval) clearInterval(interval);
    };
  }, [isMatching]);

  const formatTime = (seconds) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  };

  const handleRandomMatchmaking = () => {
    if (isMatching) {
      sendMessage({
        cmd: 'CANCEL_MATCHING',
        payload: {}
      });
    } else {
      sendMessage({
        cmd: 'START_MATCHING',
        payload: {}
      });
    }
  };

  const handleChallengeFriend = () => {
    onShowPlayerList?.();
  };

  const handlePlayVsAI = () => {
    setLocalNotification({
      title: 'Thông báo',
      message: 'Chức năng chơi với AI - đang phát triển',
      type: 'info'
    });
  };

  const handleLeaderboard = () => {
    onShowHistory?.();
  };

  const handleSettings = () => {
    setLocalNotification({
      title: 'Thông báo',
      message: 'Cài đặt - đang phát triển',
      type: 'info'
    });
  };

  return (
    <div className="w-full px-5 py-10 bg-white min-h-screen flex flex-col items-center relative">
      <div className="text-center mb-16 mt-10">
        <div className="w-20 h-20 mx-auto mb-6 bg-gray-800 border-2 border-white rounded shadow-md flex items-center justify-center">
          <svg viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="1.5" className="w-14 h-14">
            <rect x="3" y="8" width="18" height="10" rx="1" />
            <path d="M3 10h18M3 14h18M3 16h18" />
            <circle cx="7" cy="12" r="1" fill="white" />
            <circle cx="12" cy="12" r="1" fill="white" />
            <circle cx="17" cy="12" r="1" fill="white" />
            <path d="M6 6h12M8 4h8" strokeLinecap="round" />
          </svg>
        </div>
        <h1 className="text-5xl font-bold text-gray-800 mb-2 tracking-tight">Battleship</h1>
        <p className="text-base text-gray-500 font-normal">Naval Combat Strategy</p>
      </div>

      {/* Matching Indicator */}
      {isMatching && (
        <div className="w-full mb-6 px-6 py-4 bg-gray-100 border-2 border-gray-300 rounded-lg flex items-center justify-between">
          <div className="flex items-center gap-4">
            <div className="w-6 h-6 border-2 border-gray-400 border-t-gray-800 rounded-full animate-spin"></div>
            <div>
              <p className="text-sm font-semibold text-gray-800">Đang tìm đối thủ...</p>
            </div>
          </div>
          <div className="text-right">
            <div className="text-2xl font-bold text-gray-800">{formatTime(matchingTime)}</div>
            <button
              onClick={handleRandomMatchmaking}
              className="text-xs text-red-600 hover:text-red-700 font-semibold mt-1"
            >
              Hủy
            </button>
          </div>
        </div>
      )}

      <div className="w-full space-y-3 mb-16">
        <button 
          className={`w-full px-6 py-5 rounded-lg flex items-center justify-between transition-all ${
            isMatching 
              ? 'bg-gray-800 text-white' 
              : 'bg-gray-100 text-gray-800 hover:bg-gray-200 hover:translate-x-1'
          }`}
          onClick={handleRandomMatchmaking}
        >
          <div className="flex items-center gap-4">
            <div className="w-6 h-6 flex-shrink-0">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2" />
                <circle cx="9" cy="7" r="4" />
                <path d="M23 21v-2a4 4 0 0 0-3-3.87M16 3.13a4 4 0 0 1 0 7.75" />
              </svg>
            </div>
            <span className="text-lg font-medium">Random Matchmaking</span>
          </div>
          <span className="text-xl text-gray-400">→</span>
        </button>

        <button 
          className="w-full px-6 py-5 rounded-lg bg-gray-100 text-gray-800 hover:bg-gray-200 hover:translate-x-1 transition-all flex items-center justify-between"
          onClick={handleChallengeFriend}
        >
          <div className="flex items-center gap-4">
            <div className="w-6 h-6 flex-shrink-0">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M6 18L18 6M6 6l12 12" />
                <path d="M12 2L2 7l10 5 10-5-10-5zM2 17l10 5 10-5M2 12l10 5 10-5" />
              </svg>
            </div>
            <span className="text-lg font-medium">Challenge Friend</span>
          </div>
          <span className="text-xl text-gray-400">→</span>
        </button>


        <button 
          className="w-full px-6 py-5 rounded-lg bg-gray-100 text-gray-800 hover:bg-gray-200 hover:translate-x-1 transition-all flex items-center justify-between"
          onClick={handleLeaderboard}
        >
          <div className="flex items-center gap-4">
            <div className="w-6 h-6 flex-shrink-0">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M6 9H4.5a2.5 2.5 0 0 1 0-5H6M18 9h1.5a2.5 2.5 0 0 0 0-5H18M4 22h16M10 14.66V17c0 .55-.47.98-.97 1.21C7.85 18.75 7 20.24 7 22M14 14.66V17c0 .55.47.98.97 1.21C16.15 18.75 17 20.24 17 22M12 2v6" />
              </svg>
            </div>
            <span className="text-lg font-medium">Match History</span>
          </div>
          <span className="text-xl text-gray-400">→</span>
        </button>


        <button 
          className="w-full px-6 py-5 rounded-lg bg-red-50 text-red-700 hover:bg-red-100 hover:translate-x-1 transition-all flex items-center justify-between border border-red-200"
          onClick={onLogout}
        >
          <div className="flex items-center gap-4">
            <div className="w-6 h-6 flex-shrink-0">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4M16 17l5-5-5-5M21 12H9" />
              </svg>
            </div>
            <span className="text-lg font-medium">Đăng xuất</span>
          </div>
          <span className="text-xl text-red-400">→</span>
        </button>
      </div>

      <div className="flex gap-10 mt-auto mb-5">
        <div className="text-center">
          <div className="text-3xl font-bold text-gray-800 mb-1">{playerStats.wins}</div>
          <div className="text-sm text-gray-500 uppercase tracking-wide">Wins</div>
        </div>
        <div className="text-center">
          <div className="text-3xl font-bold text-gray-800 mb-1">{playerStats.losses}</div>
          <div className="text-sm text-gray-500 uppercase tracking-wide">Losses</div>
        </div>
        <div className="text-center">
          <div className="text-3xl font-bold text-gray-800 mb-1">{playerStats.winRate}%</div>
          <div className="text-sm text-gray-500 uppercase tracking-wide">Win Rate</div>
        </div>
      </div>

      <button 
        className="absolute bottom-5 right-5 w-10 h-10 rounded-full bg-gray-800 flex items-center justify-center hover:bg-gray-700 transition-colors shadow-md"
        title="Help"
      >
        <svg viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="2" className="w-5 h-5">
          <circle cx="12" cy="12" r="10" />
          <path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3M12 17h.01" />
        </svg>
      </button>

      {/* Local Notification Modal */}
      {localNotification && (
        <div className="fixed inset-0 bg-black bg-opacity-30 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-xl p-6 max-w-md w-full mx-4 animate-fadeIn">
            <div className="text-center">
              <div className={`w-16 h-16 mx-auto mb-4 rounded-full flex items-center justify-center ${
                localNotification.type === 'error' ? 'bg-red-100' :
                localNotification.type === 'success' ? 'bg-green-100' :
                localNotification.type === 'warning' ? 'bg-yellow-100' :
                'bg-blue-100'
              }`}>
                {localNotification.type === 'error' && (
                  <svg className="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M6 18L18 6M6 6l12 12" />
                  </svg>
                )}
                {localNotification.type === 'success' && (
                  <svg className="w-8 h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
                  </svg>
                )}
                {localNotification.type === 'warning' && (
                  <svg className="w-8 h-8 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
                  </svg>
                )}
                {localNotification.type === 'info' && (
                  <svg className="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                )}
              </div>
              <h3 className="text-xl font-bold text-gray-800 mb-2">{localNotification.title}</h3>
              <p className="text-gray-600 mb-6">{localNotification.message}</p>
              <button
                onClick={() => setLocalNotification(null)}
                className="px-6 py-2 bg-gray-800 hover:bg-gray-700 text-white rounded-lg font-semibold transition-colors"
              >
                Đóng
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default MainMenu;
