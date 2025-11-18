# Quick Start Guide - BattleShip Network Game

## ⚡ Bắt đầu nhanh (3 bước)

### Bước 1: Update Node.js (BẮT BUỘC!)
```bash
# Kiểm tra version hiện tại
node --version

# Nếu < v14, cài NVM và update:
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
source ~/.bashrc
nvm install 20
nvm use 20
```

### Bước 2: Cài đặt và build
```bash
# Cài dependencies và build
make install
make build

# Hoặc thủ công:
cd node-server && npm install && cd ..
cd frontend && npm install && cd ..
cd server && g++ -o server_full server_full.cpp -lpthread && cd ..
```

### Bước 3: Chạy game!
```bash
# Cách tự động (khuyến nghị)
./start-all.sh

# Hoặc thủ công (3 terminals riêng biệt):
# Terminal 1:
cd server && ./server_full

# Terminal 2:
cd node-server && npm start

# Terminal 3:
cd frontend && npm run dev
```

Mở trình duyệt: **http://localhost:5173**

---

## 🎮 Hướng dẫn chơi game

### 1. Đăng ký (lần đầu)
- Click "Register here"
- Nhập username và password (ví dụ: `player1` / `pass123`)
- Click "Register"

### 2. Đăng nhập
- Nhập username và password vừa đăng ký
- Click "Login"

### 3. Tìm đối thủ
- Bạn sẽ thấy màn hình Lobby
- Đợi hoặc mở tab mới để tạo player thứ 2
- Click "Refresh" để cập nhật danh sách
- Click "Challenge" để thách đấu

### 4. Đặt tàu (5 con tàu)
- **Carrier**: 5 ô
- **Battleship**: 4 ô  
- **Cruiser**: 3 ô
- **Submarine**: 3 ô
- **Destroyer**: 2 ô

**Cách đặt:**
1. Chọn "Horizontal" hoặc "Vertical"
2. Click vào ô trên bảng để đặt
3. Sau khi đặt hết 5 tàu, click "Confirm Placement"

### 5. Chơi game
- Đến lượt bạn: Click vào bảng **Opponent's Board**
- 💥 = Hit (trúng tàu)
- 💧 = Miss (trượt)
- Đánh chìm hết tàu đối thủ → **BẠN THẮNG!** 🎉

### 6. Chat
- Nhập tin nhắn ở ô chat
- Nhấn Enter hoặc click 📤

---

## 🔍 Kiểm tra lỗi

### Không kết nối được?
```bash
# Kiểm tra servers đang chạy:
lsof -i :8080  # C++ server
lsof -i :3000  # Node.js server  
lsof -i :5173  # Frontend
```

### Port bị chiếm?
```bash
# Kill process:
kill -9 $(lsof -t -i:8080)
kill -9 $(lsof -t -i:3000)
kill -9 $(lsof -t -i:5173)
```

### Lỗi Node.js "Unexpected token import"?
→ Node.js quá cũ! Quay lại Bước 1

---

## 📁 Cấu trúc file quan trọng

```
Project/
├── server/server_full.cpp    # C++ game server
├── node-server/server.js     # Node.js middleware
├── frontend/src/
│   ├── App.jsx               # Main React app
│   └── components/           # UI components
├── start-all.sh              # Start script (Linux/Mac)
├── start-all.bat             # Start script (Windows)
└── README_COMPLETE.md        # Full documentation
```

---

## 🎯 Demo nhanh (1 máy, 2 người chơi)

**Tab 1 - Player 1:**
1. Đăng ký: `player1` / `123`
2. Đăng nhập
3. Đợi ở lobby

**Tab 2 - Player 2:**
1. Mở tab mới: http://localhost:5173
2. Đăng ký: `player2` / `123`
3. Đăng nhập
4. Click "Challenge" player1

**Cả 2 tabs:**
- Đặt tàu
- Chơi game!

---

## 💡 Tips

- **Refresh lobby** thường xuyên để thấy người chơi mới
- **Đặt tàu** cẩn thận, không được chồng lên nhau
- **Chú ý lượt chơi**: 🟢 = lượt bạn, 🔴 = lượt đối thủ
- **Chat** để giao lưu với đối thủ!

---

## 🆘 Cần help?

1. Đọc `README_COMPLETE.md` để biết chi tiết
2. Kiểm tra logs trong terminal
3. Check browser console (F12)

**Chúc bạn chơi game vui vẻ! 🎮🚢**
