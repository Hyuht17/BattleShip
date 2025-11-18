# Hướng dẫn kết nối LAN - BattleShip Game

## 🌐 Cấu hình cho LAN đã hoàn tất!

Các file đã được cập nhật:
- ✅ `node-server/server.js` - Hỗ trợ CORS cho tất cả IP trong LAN
- ✅ `frontend/src/config.js` - Auto-detect server URL
- ✅ `frontend/vite.config.js` - Cho phép truy cập từ LAN
- ✅ `start-lan.sh` - Script khởi động tự động

## 🚀 Cách sử dụng

### Trên máy Server (máy chạy game server):

```bash
# Cấp quyền thực thi cho script
chmod +x start-lan.sh

# Khởi động server
./start-lan.sh
```

Script sẽ hiển thị IP của máy, ví dụ:
```
🌐 Server IP: 192.168.1.100
📱 Để kết nối từ máy khác trong LAN:
   Mở browser và truy cập: http://192.168.1.100:5173
```

### Trên máy Client (máy khác trong cùng mạng):

1. Mở trình duyệt (Chrome, Firefox, Edge...)
2. Truy cập: `http://192.168.1.100:5173` (thay 192.168.1.100 bằng IP thực của server)
3. Đăng ký/Đăng nhập
4. Thách đấu và chơi!

## 🔥 Firewall (Quan trọng!)

Nếu không kết nối được, cần mở các port:

```bash
# Ubuntu/Debian
sudo ufw allow 8080/tcp  # C++ Game Server
sudo ufw allow 3000/tcp  # Node.js Middleware
sudo ufw allow 5173/tcp  # React Frontend

# Kiểm tra firewall
sudo ufw status

# Hoặc tạm thời tắt firewall để test (không khuyến khích)
sudo ufw disable
```

## 🔍 Troubleshooting

### 1. Kiểm tra IP của server
```bash
hostname -I
# hoặc
ip addr show
```

### 2. Kiểm tra các port đang chạy
```bash
netstat -tlnp | grep -E '8080|3000|5173'
```

### 3. Test ping từ client đến server
```bash
ping 192.168.1.100
```

### 4. Test kết nối port từ client
```bash
# Test Node.js server
curl http://192.168.1.100:3000/health

# Test với telnet
telnet 192.168.1.100 3000
```

### 5. Kiểm tra browser console
- Mở DevTools (F12)
- Xem tab Console có lỗi CORS không
- Xem tab Network để kiểm tra requests

### 6. Nếu vẫn lỗi CORS
- Hard refresh browser: `Ctrl+Shift+R`
- Xóa cache: `Ctrl+Shift+Del`
- Thử browser khác
- Kiểm tra console log có hiện IP đúng không:
  ```
  🌐 Connecting to: http://192.168.1.100:3000
  ```

## 📝 Lưu ý

1. **Cùng mạng**: Tất cả các máy phải trong cùng mạng LAN (cùng WiFi hoặc cùng switch)
2. **IP động**: IP có thể thay đổi sau khi khởi động lại router. Dùng IP tĩnh nếu cần:
   ```bash
   # Xem IP hiện tại
   ip addr show
   
   # Đặt IP tĩnh (Ubuntu)
   sudo nano /etc/netplan/01-network-manager-all.yaml
   ```

3. **Port forwarding**: Nếu muốn chơi qua Internet (không chỉ LAN), cần cấu hình port forwarding trên router.

## 🎮 Test kết nối

1. Trên máy server, mở: `http://localhost:5173`
2. Trên máy client, mở: `http://[SERVER_IP]:5173`
3. Đăng ký 2 tài khoản khác nhau
4. Thách đấu và chơi!

## ⚡ Performance Tips

- Kết nối có dây (Ethernet) tốt hơn WiFi
- Đảm bảo không có máy nào download/upload nặng
- Router quality ảnh hưởng đến latency

## 📞 Support

Nếu gặp vấn đề, kiểm tra:
1. Firewall đã mở chưa
2. Tất cả servers đang chạy chưa
3. IP có đúng không
4. Console có lỗi gì không

Good luck! 🚀
