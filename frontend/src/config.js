// Auto-detect server URL for LAN support
export const getServerURL = () => {
  const hostname = window.location.hostname;
  const port = import.meta.env.VITE_NODE_PORT || 3000;
  
  // Nếu đang dev trên localhost
  if (hostname === 'localhost' || hostname === '127.0.0.1') {
    return `http://localhost:${port}`;
  }
  
  // Nếu truy cập từ LAN IP, kết nối đến cùng IP đó
  return `http://${hostname}:${port}`;
};

export const SOCKET_SERVER_URL = getServerURL();

console.log('🌐 Socket Server URL:', SOCKET_SERVER_URL);
