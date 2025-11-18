#!/bin/bash

# Lấy IP của máy
IP=$(hostname -I | awk '{print $1}')

echo "╔════════════════════════════════════════╗"
echo "║  BattleShip Server - LAN Mode          ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "🌐 Server IP: $IP"
echo ""
echo "📱 Để kết nối từ máy khác trong LAN:"
echo "   Mở browser và truy cập: http://$IP:5173"
echo ""
echo "⚙️  Starting servers..."
echo ""

# Check if server binary exists
if [ ! -f "server/server_full" ]; then
    echo "🔨 Compiling C++ server..."
    cd server
    if [ -f "Makefile" ]; then
        make
    else
        g++ -o server_full server_full.cpp -lpthread
    fi
    cd ..
fi

# Start C++ server
echo "🚀 Starting C++ server on port 8080..."
cd server
./server_full &
CPP_PID=$!
cd ..

sleep 2

# Start Node.js server
echo "🚀 Starting Node.js middleware on port 3000..."
cd node-server
npm start &
NODE_PID=$!
cd ..

sleep 3

# Start Frontend with host flag
echo "🚀 Starting React frontend on port 5173..."
cd frontend
npm run dev &
VITE_PID=$!
cd ..

sleep 3

echo ""
echo "✅ All servers started!"
echo ""
echo "📊 Access URLs:"
echo "   - This machine:  http://localhost:5173"
echo "   - LAN devices:   http://$IP:5173"
echo ""
echo "🎮 Game Servers:"
echo "   - C++ Server:    localhost:8080"
echo "   - Node Server:   http://$IP:3000"
echo ""
echo "🛑 Press Ctrl+C to stop all servers"
echo ""

# Cleanup on exit
trap "echo ''; echo '🛑 Stopping all servers...'; kill $CPP_PID $NODE_PID $VITE_PID 2>/dev/null; echo '✅ All servers stopped'; exit" INT TERM

# Keep script running
wait
