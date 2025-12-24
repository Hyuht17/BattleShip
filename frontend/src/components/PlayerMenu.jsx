import { useState } from 'react';
import './PlayerMenu.css';

function PlayerMenu({ user, onLogout, onShowHistory, onShowSettings }) {
  const [isOpen, setIsOpen] = useState(false);

  return (
    <div className="player-menu">
      <button 
        className="menu-toggle"
        onClick={() => setIsOpen(!isOpen)}
        aria-label="Toggle menu"
      >
        <span className="menu-icon"></span>
        <span className="menu-icon"></span>
        <span className="menu-icon"></span>
      </button>

      {isOpen && (
        <>
          <div 
            className="menu-overlay"
            onClick={() => setIsOpen(false)}
          ></div>
          <div className="menu-panel">
            <div className="menu-header">
              <div className="player-avatar">
                <div className="avatar-circle">
                  {user?.username?.[0]?.toUpperCase() || 'P'}
                </div>
              </div>
              <div className="player-info">
                <h3 className="player-name">{user?.username || 'Player'}</h3>
                <div className="player-elo">
                  <span className="elo-label">ELO</span>
                  <span className="elo-value">{user?.elo || 800}</span>
                </div>
              </div>
            </div>

            <nav className="menu-nav">
              <button 
                className="menu-item"
                onClick={() => {
                  onShowHistory?.();
                  setIsOpen(false);
                }}
              >
                <span className="menu-item-icon history-icon"></span>
                <span className="menu-item-text">Lịch sử trận đấu</span>
              </button>
              
              <button 
                className="menu-item"
                onClick={() => {
                  onShowSettings?.();
                  setIsOpen(false);
                }}
              >
                <span className="menu-item-icon settings-icon"></span>
                <span className="menu-item-text">Cài đặt</span>
              </button>
              
              <div className="menu-divider"></div>
              
              <button 
                className="menu-item logout-item"
                onClick={() => {
                  onLogout?.();
                  setIsOpen(false);
                }}
              >
                <span className="menu-item-icon logout-icon"></span>
                <span className="menu-item-text">Đăng xuất</span>
              </button>
            </nav>
          </div>
        </>
      )}
    </div>
  );
}

export default PlayerMenu;

