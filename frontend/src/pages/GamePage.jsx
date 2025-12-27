import { useNavigate } from 'react-router-dom';
import { useEffect } from 'react';
import GameScreen from '../components/GameScreen';

function GamePage({ socket, user, gameState, setGameState, onGameEnd }) {
  const navigate = useNavigate();

  useEffect(() => {
    // Redirect to login if not authenticated
    if (!user) {
      navigate('/login');
      return;
    }

    // Redirect to lobby if no game state
    if (!gameState) {
      navigate('/lobby');
      return;
    }
  }, [user, gameState, navigate]);

  const handleGameEnd = () => {
    // Clear in-game flag
    localStorage.removeItem('battleship_in_game');
    
    // Call parent handler
    if (onGameEnd) {
      onGameEnd();
    }
    
    // Navigate back to lobby
    navigate('/lobby');
  };

  if (!user || !gameState) return null;

  return (
    <GameScreen 
      socket={socket}
      user={user}
      gameState={gameState}
      setGameState={setGameState}
      onGameEnd={handleGameEnd}
    />
  );
}

export default GamePage;
