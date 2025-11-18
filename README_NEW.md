# 🚢 BattleShip Network Game

> Trò chơi Hải chiến mạng đa người chơi với kiến trúc Client-Server hoàn chỉnh

[![Node.js](https://img.shields.io/badge/Node.js-v14+-green.svg)](https://nodejs.org/)
[![React](https://img.shields.io/badge/React-19-blue.svg)](https://reactjs.org/)
[![C++](https://img.shields.io/badge/C++-11+-red.svg)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-Complete-success.svg)]()

## 📖 Giới thiệu

**BattleShip Network Game** là đồ án môn Lập trình mạng, implement trò chơi Hải chiến (Battleship) truyền thống với các tính năng:

- 🎮 Chơi mạng real-time với nhiều người
- 🔐 Hệ thống đăng ký/đăng nhập
- ⚔️ Thách đấu và ghép cặp người chơi
- 🚢 Đặt tàu với validation đầy đủ
- 💬 Chat trong game
- 🏆 Xác định thắng/thua tự động
- 📱 Giao diện responsive, hiện đại

## 🏗️ Kiến trúc

```
┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│                 │         │                 │         │                 │
│  React Frontend │◄───────►│  Node.js Server │◄───────►│   C++ Server    │
│   (Port 5173)   │ WebSocket│   (Port 3000)   │  TCP/IP │   (Port 8080)   │
│                 │         │                 │         │                 │
└─────────────────┘         └─────────────────┘         └─────────────────┘
      UI/UX                    Middleware              Game Logic/Storage
```

### Thành phần chính:

1. **C++ TCP Server** - Core game logic, authentication, state management
2. **Node.js Middleware** - WebSocket ↔ TCP bridge, message routing
3. **React Frontend** - Modern UI with real-time updates

## ⚡ Quick Start

### 1. Kiểm tra yêu cầu

```bash
node --version  # Cần >= v14 (khuyến nghị v20)
g++ --version   # Cần compiler C++
npm --version   # Package manager
```

### 2. Cài đặt

```bash
# Clone repo
git clone https://github.com/Hyuht17/BattleShip.git
cd BattleShip

# Kiểm tra và cài đặt tự động
./demo.sh

# Hoặc thủ công:
make install  # Cài dependencies
make build    # Compile C++ server
```

### 3. Chạy game

```bash
# Cách 1: Script tự động (khuyến nghị)
./start-all.sh

# Cách 2: Makefile
make run

# Cách 3: Thủ công (3 terminals)
# Terminal 1
cd server && ./server_full

# Terminal 2
cd node-server && npm start

# Terminal 3
cd frontend && npm run dev
```

### 4. Mở game

Truy cập: **http://localhost:5173**

## 🎮 Hướng dẫn chơi

### Bước 1: Đăng ký/Đăng nhập
- Nhập username và password
- Click "Register" (lần đầu) hoặc "Login"

### Bước 2: Tìm đối thủ
- Xem danh sách người chơi online
- Click "Challenge" để thách đấu

### Bước 3: Đặt tàu
Đặt 5 tàu trên lưới 10x10:
- 🛳️ Carrier (5 ô)
- 🚢 Battleship (4 ô)
- ⛴️ Cruiser (3 ô)
- 🚤 Submarine (3 ô)
- ⛵ Destroyer (2 ô)

### Bước 4: Chơi!
- Lượt của bạn: Click vào bảng đối thủ
- 💥 = Hit (trúng)
- 💧 = Miss (trượt)
- Đánh chìm hết tàu → Thắng! 🎉

## 📁 Cấu trúc Project

```
BattleShip/
├── server/
│   ├── server_full.cpp      # C++ game server (COMPLETE)
│   └── users.dat            # User database
├── node-server/
│   ├── server.js            # Node.js middleware
│   └── package.json
├── frontend/
│   ├── src/
│   │   ├── App.jsx          # Main app
│   │   └── components/      # React components
│   └── package.json
├── start-all.sh             # Start script
├── demo.sh                  # Demo & check script
├── Makefile                 # Build automation
└── README.md               # This file
```

## ✨ Tính năng

### Core Features ✅
- [x] Đăng ký và đăng nhập
- [x] Danh sách người chơi online
- [x] Hệ thống thách đấu
- [x] Đặt tàu với validation
- [x] Turn-based gameplay
- [x] Hit/miss detection
- [x] Win/lose condition
- [x] Disconnect handling

### Advanced Features ✅
- [x] Real-time chat
- [x] Multiple concurrent games
- [x] Responsive UI design
- [x] Smooth animations
- [x] Error handling
- [x] Message buffering

## 🛠️ Công nghệ

| Layer | Technology | Purpose |
|-------|-----------|---------|
| Frontend | React 19, Vite, Socket.IO | UI & real-time communication |
| Middleware | Node.js, Express, Socket.IO | WebSocket ↔ TCP bridge |
| Backend | C++, pthread, POSIX sockets | Game logic & state management |
| Protocol | JSON over TCP/IP | Message format |

## 📚 Tài liệu

- **[QUICKSTART.md](QUICKSTART.md)** - Hướng dẫn bắt đầu nhanh
- **[README_COMPLETE.md](README_COMPLETE.md)** - Tài liệu đầy đủ
- **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Tổng kết dự án
- **[TESTING_CHECKLIST.md](TESTING_CHECKLIST.md)** - Checklist testing

## 🐛 Troubleshooting

### Node.js quá cũ?
```bash
# Cài NVM
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
source ~/.bashrc

# Update Node.js
nvm install 20
nvm use 20
```

### Port bị chiếm?
```bash
# Kiểm tra
lsof -i :8080
lsof -i :3000
lsof -i :5173

# Kill process
kill -9 $(lsof -t -i:8080)
```

### Lỗi compile C++?
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# MacOS
xcode-select --install
```

## 👥 Nhóm thực hiện

**Đặng Quang Huy**
- Module: Authentication, Player Management, Matching System
- GitHub: [@Hyuht17](https://github.com/Hyuht17)

**Lê Bá Ngọc Hiểu**
- Module: Game Logic, Frontend UI/UX, Chat System
- GitHub: [Your GitHub]

## 📝 License

Dự án được phát triển cho mục đích học tập - Môn Lập trình mạng

## 🎯 Demo

![Demo Screenshot](https://via.placeholder.com/800x400/667eea/ffffff?text=BattleShip+Game+Demo)

**Features showcased:**
- Login/Register flow
- Player lobby with online status
- Ship placement interface
- Live gameplay with turn indicator
- Real-time chat
- Win/lose notifications

## 🚀 Deployment

### Development
```bash
./start-all.sh
```

### Production
```bash
# Build frontend
cd frontend && npm run build

# Configure nginx/apache for static files
# Run Node.js as systemd service
# Run C++ server as systemd service
```

## 📊 Stats

- **Lines of Code**: ~5000+
- **Components**: 6 React components
- **API Endpoints**: 8 commands
- **Concurrent Games**: Unlimited
- **Response Time**: <200ms

## 🙏 Acknowledgments

- Giảng viên môn Lập trình mạng
- Open source community
- Stack Overflow contributors

---

<div align="center">

**⭐ Star this repo if you like it!**

Made with ❤️ by Huy & Hieu

[Report Bug](https://github.com/Hyuht17/BattleShip/issues) · [Request Feature](https://github.com/Hyuht17/BattleShip/issues)

</div>
