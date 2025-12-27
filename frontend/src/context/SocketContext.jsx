import { createContext, useContext, useEffect, useRef, useState } from 'react';
import { io } from 'socket.io-client';
import { SOCKET_SERVER_URL } from '../config';

const SocketContext = createContext(null);

export const useSocket = () => {
  const context = useContext(SocketContext);
  if (!context) {
    throw new Error('useSocket must be used within SocketProvider');
  }
  return context;
};

export function SocketProvider({ children }) {
  const socketRef = useRef(null);
  const [connected, setConnected] = useState(false);
  const [user, setUser] = useState(null);

  useEffect(() => {
    // Initialize socket connection
    socketRef.current = io(SOCKET_SERVER_URL, {
      transports: ['websocket', 'polling'],
      reconnection: true,
      reconnectionDelay: 1000,
      reconnectionAttempts: 5
    });

    socketRef.current.on('connect', () => {
      console.log('Connected to Node.js server');
      setConnected(true);
    });

    socketRef.current.on('disconnect', () => {
      console.log('Disconnected from server');
      setConnected(false);
    });

    return () => {
      if (socketRef.current) {
        socketRef.current.disconnect();
      }
    };
  }, []);

  const sendMessage = (data) => {
    if (socketRef.current && socketRef.current.connected) {
      socketRef.current.emit('client-message', data);
    }
  };

  const value = {
    socket: socketRef.current,
    connected,
    user,
    setUser,
    sendMessage
  };

  return (
    <SocketContext.Provider value={value}>
      {children}
    </SocketContext.Provider>
  );
}
