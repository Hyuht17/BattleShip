import { useNavigate } from 'react-router-dom';
import { useEffect } from 'react';
import LobbyScreen from '../components/LobbyScreen';

function LobbyPage({ socket, user, onLogout, gameState, setGameState }) {
  const navigate = useNavigate();

  useEffect(() => {
    // Redirect to login if not authenticated
    if (!user) {
      navigate('/login');
      return;
    }

    // Clear in-game flag when in lobby
    localStorage.removeItem('battleship_in_game');
  }, [user, navigate]);

  useEffect(() => {
    // Navigate to game when game starts
    if (gameState) {
      localStorage.setItem('battleship_in_game', 'true');
      navigate('/game');
    }
  }, [gameState, navigate]);

  const handleLogoutClick = () => {
    onLogout();
    navigate('/login');
  };

  if (!user) return null;

  return (
    <LobbyScreen 
      socket={socket}
      user={user}
      onLogout={handleLogoutClick}
      setGameState={setGameState}
    />
  );
}

export default LobbyPage;
