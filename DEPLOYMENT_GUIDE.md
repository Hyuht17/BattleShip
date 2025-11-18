# Kiến trúc hệ thống BattleShip - Multi-Machine Setup

## 📐 Kiến trúc 3 tầng

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Client 1      │         │   Client 2       │         │   Client N      │
│  (Browser)      │         │  (Browser)       │         │  (Browser)      │
│  React Frontend │         │  React Frontend  │         │  React Frontend │
└────────┬────────┘         └────────┬─────────┘         └────────┬────────┘
         │                           │                            │
         │         WebSocket         │                            │
         └───────────────┬───────────┴────────────────────────────┘
                         │
                    ┌────▼─────────────┐
                    │   Node.js Server │  ← Port 3000 (WebSocket ↔ TCP Bridge)
                    │   (server.js)    │    Chỉ chuyển đổi protocol, không xử lý logic
                    └────────┬─────────┘
                             │
                             │ TCP Socket
                             │
                    ┌────────▼─────────┐
                    │  C++ Game Server │  ← Port 8080 (TCP Server)
                    │ (server_full.cpp)│    Xử lý toàn bộ game logic
                    │                  │    - Authentication
                    │  ┌────────────┐  │    - Game matching
                    │  │ Game Logic │  │    - Move validation
                    │  │ Storage    │  │    - Win condition
                    │  └────────────┘  │
                    └──────────────────┘
```

## 🖥️ Các kịch bản triển khai

### Kịch bản 1: Tất cả trên 1 máy (Development)
```
[Máy Server - 192.168.1.100]
├── C++ Server (port 8080)
├── Node.js Server (port 3000)
└── React Frontend (port 5173)

[Máy Client 1 - 192.168.1.101]
└── Browser → http://192.168.1.100:5173

[Máy Client 2 - 192.168.1.102]
└── Browser → http://192.168.1.100:5173
```

**Cấu hình:**
- Node.js: `CPP_SERVER_HOST=localhost`
- Frontend: Auto-detect (sẽ dùng 192.168.1.100:3000)

### Kịch bản 2: C++ Server riêng, Node.js riêng (Production-like)
```
[Máy Server 1 - 192.168.1.100]
└── C++ Server (port 8080)

[Máy Server 2 - 192.168.1.101]
├── Node.js Server (port 3000)
└── React Frontend (port 5173)

[Máy Client 1 - 192.168.1.102]
└── Browser → http://192.168.1.101:5173

[Máy Client 2 - 192.168.1.103]
└── Browser → http://192.168.1.101:5173
```

**Cấu hình:**
- Node.js: `CPP_SERVER_HOST=192.168.1.100`
- Frontend: Auto-detect (sẽ dùng 192.168.1.101:3000)

### Kịch bản 3: Load Balancing (Multiple Node.js)
```
[Máy Server - 192.168.1.100]
└── C++ Server (port 8080)

[Máy Proxy 1 - 192.168.1.101]
└── Node.js Server (port 3000)

[Máy Proxy 2 - 192.168.1.102]
└── Node.js Server (port 3000)

[Load Balancer - 192.168.1.103]
└── Nginx/HAProxy → round-robin đến Node.js servers
```

## 🚀 Hướng dẫn triển khai

### Bước 1: Khởi động C++ Server (Máy Server chính)

```bash
cd server

# Compile
g++ -o server_full server_full.cpp -lpthread

# Chạy server
./server_full
```

Kiểm tra IP của máy này:
```bash
hostname -I
# Ví dụ output: 192.168.1.100
```

### Bước 2: Cấu hình Node.js Server

**Nếu cùng máy với C++ Server:**
```bash
cd node-server
# Không cần làm gì, dùng localhost mặc định
npm start
```

**Nếu khác máy với C++ Server:**
```bash
cd node-server

# Tạo file .env
cat > .env << EOF
CPP_SERVER_HOST=192.168.1.100
CPP_SERVER_PORT=8080
NODE_SERVER_PORT=3000
EOF

# Khởi động
npm start
```

Kiểm tra log phải hiện:
```
📡 C++ Server: 192.168.1.100:8080
🌐 Server LAN IP: 192.168.1.101
✅ Node.js server listening on:
   - Local: http://localhost:3000
   - LAN:   http://192.168.1.101:3000
```

### Bước 3: Khởi động Frontend

```bash
cd frontend
npm run dev
```

Frontend sẽ tự động detect và kết nối đến Node.js server dựa trên hostname.

### Bước 4: Test kết nối từ Client

**Từ máy client bất kỳ:**
1. Mở browser
2. Truy cập: `http://[NODE_SERVER_IP]:5173`
3. Đăng ký/Đăng nhập
4. Chơi game!

## 🔧 Troubleshooting theo tầng

### Kiểm tra C++ Server
```bash
# Kiểm tra C++ server đang chạy
netstat -tlnp | grep 8080

# Test kết nối đến C++ server
telnet 192.168.1.100 8080

# Xem log C++ server
# (sẽ hiện trong terminal đang chạy ./server_full)
```

### Kiểm tra Node.js Server
```bash
# Kiểm tra Node.js đang chạy
netstat -tlnp | grep 3000

# Test health endpoint
curl http://192.168.1.101:3000/health

# Xem log Node.js
# (sẽ hiện trong terminal đang chạy npm start)
```

### Kiểm tra Frontend
```bash
# Kiểm tra Vite đang chạy
netstat -tlnp | grep 5173

# Test truy cập
curl http://192.168.1.101:5173
```

### Kiểm tra kết nối end-to-end

**Mở browser console (F12) và xem:**
```
🌐 Connecting to: http://192.168.1.101:3000
Connected to Node.js server
Connected to C++ game server: {message: "Welcome to BattleShip Server"}
```

## 🔥 Firewall Configuration

### Trên máy chạy C++ Server:
```bash
sudo ufw allow 8080/tcp
```

### Trên máy chạy Node.js Server:
```bash
sudo ufw allow 3000/tcp
sudo ufw allow 5173/tcp
```

### Kiểm tra firewall:
```bash
sudo ufw status
```

## 📊 Monitoring

### Xem số connection đến C++ Server:
```bash
netstat -ant | grep 8080 | grep ESTABLISHED | wc -l
```

### Xem số connection đến Node.js:
```bash
netstat -ant | grep 3000 | grep ESTABLISHED | wc -l
```

### Xem real-time traffic:
```bash
# Install tcpdump nếu chưa có
sudo apt-get install tcpdump

# Monitor C++ server traffic
sudo tcpdump -i any port 8080 -A

# Monitor Node.js traffic
sudo tcpdump -i any port 3000 -A
```

## 🎯 Performance Tips

1. **Giữ Node.js và C++ Server gần nhau** (cùng switch/LAN segment) để giảm latency
2. **Dùng kết nối có dây** thay vì WiFi cho server
3. **Nếu có nhiều client**, cân nhắc deploy multiple Node.js servers
4. **Monitor CPU/Memory** của C++ server vì nó xử lý toàn bộ logic

## 🔐 Security Notes

1. **Trong môi trường production**, nên thêm:
   - SSL/TLS cho WebSocket (wss://)
   - Authentication token
   - Rate limiting
   - Input validation

2. **Firewall rules** nên chặn direct access đến C++ server từ bên ngoài:
   ```bash
   # Chỉ cho phép Node.js server kết nối đến C++ server
   sudo ufw allow from 192.168.1.101 to any port 8080 proto tcp
   ```

## 📝 Environment Variables

### Node.js Server (.env)
```bash
CPP_SERVER_HOST=192.168.1.100    # IP của máy chạy C++ server
CPP_SERVER_PORT=8080              # Port của C++ server
NODE_SERVER_PORT=3000             # Port của Node.js server
```

### Frontend
Không cần config! Auto-detect dựa trên `window.location.hostname`

## 🆘 Common Issues

### Issue: "Cannot connect to C++ server"
**Solution:**
1. Kiểm tra C++ server có đang chạy: `netstat -tlnp | grep 8080`
2. Kiểm tra firewall: `sudo ufw status`
3. Test telnet: `telnet [CPP_IP] 8080`
4. Xem log Node.js có error gì không

### Issue: "CORS error"
**Solution:**
- Node.js server đã config CORS cho LAN rồi
- Hard refresh browser: Ctrl+Shift+R
- Clear cache và thử lại

### Issue: "Connection timeout"
**Solution:**
1. Ping giữa các máy: `ping [IP]`
2. Kiểm tra cùng subnet: `ip addr show`
3. Kiểm tra router không block

## ✅ Quick Test Script

```bash
#!/bin/bash
# test-connections.sh

echo "Testing BattleShip System..."

# Test C++ Server
echo -n "C++ Server (8080): "
nc -zv 192.168.1.100 8080 2>&1 | grep succeeded && echo "✅" || echo "❌"

# Test Node.js Server
echo -n "Node.js Server (3000): "
curl -s http://192.168.1.101:3000/health | grep ok && echo "✅" || echo "❌"

# Test Frontend
echo -n "Frontend (5173): "
curl -s -o /dev/null -w "%{http_code}" http://192.168.1.101:5173 | grep 200 && echo "✅" || echo "❌"
```

## 📞 Support

Nếu gặp vấn đề, thu thập thông tin sau:
1. IP của tất cả các máy: `hostname -I`
2. Ports đang listen: `netstat -tlnp`
3. Firewall status: `sudo ufw status`
4. Log từ C++ server
5. Log từ Node.js server
6. Browser console log (F12)

Good luck! 🚀
