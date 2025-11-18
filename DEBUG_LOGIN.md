# 🔍 Hướng dẫn Debug - Login Issue

## Vấn đề: Không thể login

### ✅ Backend đã test OK
- C++ server hoạt động: ✅
- Node.js middleware: ✅  
- Protocol communication: ✅

### 🧪 Test backend thủ công:
```bash
cd node-server
node test-login.js
```

Nếu thấy "✅ SUCCESS! Login worked!" thì backend OK.

---

## 🌐 Kiểm tra Frontend

### Bước 1: Mở Browser Console

1. Mở http://localhost:5173
2. Nhấn **F12** (hoặc Right-click → Inspect)
3. Chọn tab **Console**

### Bước 2: Thử đăng ký user mới

1. Click "Register here"
2. Nhập:
   - Username: `player1`
   - Password: `pass123`
3. Click "Register"

**Xem trong Console:**
```
[REGISTER] Attempting register: player1
[SERVER MESSAGE] { cmd: "REGISTER_SUCCESS", ... }
```

Nếu thấy "Registration successful!" alert → OK!

### Bước 3: Thử login

1. Click "Login here" (nếu đang ở Register)
2. Nhập **chính xác** username/password vừa đăng ký:
   - Username: `player1`
   - Password: `pass123`
3. Click "Login"

**Xem trong Console:**
```
[LOGIN] Attempting login: player1
[SERVER MESSAGE] { cmd: "LOGIN_SUCCESS", payload: { username: "player1" } }
[LOGIN SUCCESS] User: player1
```

---

## ❌ Nếu vẫn lỗi

### Lỗi 1: "Invalid credentials"

**Nguyên nhân:** Username/password không đúng

**Giải pháp:**
1. Đảm bảo đã đăng ký user trước
2. Nhập đúng chính xác username/password
3. Không có khoảng trắng thừa
4. Phân biệt chữ hoa/thường

### Lỗi 2: Không có response trong console

**Nguyên nhân:** Socket không kết nối

**Kiểm tra:**
```javascript
// Trong console, gõ:
socketRef.current.connected
// Nếu trả về true → OK
// Nếu false → Server không chạy
```

**Giải pháp:**
```bash
# Restart tất cả servers
./start-all.sh
```

### Lỗi 3: "Connection refused"

**Nguyên nhân:** Servers không chạy

**Kiểm tra:**
```bash
lsof -i :8080  # C++ server
lsof -i :3000  # Node.js
lsof -i :5173  # Frontend

# Nếu trống → Server không chạy
```

**Giải pháp:**
```bash
./start-all.sh
```

---

## 🔧 Debug Commands

### 1. Check server processes
```bash
ps aux | grep -E "(server_full|node|vite)"
```

### 2. Check server logs (if running in terminals)
- Terminal 1: C++ server output
- Terminal 2: Node.js server output  
- Terminal 3: Vite frontend output

### 3. Test raw socket connection
```bash
cd node-server
node test-login.js
```

### 4. Check users database
```bash
cat server/users.dat
# Sẽ thấy: username:password:score:games:wins
```

### 5. Clear users (if needed)
```bash
rm server/users.dat
# Restart C++ server
```

---

## 📸 Screenshot Browser Console

**Khi login thành công, bạn sẽ thấy:**

```
Connected to Node.js server
Connected to C++ game server: { message: 'Connected to game server' }
Message from server: { cmd: "WELCOME", payload: {...} }
[LOGIN] Attempting login: player1
Message from server: { cmd: "LOGIN_SUCCESS", payload: { username: "player1" } }
[LOGIN SUCCESS] User: player1
```

**Screen sẽ chuyển từ Login → Lobby!**

---

## ✅ Checklist

- [ ] C++ server đang chạy (port 8080)
- [ ] Node.js server đang chạy (port 3000)
- [ ] Frontend đang chạy (port 5173)
- [ ] Browser console không có error màu đỏ
- [ ] Socket.IO connected (xem status trên header)
- [ ] Đã đăng ký user trước khi login
- [ ] Username/password nhập đúng

---

## 🆘 Vẫn không được?

**Thử scenario hoàn chỉnh:**

```bash
# Terminal 1
cd server && ./server_full

# Terminal 2  
cd node-server && npm start

# Terminal 3
cd frontend && npm run dev

# Browser
# 1. Mở http://localhost:5173
# 2. F12 → Console tab
# 3. Register: player1 / pass123
# 4. Login: player1 / pass123
# 5. Xem console logs
```

**Nếu vẫn lỗi, copy toàn bộ console logs và check!**
