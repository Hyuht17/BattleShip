# BattleShip Game - C Client

## Mô tả

C client kết nối TCP đến C++ server, giao tiếp với Node.js qua stdin/stdout.

## Kiến trúc

```
Browser (React)
    ↕ WebSocket
Node.js Bridge
    ↕ stdin/stdout (IPC)
C Client  ← BẠN Ở ĐÂY
    ↕ TCP Socket
C++ Server
```

## Build

```bash
make
```

## Chạy thủ công (để test)

```bash
./client
```

Sau đó gõ JSON và Enter để gửi:
```json
{"cmd":"LOGIN","payload":{"username":"test","password":"123"}}
```

## Code chính

- `client.c`: Code C client
- 2 threads:
  - `receive_thread`: Nhận data từ C++ server → printf stdout
  - `stdin_thread`: Đọc stdin → gửi đến C++ server
