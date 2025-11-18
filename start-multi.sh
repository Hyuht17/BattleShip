#!/bin/bash

# Script để khởi động server phù hợp với role của máy

show_usage() {
    echo "╔═══════════════════════════════════════════════════╗"
    echo "║  BattleShip Multi-Machine Deployment Script      ║"
    echo "╚═══════════════════════════════════════════════════╝"
    echo ""
    echo "Usage: $0 [role]"
    echo ""
    echo "Roles:"
    echo "  all           - Start all servers (C++, Node.js, Frontend)"
    echo "  cpp           - Start only C++ server"
    echo "  nodejs        - Start only Node.js server"
    echo "  frontend      - Start only Frontend"
    echo "  cpp-nodejs    - Start C++ and Node.js servers"
    echo ""
    echo "Examples:"
    echo "  $0 all              # Chạy tất cả trên 1 máy"
    echo "  $0 cpp              # Máy chuyên chạy C++ server"
    echo "  $0 nodejs           # Máy chuyên chạy Node.js proxy"
    echo "  $0 cpp-nodejs       # Máy chạy backend (C++ + Node.js)"
    echo "  $0 frontend         # Máy chạy frontend"
    echo ""
}

# Get IP
IP=$(hostname -I | awk '{print $1}')

start_cpp_server() {
    echo "🔨 Starting C++ Game Server..."
    cd server
    
    if [ ! -f "server_full" ]; then
        echo "   Compiling..."
        if [ -f "Makefile" ]; then
            make
        else
            g++ -o server_full server_full.cpp -lpthread
        fi
    fi
    
    echo "   C++ Server started on port 8080"
    echo "   📡 Other servers should connect to: $IP:8080"
    ./server_full &
    CPP_PID=$!
    cd ..
    sleep 2
}

start_nodejs_server() {
    echo "🔨 Starting Node.js Middleware..."
    cd node-server
    
    # Check if .env exists
    if [ ! -f ".env" ]; then
        echo ""
        echo "⚠️  No .env file found!"
        echo ""
        read -p "   Is C++ server on THIS machine? (y/n): " on_same_machine
        if [ "$on_same_machine" = "y" ] || [ "$on_same_machine" = "Y" ]; then
            echo "   Using localhost for C++ server"
        else
            echo ""
            echo "   Enter C++ server IP (the machine running server_full.cpp)"
            read -p "   C++ Server IP: " cpp_ip
            echo "CPP_SERVER_HOST=$cpp_ip" > .env
            echo "CPP_SERVER_PORT=8080" >> .env
            echo "NODE_SERVER_PORT=3000" >> .env
            echo ""
            echo "✅ Created .env file:"
            echo "   CPP_SERVER_HOST=$cpp_ip"
            echo "   Node.js will connect to C++ server at $cpp_ip:8080"
            echo ""
        fi
    else
        echo "   Found .env configuration:"
        grep CPP_SERVER_HOST .env || echo "   CPP_SERVER_HOST=localhost (default)"
    fi
    
    echo "   Node.js Server starting on port 3000..."
    echo "   📡 Clients should connect to: http://$IP:3000"
    npm start &
    NODE_PID=$!
    cd ..
    sleep 3
}

start_frontend() {
    echo "🔨 Starting React Frontend..."
    cd frontend
    
    echo "   Frontend started on port 5173"
    echo "   🌐 Access from browser: http://$IP:5173"
    npm run dev &
    VITE_PID=$!
    cd ..
    sleep 3
}

# Main logic
ROLE=${1:-help}

if [ "$ROLE" = "help" ] || [ "$ROLE" = "-h" ] || [ "$ROLE" = "--help" ]; then
    show_usage
    exit 0
fi

echo "╔═══════════════════════════════════════════════════╗"
echo "║  BattleShip Server - Multi-Machine Mode           ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""
echo "🌐 Server IP: $IP"
echo "📋 Role: $ROLE"
echo ""

case "$ROLE" in
    all)
        start_cpp_server
        start_nodejs_server
        start_frontend
        echo ""
        echo "✅ All servers started!"
        echo ""
        echo "📊 Access URLs:"
        echo "   - This machine:  http://localhost:5173"
        echo "   - LAN devices:   http://$IP:5173"
        echo ""
        echo "📡 Server Endpoints:"
        echo "   - C++ Server:    $IP:8080 (TCP)"
        echo "   - Node Server:   http://$IP:3000 (WebSocket)"
        echo "   - Frontend:      http://$IP:5173 (HTTP)"
        ;;
    
    cpp)
        start_cpp_server
        echo ""
        echo "✅ C++ Server started!"
        echo ""
        echo "📡 C++ Server listening on: $IP:8080"
        echo ""
        echo "⚙️  Next steps:"
        echo "   On Node.js server machine, set CPP_SERVER_HOST=$IP in .env"
        echo "   Then run: ./start-multi.sh nodejs"
        ;;
    
    nodejs)
        start_nodejs_server
        echo ""
        echo "✅ Node.js Server started!"
        echo ""
        echo "📡 Node.js Server listening on: http://$IP:3000"
        echo ""
        echo "⚙️  Next steps:"
        echo "   On frontend machine, run: ./start-multi.sh frontend"
        echo "   Or clients can access directly: http://$IP:5173"
        ;;
    
    frontend)
        start_frontend
        echo ""
        echo "✅ Frontend started!"
        echo ""
        echo "🌐 Access from browser: http://$IP:5173"
        ;;
    
    cpp-nodejs)
        start_cpp_server
        start_nodejs_server
        echo ""
        echo "✅ Backend servers started!"
        echo ""
        echo "📡 Server Endpoints:"
        echo "   - C++ Server:    $IP:8080 (TCP)"
        echo "   - Node Server:   http://$IP:3000 (WebSocket)"
        echo ""
        echo "⚙️  Next steps:"
        echo "   Clients can access: http://$IP:5173"
        echo "   (Make sure frontend is running on this or another machine)"
        ;;
    
    *)
        echo "❌ Invalid role: $ROLE"
        echo ""
        show_usage
        exit 1
        ;;
esac

if [ -n "$CPP_PID" ] || [ -n "$NODE_PID" ] || [ -n "$VITE_PID" ]; then
    echo ""
    echo "🛑 Press Ctrl+C to stop all servers"
    echo ""
    
    # Cleanup on exit
    trap "echo ''; echo '🛑 Stopping servers...'; [ -n '$CPP_PID' ] && kill $CPP_PID 2>/dev/null; [ -n '$NODE_PID' ] && kill $NODE_PID 2>/dev/null; [ -n '$VITE_PID' ] && kill $VITE_PID 2>/dev/null; echo '✅ Stopped'; exit" INT TERM
    
    # Keep script running
    wait
fi
