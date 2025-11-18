# BattleShip Makefile

.PHONY: all install build run clean help

# Colors for output
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[1;33m
NC=\033[0m # No Color

all: help

help:
	@echo "╔═══════════════════════════════════════╗"
	@echo "║   BattleShip Game - Build System     ║"
	@echo "╚═══════════════════════════════════════╝"
	@echo ""
	@echo "Available commands:"
	@echo "  make install    - Install all dependencies"
	@echo "  make build      - Build C++ server"
	@echo "  make run        - Start all servers"
	@echo "  make clean      - Clean build files"
	@echo "  make test       - Test connections"
	@echo ""

install:
	@echo "$(YELLOW)Installing dependencies...$(NC)"
	@cd node-server && npm install
	@cd frontend && npm install
	@echo "$(GREEN)✓ Dependencies installed$(NC)"

build:
	@echo "$(YELLOW)Building C++ server...$(NC)"
	@cd server && g++ -o server_full server_full.cpp -lpthread
	@echo "$(GREEN)✓ C++ server built$(NC)"

run:
	@echo "$(GREEN)Starting all servers...$(NC)"
	@chmod +x start-all.sh
	@./start-all.sh

clean:
	@echo "$(YELLOW)Cleaning build files...$(NC)"
	@rm -f server/server_full
	@rm -f server/users.dat
	@rm -rf node-server/node_modules
	@rm -rf frontend/node_modules
	@echo "$(GREEN)✓ Clean complete$(NC)"

test:
	@echo "$(YELLOW)Testing server connections...$(NC)"
	@echo "Testing C++ server (port 8080)..."
	@nc -zv localhost 8080 2>&1 || echo "C++ server not running"
	@echo "Testing Node.js server (port 3000)..."
	@nc -zv localhost 3000 2>&1 || echo "Node.js server not running"
	@echo "Testing Frontend (port 5173)..."
	@nc -zv localhost 5173 2>&1 || echo "Frontend not running"

# Development shortcuts
dev-cpp:
	@cd server && ./server_full

dev-node:
	@cd node-server && npm start

dev-frontend:
	@cd frontend && npm run dev
