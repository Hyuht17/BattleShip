#!/bin/bash
# Quick compile script for BattleShip server

echo "========================================="
echo "  Compiling BattleShip Server"
echo "========================================="
echo ""

echo "[1/2] Cleaning old binary..."
rm -f server_full

echo "[2/2] Compiling server_full.cpp..."
g++ -o server_full server_full.cpp -lpthread -std=c++11

if [ -f server_full ]; then
    echo ""
    echo "✓ Compilation successful!"
    echo ""
    echo "Run server with: ./server_full"
    echo "Server will listen on port 8080"
else
    echo ""
    echo "✗ Compilation failed!"
    exit 1
fi
















































