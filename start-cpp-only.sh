#!/bin/bash

# Script khởi động cho kiến trúc: Chỉ C++ server public, các máy chạy frontend local

echo "╔═══════════════════════════════════════════════════╗"
echo "║  BattleShip - C++ Server Public Mode              ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""

IP=$(hostname -I | awk '{print $1}')

echo "🎯 Kiến trúc:"
echo "   - C++ Server: PUBLIC trong LAN (port 8080)"
echo "   - Node.js + Frontend: LOCAL trên từng máy"
echo ""
echo "📡 C++ Server IP: $IP:8080"
echo ""

# Check if already running
if lsof -i :8080 >/dev/null 2>&1; then
    echo "⚠️  Port 8080 đã được dùng!"
    echo "   Dừng process cũ..."
    pkill -f server_full
    sleep 1
fi

# Compile if needed
if [ ! -f "server/server_full" ]; then
    echo "🔨 Compiling C++ server..."
    cd server
    g++ -o server_full server_full.cpp -lpthread
    cd ..
fi

# Start C++ server
echo "🚀 Starting C++ Server..."
cd server
./server_full &
CPP_PID=$!
cd ..

sleep 2

echo ""
echo "✅ C++ Server started!"
echo ""
echo "╔═══════════════════════════════════════════════════╗"
echo "║  Hướng dẫn cho máy khác:                          ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""
echo "1️⃣  Trên máy khác, tạo file node-server/.env:"
echo ""
echo "    CPP_SERVER_HOST=$IP"
echo "    CPP_SERVER_PORT=8080"
echo "    NODE_SERVER_PORT=3000"
echo ""
echo "2️⃣  Chạy Node.js và Frontend:"
echo ""
echo "    cd node-server && npm start"
echo "    cd frontend && npm run dev"
echo ""
echo "3️⃣  Mở browser trên máy đó:"
echo ""
echo "    http://localhost:5173"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🔥 Firewall (nếu cần):"
echo "    sudo ufw allow 8080/tcp"
echo ""
echo "🧪 Test từ máy khác:"
echo "    telnet $IP 8080"
echo ""
echo "🛑 Press Ctrl+C to stop C++ server"
echo ""

# Cleanup on exit
trap "echo ''; echo '🛑 Stopping C++ server...'; kill $CPP_PID 2>/dev/null; echo '✅ Stopped'; exit" INT TERM

# Keep script running
wait
