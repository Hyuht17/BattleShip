# 🚢 BattleShip Game - Hướng dẫn chạy

## Kiến trúc hệ thống

```
Frontend (React + Vite)  ←→  Node.js Server  ←→  C++ Server
     Port 5173               Port 3000              Port 8080
```

## Yêu cầu

- Node.js (v16 trở lên)
- npm hoặc yarn
- g++ compiler (cho C++ server)
- Windows/Linux/MacOS

## Hướng dẫn cài đặt và chạy

### 1. Cài đặt Node.js Server

```powershell
cd node-server
npm install
```

### 2. Cài đặt Frontend

```powershell
cd frontend
npm install
```

### 3. Compile C++ Server (Linux/Mac)

```bash
cd server
g++ server.cpp -o server
```

**Lưu ý:** Code C++ hiện tại cần được hoàn thiện thêm. Bạn cần thêm hàm `handle_client()` và các xử lý khác.

### 4. Chạy các server

#### Bước 1: Chạy C++ Server
```bash
cd server
./server
```

#### Bước 2: Chạy Node.js Server
Mở terminal mới:
```powershell
cd node-server
npm start
```

Hoặc dùng development mode với auto-reload:
```powershell
npm run dev
```

#### Bước 3: Chạy Frontend
Mở terminal mới:
```powershell
cd frontend
npm run dev
```

### 5. Truy cập ứng dụng

Mở trình duyệt và truy cập: **http://localhost:5173**

## Cấu trúc dự án

```
Project/
├── server/
│   └── server.cpp          # C++ TCP Server (Port 8080)
├── node-server/
│   ├── server.js           # Node.js Middleware (Port 3000)
│   ├── package.json
│   └── README.md
├── frontend/
│   ├── src/
│   │   ├── App.jsx         # React Component với Socket.IO
│   │   ├── App.css
│   │   └── main.jsx
│   ├── package.json
│   └── vite.config.js
└── README_SETUP.md         # File này
```

## Luồng dữ liệu

1. **Frontend → Node.js Server:**
   - Sử dụng Socket.IO WebSocket
   - Event: `client-message`

2. **Node.js Server → C++ Server:**
   - Sử dụng TCP Socket
   - Format: JSON string + newline

3. **C++ Server → Node.js Server:**
   - Sử dụng TCP Socket
   - Gửi raw data

4. **Node.js Server → Frontend:**
   - Sử dụng Socket.IO WebSocket
   - Events: `server-message`, `server-connected`, `server-error`, `server-disconnected`

## API Events

### Frontend Events

#### Gửi từ Client:
- `client-message`: Gửi message tới game server
  ```javascript
  socket.emit('client-message', {
    type: 'game-action',
    message: 'your message',
    timestamp: Date.now()
  });
  ```

#### Nhận từ Server:
- `server-connected`: Kết nối thành công với C++ server
- `server-message`: Nhận data từ C++ server
- `server-error`: Có lỗi xảy ra
- `server-disconnected`: Mất kết nối với C++ server

## Troubleshooting

### Frontend không kết nối được với Node.js server
- Kiểm tra Node.js server có đang chạy không
- Kiểm tra port 3000 có bị sử dụng không
- Xem console trong trình duyệt để biết lỗi chi tiết

### Node.js server không kết nối được với C++ server
- Kiểm tra C++ server có đang chạy không
- Kiểm tra port 8080 có bị sử dụng không
- Xem logs trong terminal Node.js server

### C++ server không hoạt động
- Đảm bảo đã compile thành công
- Kiểm tra code C++ có đầy đủ các hàm cần thiết không
- Code hiện tại thiếu hàm `handle_client()` - cần được bổ sung

## Phát triển tiếp

### C++ Server cần bổ sung:
1. Hàm `handle_client()` để xử lý client requests
2. Game logic (đặt thuyền, bắn, kiểm tra thắng/thua)
3. Quản lý nhiều clients (multiplayer)
4. Protocol định dạng message

### Frontend cần bổ sung:
1. Game board UI (bảng 10x10)
2. Ship placement interface
3. Attack interface
4. Game state management
5. Player info display

### Node.js Server:
- Có thể thêm authentication
- Logging
- Rate limiting
- Room management cho multiplayer

## Tài liệu tham khảo

- [Socket.IO Documentation](https://socket.io/docs/)
- [React Documentation](https://react.dev/)
- [Node.js Net Module](https://nodejs.org/api/net.html)
- [Vite Documentation](https://vitejs.dev/)
