#!/bin/bash

# BattleShip Game - Start All Servers Script
# This script starts all three servers in separate terminals

echo "╔═══════════════════════════════════════╗"
echo "║  Starting BattleShip Game Servers    ║"
echo "╚═══════════════════════════════════════╝"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if C++ server is compiled
if [ ! -f "$SCRIPT_DIR/server/server_full" ]; then
    echo -e "${YELLOW}Compiling C++ server...${NC}"
    cd "$SCRIPT_DIR/server"
    g++ -o server_full server_full.cpp -lpthread
    if [ $? -ne 0 ]; then
        echo "Failed to compile C++ server!"
        exit 1
    fi
    cd "$SCRIPT_DIR"
fi

# Check if dependencies are installed
if [ ! -d "$SCRIPT_DIR/node-server/node_modules" ]; then
    echo -e "${YELLOW}Installing node-server dependencies...${NC}"
    cd "$SCRIPT_DIR/node-server"
    npm install
    cd "$SCRIPT_DIR"
fi

if [ ! -d "$SCRIPT_DIR/frontend/node_modules" ]; then
    echo -e "${YELLOW}Installing frontend dependencies...${NC}"
    cd "$SCRIPT_DIR/frontend"
    npm install
    cd "$SCRIPT_DIR"
fi

echo -e "${GREEN}Starting servers...${NC}"

# Start C++ server
gnome-terminal --tab --title="C++ Server" -- bash -c "cd '$SCRIPT_DIR/server' && ./server_full; exec bash" 2>/dev/null || \
xterm -T "C++ Server" -e "cd '$SCRIPT_DIR/server' && ./server_full; bash" 2>/dev/null || \
osascript -e 'tell app "Terminal" to do script "cd '"$SCRIPT_DIR/server"' && ./server_full"' 2>/dev/null &

sleep 2

# Start Node.js server
gnome-terminal --tab --title="Node.js Server" -- bash -c "cd '$SCRIPT_DIR/node-server' && npm start; exec bash" 2>/dev/null || \
xterm -T "Node.js Server" -e "cd '$SCRIPT_DIR/node-server' && npm start; bash" 2>/dev/null || \
osascript -e 'tell app "Terminal" to do script "cd '"$SCRIPT_DIR/node-server"' && npm start"' 2>/dev/null &

sleep 2

# Start React frontend
gnome-terminal --tab --title="React Frontend" -- bash -c "cd '$SCRIPT_DIR/frontend' && npm run dev; exec bash" 2>/dev/null || \
xterm -T "React Frontend" -e "cd '$SCRIPT_DIR/frontend' && npm run dev; bash" 2>/dev/null || \
osascript -e 'tell app "Terminal" to do script "cd '"$SCRIPT_DIR/frontend"' && npm run dev"' 2>/dev/null &

echo ""
echo -e "${GREEN}╔═══════════════════════════════════════╗${NC}"
echo -e "${GREEN}║     All Servers Started!              ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════╝${NC}"
echo ""
echo "C++ Server:      localhost:8080"
echo "Node.js Server:  localhost:3000"
echo "Frontend:        http://localhost:5173"
echo ""
echo "Press Ctrl+C in each terminal to stop the servers"
