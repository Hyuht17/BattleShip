# 🚢 BattleShip Network Game

> Trò chơi Hải chiến mạng đa người chơi với Qt Desktop Client

[![Qt](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)
[![C](https://img.shields.io/badge/C-11-blue.svg)](https://en.cppreference.com/w/c)
[![C++](https://img.shields.io/badge/C++-11+-red.svg)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-Active-success.svg)]()

## 📖 Giới thiệu

**BattleShip Network Game** là đồ án môn Lập trình mạng, implement trò chơi Hải chiến (Battleship) với Qt desktop GUI:

- 🎮 Chơi mạng real-time với nhiều người
- 🔐 Hệ thống đăng ký/đăng nhập
- ⚔️ Matchmaking tự động theo ELO
- 🚢 Đặt tàu với validation đầy đủ
- 💥 Đánh tàu trên lưới 10x10
- 🏆 Hệ thống ELO ranking
- 📊 Leaderboard và match history
- 🖥️ Desktop native app với Qt

## 🏗️ Kiến trúc - Đơn giản hơn (Bỏ Node.js & React)

```
┌─────────────┐    ┌─────────────┐           ┌──────────────┐           
│  Qt Client  │    │  Qt Client  │    ...    │  Qt Client   │           
│  (Desktop)  │    │  (Desktop)  │           │  (Desktop)   │           
└──────┬──────┘    └──────┬──────┘           └──────┬───────┘           
       │                  │                         │                    
       │ TCP Socket       │ TCP Socket              │ TCP Socket         
       │                  │                         │                    
       └──────────────────┴─────────────────────────┘                    
                          │                                              
              ┌───────────▼────────────┐                 ┌─────────────┐
              │   C++ Game Server      │◄────────────────┤  Storage    │
              │      (Port 8080)       │                 │  (File DB)  │
              │  • Authentication      │                 │             │
              │  • Game matching       │                 └─────────────┘
              │  • Move validation     │                                
              │  • Win condition       │                                
              │  • All game logic      │                                
              └────────────────────────┘                                

Luồng:  Qt GUI → C Library (TCP) → C++ Server
```

### Vai trò từng thành phần:

| Thành phần | Vai trò | Công nghệ | Port |
|------------|---------|-----------|------|
| **C++ Server** | ⚙️ Core game engine - Xử lý toàn bộ logic game | C++11, POSIX Sockets, pthread | 8080 |
| **Node.js Middleware** | 🔄 Protocol converter - Chỉ chuyển đổi WebSocket ↔ TCP | Node.js, Express, Socket.IO | 3000 |
| **React Frontend** | 🎨 User interface - Hiển thị và tương tác | React 19, Vite, CSS3 | 5173 |

### Thành phần chính:

1. **C++ TCP Server** - Core game logic, authentication, state management
2. **Node.js Middleware** - WebSocket ↔ TCP bridge, message routing (no business logic)
3. **React Frontend** - Modern UI with real-time updates

## 📡 Deployment Options

### Tùy chọn 1: Tất cả trên 1 máy (Development)
```bash
./start-all.sh
# hoặc
./start-multi.sh all
```

### Tùy chọn 2: Multi-machine LAN Setup
```bash
# Máy 1: C++ Server
./start-multi.sh cpp

# Máy 2: Node.js + Frontend
./start-multi.sh nodejs
./start-multi.sh frontend
```

📚 **Chi tiết:** Xem [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)

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
- [x] **LAN multiplayer support** 🌐
- [x] **Multi-machine deployment** 🖥️

## 🌐 LAN & Multi-Machine Setup

### Quick LAN Setup (Tất cả máy cùng mạng WiFi)

```bash
# Trên máy server
./start-lan.sh

# Output sẽ hiển thị IP, ví dụ: 192.168.1.100
# Các máy khác truy cập: http://192.168.1.100:5173
```

### Advanced Multi-Machine Setup

#### Kịch bản 1: Máy backend + Máy client

```bash
# Máy 1 (Backend): Chạy C++ + Node.js
./start-multi.sh cpp-nodejs

# Máy 2 (Client): Mở browser
# Truy cập: http://[IP_MÁY_1]:5173
```

#### Kịch bản 2: Mỗi service 1 máy riêng

```bash
# Máy 1: C++ Server only
./start-multi.sh cpp

# Máy 2: Node.js Middleware
# Tạo file .env: echo 'CPP_SERVER_HOST=192.168.1.100' > node-server/.env
./start-multi.sh nodejs

# Máy 3: Frontend
./start-multi.sh frontend

# Clients: Truy cập http://[IP_MÁY_2]:5173
```

#### Test kết nối

```bash
./test-connections.sh
```

📚 **Hướng dẫn chi tiết:** 
- [LAN_SETUP.md](LAN_SETUP.md) - Cấu hình LAN đơn giản
- [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) - Triển khai multi-machine chuyên sâu

## 🔧 Configuration

### Node.js Server (.env)
```bash
CPP_SERVER_HOST=localhost    # IP của C++ server
CPP_SERVER_PORT=8080          # Port của C++ server
NODE_SERVER_PORT=3000         # Port của Node.js
```

### Firewall (nếu cần)
```bash
sudo ufw allow 8080/tcp   # C++ Server
sudo ufw allow 3000/tcp   # Node.js
sudo ufw allow 5173/tcp   # Frontend
```

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
