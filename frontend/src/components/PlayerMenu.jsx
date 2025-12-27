import { useState } from 'react';

function PlayerMenu({ user, onLogout, onShowHistory, onShowSettings }) {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <div className="relative z-[1000]">
      <button 
        className="bg-white/95 border-2 border-gray-700 rounded-lg w-12 h-12 flex flex-col justify-center items-center gap-1.5 cursor-pointer transition-all shadow-md hover:bg-white hover:shadow-lg hover:-translate-y-0.5 p-2"
        onClick={() => setIsOpen(!isOpen)}
        aria-label="Toggle menu"
      >
        <span className="w-6 h-0.5 bg-gray-700 rounded transition-all"></span>
        <span className="w-6 h-0.5 bg-gray-700 rounded transition-all"></span>
        <span className="w-6 h-0.5 bg-gray-700 rounded transition-all"></span>
      </button>

      {isOpen && (
        <>
          <div 
            className="fixed inset-0 bg-black/50 z-[999] animate-fadeIn"
            onClick={() => setIsOpen(false)}
          ></div>
          <div className="fixed top-0 right-0 w-80 h-screen bg-gradient-to-b from-gray-800 to-gray-900 shadow-2xl z-[1000] flex flex-col animate-slideInRight">
            {/* Header */}
            <div className="p-8 bg-white/10 border-b-2 border-white/20 flex items-center gap-4">
              <div className="flex-shrink-0">
                <div className="w-16 h-16 rounded-full bg-gradient-to-br from-blue-500 to-blue-700 flex items-center justify-center text-white text-3xl font-bold border-3 border-white/30 shadow-lg">
                  {user?.username?.[0]?.toUpperCase() || 'P'}
                </div>
              </div>
              <div className="flex-1 min-w-0">
                <h3 className="text-white text-xl font-bold mb-2 text-shadow-sm whitespace-nowrap overflow-hidden text-ellipsis">
                  {user?.username || 'Player'}
                </h3>
                <div className="flex items-center gap-2">
                  <span className="text-white/80 text-sm font-medium">ELO</span>
                  <span className="text-yellow-400 text-lg font-bold text-shadow-sm">{user?.elo || 800}</span>
                </div>
              </div>
            </div>

            {/* Navigation */}
            <nav className="flex-1 py-4">
              <button 
                className="w-full px-6 py-4 bg-transparent border-0 text-white flex items-center gap-4 cursor-pointer transition-all text-left text-base font-medium hover:bg-white/10 hover:pl-8"
                onClick={() => {
                  onShowHistory?.();
                  setIsOpen(false);
                }}
              >
                <svg viewBox="0 0 24 24" fill="white" className="w-6 h-6 flex-shrink-0 opacity-90">
                  <path d="M13 3c-4.97 0-9 4.03-9 9H1l3.89 3.89.07.14L9 12H6c0-3.87 3.13-7 7-7s7 3.13 7 7-3.13 7-7 7c-1.93 0-3.68-.79-4.94-2.06l-1.42 1.42C8.27 19.99 10.51 21 13 21c4.97 0 9-4.03 9-9s-4.03-9-9-9zm-1 5v5l4.28 2.54.72-1.21-3.5-2.08V8H12z"/>
                </svg>
                <span className="flex-1">Lịch sử trận đấu</span>
              </button>
              
              <button 
                className="w-full px-6 py-4 bg-transparent border-0 text-white flex items-center gap-4 cursor-pointer transition-all text-left text-base font-medium hover:bg-white/10 hover:pl-8"
                onClick={() => {
                  onShowSettings?.();
                  setIsOpen(false);
                }}
              >
                <svg viewBox="0 0 24 24" fill="white" className="w-6 h-6 flex-shrink-0 opacity-90">
                  <path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94L14.4 2.81c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/>
                </svg>
                <span className="flex-1">Cài đặt</span>
              </button>
              
              <div className="h-px bg-white/20 mx-6 my-2"></div>
              
              <button 
                className="w-full px-6 py-4 bg-transparent border-0 text-red-400 flex items-center gap-4 cursor-pointer transition-all text-left text-base font-medium hover:bg-red-500/20 hover:pl-8"
                onClick={() => {
                  onLogout?.();
                  setIsOpen(false);
                }}
              >
                <svg viewBox="0 0 24 24" fill="currentColor" className="w-6 h-6 flex-shrink-0 opacity-90">
                  <path d="M17 7l-1.41 1.41L18.17 11H8v2h10.17l-2.58 2.59L17 17l5-5zM4 5h8V3H4c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h8v-2H4V5z"/>
                </svg>
                <span className="flex-1">Đăng xuất</span>
              </button>
            </nav>
          </div>
        </>
      )}
    </div>
  );
}

export default PlayerMenu;

