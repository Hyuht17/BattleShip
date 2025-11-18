import { useState } from 'react';
import './LoginScreen.css';

function LoginScreen({ onLogin, onRegister, connected }) {
  const [isLogin, setIsLogin] = useState(true);
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    
    if (!username || !password) {
      alert('Please fill in all fields');
      return;
    }

    if (isLogin) {
      onLogin(username, password);
    } else {
      onRegister(username, password);
    }
  };

  return (
    <div className="login-screen">
      <div className="login-container">
        <h2>{isLogin ? '🔐 Login' : '📝 Register'}</h2>
        
        <form onSubmit={handleSubmit} className="login-form">
          <div className="form-group">
            <label htmlFor="username">Username</label>
            <input
              type="text"
              id="username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              placeholder="Enter username"
              disabled={!connected}
              autoComplete="username"
            />
          </div>

          <div className="form-group">
            <label htmlFor="password">Password</label>
            <input
              type="password"
              id="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="Enter password"
              disabled={!connected}
              autoComplete={isLogin ? "current-password" : "new-password"}
            />
          </div>

          <button 
            type="submit" 
            className="submit-button"
            disabled={!connected}
          >
            {isLogin ? 'Login' : 'Register'}
          </button>
        </form>

        <div className="toggle-mode">
          {isLogin ? (
            <p>
              Don't have an account?{' '}
              <button onClick={() => setIsLogin(false)} className="link-button">
                Register here
              </button>
            </p>
          ) : (
            <p>
              Already have an account?{' '}
              <button onClick={() => setIsLogin(true)} className="link-button">
                Login here
              </button>
            </p>
          )}
        </div>

        {!connected && (
          <div className="connection-warning">
            ⚠️ Waiting for server connection...
          </div>
        )}
      </div>
    </div>
  );
}

export default LoginScreen;
