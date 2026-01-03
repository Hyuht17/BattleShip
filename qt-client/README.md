# Battleship Qt Client

Qt desktop client cho Battleship game, sử dụng C library thuần để kết nối TCP đến C++ server.

## Kiến trúc

```
Qt GUI (C++) → GameClient (C++ wrapper) → C Client Library (pure C) → TCP Socket → Server (C++)
```

**Luồng dữ liệu:**
1. User nhấn nút "Login" trên Qt Widget
2. Widget gọi `GameClient::sendMessage(json)`
3. GameClient gọi C function: `client_send(client, json)`
4. C library gửi qua TCP socket đến server port 8080
5. Server xử lý và response
6. C library nhận qua `client_receive()`
7. GameClient emit signal `messageReceived(json)`
8. Qt Widget cập nhật UI

**Thành phần:**
- **C Client Library** (`../client-lib/`): TCP socket thuần C, không phụ thuộc Qt
- **Qt Wrapper** (`GameClient.cpp`): C++ wrapper gọi C library qua `extern "C"`
- **Qt Widgets**: LoginWidget, LobbyWidget, GameWidget - chỉ làm UI

## Build

### Requirements
- Qt 6.x
- CMake 3.16+
- GCC/Clang với C11 và C++17

### Build steps

```bash
# Build C library
cd client-lib
make
./test_client  # Test kết nối (cần server đang chạy)

# Build Qt app
cd ../qt-client
mkdir build
cd build
cmake ..
make

# Run
./battleship_qt
```

## Cách hoạt động

1. **GameClient** (C++) tạo `BattleshipClient*` (C struct)
2. Gọi `client_connect()` để kết nối TCP đến server:8080
3. `QTimer` poll mỗi 100ms gọi `client_receive()` để đọc messages
4. Khi nhận JSON → emit signal `messageReceived()` → Qt widgets xử lý
5. Widgets gọi `sendMessage()` → gọi C `client_send()` → gửi JSON qua socket

## JSON Protocol

Giống với web version, tất cả messages đều là JSON:

### Client → Server
```json
{"type":"LOGIN","username":"user","password":"pass"}
{"type":"FIND_MATCH"}
{"type":"MOVE","row":3,"col":5}
{"type":"SURRENDER"}
```

### Server → Client
```json
{"type":"LOGIN_SUCCESS","user_id":1,"username":"user","elo":1000}
{"type":"MATCH_FOUND","opponent_name":"Player2","opponent_elo":1050}
{"type":"MOVE_RESULT","row":3,"col":5,"hit":true,"sunk":false}
{"type":"GAME_OVER","winner":"user","reason":"Opponent surrendered","elo_change":15}
```

## Features

- ✅ Direct TCP connection (không cần Node.js bridge)
- ✅ C library thuần có thể dùng cho CLI client
- ✅ Qt Widgets UI với Login, Lobby, Game screens
- ✅ Message buffering xử lý incomplete JSON
- ✅ Auto-reconnect và error handling
- ✅ Real-time game updates

## Testing

```bash
# Terminal 1: Run server
cd server
./server_full

# Terminal 2: Test C client
cd client-lib
./test_client

# Terminal 3: Run Qt app
cd qt-client/build
./battleship_qt
```
