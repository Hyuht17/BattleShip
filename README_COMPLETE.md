# BattleShip Network Game - Hướng dẫn hoàn chỉnh

## 🎯 Tổng quan dự án

Đây là trò chơi BattleShip mạng với kiến trúc Client-Server đầy đủ:
- **C++ TCP Server**: Xử lý logic game, authentication, matching
- **Node.js Middleware**: Kết nối giữa frontend và C++ server
- **React Frontend**: Giao diện người dùng hiện đại

## 📋 Yêu cầu hệ thống

- Node.js v14+ (khuyến nghị v18 hoặc v20)
- G++ compiler (Linux/MacOS) hoặc MinGW (Windows)
- npm hoặc yarn

## 🚀 Hướng dẫn cài đặt

### Bước 1: Update Node.js (quan trọng!)

Kiểm tra phiên bản hiện tại:
```bash
node --version
```

Nếu phiên bản < v14, cần update:

**Cách 1: Sử dụng NVM (khuyến nghị)**
```bash
# Cài NVM
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
source ~/.bashrc

# Cài Node.js v20 LTS
nvm install 20
nvm use 20
nvm alias default 20
```

**Cách 2: Tải từ nodejs.org**
Tải và cài đặt từ: https://nodejs.org/

### Bước 2: Cài đặt dependencies

#### Frontend:
```bash
cd frontend
npm install
```

#### Node.js Server:
```bash
cd node-server
npm install
```

### Bước 3: Compile C++ Server

```bash
cd server
g++ -o server_full server_full.cpp -lpthread
```

Hoặc trên Windows:
```bash
g++ -o server_full.exe server_full.cpp -lpthread -lws2_32
```

## 🎮 Chạy ứng dụng

### Cách 1: Chạy thủ công (khuyến nghị cho development)

**Terminal 1 - C++ Server:**
```bash
cd server
./server_full
```

**Terminal 2 - Node.js Middleware:**
```bash
cd node-server
npm start
```

**Terminal 3 - React Frontend:**
```bash
cd frontend
npm run dev
```

### Cách 2: Sử dụng script tự động

#### Linux/MacOS:
```bash
# Tạo script
chmod +x start-all.sh
./start-all.sh
```

#### Windows:
```cmd
start-all.bat
```

## 📖 Hướng dẫn sử dụng

### 1. Đăng ký tài khoản
- Mở trình duyệt: http://localhost:5173
- Click "Register here"
- Nhập username và password
- Click "Register"

### 2. Đăng nhập
- Nhập thông tin đã đăng ký
- Click "Login"

### 3. Thách đấu
- Ở màn hình Lobby, xem danh sách người chơi online
- Click "Challenge" để thách đấu người chơi khác
- Đối phương sẽ nhận thông báo và chấp nhận/từ chối

### 4. Đặt tàu
- Khi bắt đầu game, đặt 5 tàu trên bảng:
  - Carrier (5 ô)
  - Battleship (4 ô)
  - Cruiser (3 ô)
  - Submarine (3 ô)
  - Destroyer (2 ô)
- Chọn hướng ngang/dọc
- Click vào ô để đặt tàu
- Click "Confirm Placement" khi hoàn thành

### 5. Chơi game
- Đến lượt của bạn, click vào bảng đối thủ để tấn công
- 💥 = Trúng (Hit)
- 💧 = Trượt (Miss)
- Người đánh chìm hết tàu đối thủ trước thắng!

### 6. Chat
- Sử dụng chat box bên phải để nhắn tin với đối thủ
- Nhập tin nhắn và nhấn Enter hoặc click 📤

## 🏗️ Cấu trúc dự án

```
Project/
├── server/                 # C++ TCP Server
│   ├── server_full.cpp    # Server code hoàn chỉnh
│   └── users.dat          # Database người dùng (auto-created)
│
├── node-server/           # Node.js Middleware
│   ├── server.js          # Socket.IO server
│   └── package.json
│
└── frontend/              # React Frontend
    ├── src/
    │   ├── App_new.jsx          # Main app
    │   ├── components/
    │   │   ├── LoginScreen.jsx
    │   │   ├── LobbyScreen.jsx
    │   │   ├── GameScreen.jsx
    │   │   ├── GameBoard.jsx
    │   │   ├── ShipPlacement.jsx
    │   │   └── ChatBox.jsx
    │   └── *.css           # Styling files
    └── package.json
```

## 🔧 Troubleshooting

### Lỗi: "Port already in use"
```bash
# Linux/MacOS
sudo lsof -i :8080  # hoặc :3000, :5173
kill -9 <PID>

# Windows
netstat -ano | findstr :8080
taskkill /PID <PID> /F
```

### Lỗi: "Cannot connect to C++ server"
- Đảm bảo C++ server đang chạy
- Kiểm tra firewall không chặn port 8080

### Lỗi: "Module not found"
```bash
# Xóa node_modules và cài lại
rm -rf node_modules package-lock.json
npm install
```

### Lỗi biên dịch C++
```bash
# Cài pthread library (Ubuntu/Debian)
sudo apt-get install build-essential

# MacOS
xcode-select --install
```

## 📝 Protocol API

### Commands (Client → Server):
- `REGISTER`: Đăng ký tài khoản
- `LOGIN`: Đăng nhập
- `PLAYER_LIST`: Lấy danh sách người chơi
- `CHALLENGE`: Gửi lời thách đấu
- `CHALLENGE_REPLY`: Trả lời thách đấu
- `PLACE_SHIPS`: Đặt tàu
- `MOVE`: Thực hiện nước đi
- `CHAT`: Gửi tin nhắn

### Responses (Server → Client):
- `LOGIN_SUCCESS`: Đăng nhập thành công
- `GAME_START`: Bắt đầu game
- `MOVE_RESULT`: Kết quả nước đi
- `GAME_END`: Kết thúc game
- `SYSTEM_MSG`: Thông báo hệ thống

## 🎯 Tính năng đã implement

✅ Đăng ký và đăng nhập tài khoản
✅ Hiển thị danh sách người chơi online
✅ Thách đấu và chấp nhận/từ chối
✅ Đặt tàu với validation
✅ Gameplay đầy đủ (tấn công, hit/miss)
✅ Phát hiện thắng/thua
✅ Chat trong game
✅ Xử lý disconnect
✅ Giao diện đẹp, responsive
✅ Real-time updates

## 👥 Phân công công việc

**Đặng Quang Huy**: 
- Authentication (REGISTER, LOGIN)
- Player management (PLAYER_LIST, CHALLENGE)
- Scoring system

**Lê Bá Ngọc Hiểu**:
- Game logic (MOVE, PLACE_SHIPS)
- Win/lose detection
- Chat feature
- Frontend UI

## 📞 Hỗ trợ

Nếu gặp vấn đề, kiểm tra:
1. Tất cả 3 servers đang chạy
2. Ports 8080, 3000, 5173 không bị chặn
3. Node.js version >= 14
4. Browser console để xem lỗi

## 🎉 Chúc bạn chơi game vui vẻ!
