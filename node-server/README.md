# BattleShip Node.js Middleware Server

Đây là Node.js server làm middleware giữa React frontend và C++ server.

## Cài đặt

```bash
npm install
```

## Chạy server

### Development mode (với auto-reload)
```bash
npm run dev
```

### Production mode
```bash
npm start
```

## Cấu hình

- Node.js server chạy trên port: **3000**
- Kết nối tới C++ server tại: **localhost:8080**
- Frontend chạy tại: **http://localhost:5173**

## Các tính năng

- Socket.IO để giao tiếp real-time với frontend
- TCP socket để kết nối với C++ server
- CORS được bật cho frontend
- Health check endpoint tại `/health`

## API

### WebSocket Events

#### Từ Client -> Server
- `client-message`: Gửi message tới C++ server

#### Từ Server -> Client
- `server-connected`: Khi kết nối thành công với C++ server
- `server-message`: Nhận data từ C++ server
- `server-error`: Khi có lỗi từ C++ server
- `server-disconnected`: Khi mất kết nối với C++ server
