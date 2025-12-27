import { useNavigate } from 'react-router-dom';
import LoginScreen from '../components/LoginScreen';

function LoginPage({ socket, onLogin }) {
  const navigate = useNavigate();

  const handleLoginSuccess = (userData) => {
    onLogin(userData);
    navigate('/lobby');
  };

  return (
    <LoginScreen 
      socket={socket} 
      onLoginSuccess={handleLoginSuccess}
    />
  );
}

export default LoginPage;
