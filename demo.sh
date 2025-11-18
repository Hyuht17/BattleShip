#!/bin/bash

# ╔═══════════════════════════════════════════════════════════════╗
# ║  BattleShip Network Game - Complete Project Demonstration    ║
# ║  Automated Demo Script                                        ║
# ╚═══════════════════════════════════════════════════════════════╝

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

clear

echo -e "${PURPLE}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                                                               ║"
echo "║        🚢  BattleShip Network Game  🚢                        ║"
echo "║                                                               ║"
echo "║         Đồ án môn Lập trình mạng                             ║"
echo "║                                                               ║"
echo "║    Nhóm thực hiện:                                            ║"
echo "║    - Đặng Quang Huy                                           ║"
echo "║    - Lê Bá Ngọc Hiểu                                          ║"
echo "║                                                               ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Function to print section header
print_section() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
}

# Function to print success
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Function to print error
print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Function to print info
print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Function to print warning
print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# Check requirements
print_section "Kiểm tra yêu cầu hệ thống"

# Check Node.js
if command -v node &> /dev/null; then
    NODE_VERSION=$(node --version)
    NODE_MAJOR=$(echo $NODE_VERSION | cut -d. -f1 | sed 's/v//')
    if [ $NODE_MAJOR -ge 14 ]; then
        print_success "Node.js version: $NODE_VERSION"
    else
        print_error "Node.js version quá cũ: $NODE_VERSION (cần >= v14)"
        print_warning "Vui lòng update Node.js: https://nodejs.org/"
        exit 1
    fi
else
    print_error "Node.js chưa được cài đặt"
    exit 1
fi

# Check npm
if command -v npm &> /dev/null; then
    NPM_VERSION=$(npm --version)
    print_success "npm version: $NPM_VERSION"
else
    print_error "npm chưa được cài đặt"
    exit 1
fi

# Check g++
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n 1)
    print_success "g++ compiler: $GCC_VERSION"
else
    print_error "g++ chưa được cài đặt"
    exit 1
fi

# Project structure
print_section "Cấu trúc dự án"
print_info "Server C++:      server/server_full.cpp"
print_info "Node.js Server:  node-server/server.js"
print_info "React Frontend:  frontend/src/"
print_info ""
print_info "Kiến trúc: React (Port 5173) → Node.js (Port 3000) → C++ (Port 8080)"

# Check if C++ server is compiled
print_section "Kiểm tra build"

if [ -f "$SCRIPT_DIR/server/server_full" ]; then
    print_success "C++ server đã được compile"
else
    print_warning "C++ server chưa được compile, đang compile..."
    cd "$SCRIPT_DIR/server"
    g++ -o server_full server_full.cpp -lpthread
    if [ $? -eq 0 ]; then
        print_success "Compile C++ server thành công"
    else
        print_error "Compile C++ server thất bại"
        exit 1
    fi
    cd "$SCRIPT_DIR"
fi

# Check dependencies
print_section "Kiểm tra dependencies"

if [ ! -d "$SCRIPT_DIR/node-server/node_modules" ]; then
    print_warning "Node-server dependencies chưa được cài, đang cài đặt..."
    cd "$SCRIPT_DIR/node-server"
    npm install --silent
    if [ $? -eq 0 ]; then
        print_success "Cài đặt node-server dependencies thành công"
    else
        print_error "Cài đặt node-server dependencies thất bại"
        exit 1
    fi
    cd "$SCRIPT_DIR"
else
    print_success "Node-server dependencies đã được cài đặt"
fi

if [ ! -d "$SCRIPT_DIR/frontend/node_modules" ]; then
    print_warning "Frontend dependencies chưa được cài, đang cài đặt..."
    cd "$SCRIPT_DIR/frontend"
    npm install --silent
    if [ $? -eq 0 ]; then
        print_success "Cài đặt frontend dependencies thành công"
    else
        print_error "Cài đặt frontend dependencies thất bại"
        exit 1
    fi
    cd "$SCRIPT_DIR"
else
    print_success "Frontend dependencies đã được cài đặt"
fi

# Show features
print_section "Tính năng đã implement"
print_success "Authentication (Register/Login)"
print_success "Player List & Status Management"
print_success "Challenge System"
print_success "Ship Placement with Validation"
print_success "Turn-based Gameplay"
print_success "Hit/Miss Detection"
print_success "Win/Lose Condition"
print_success "Real-time Chat"
print_success "Disconnect Handling"
print_success "Responsive UI Design"

# Technical details
print_section "Chi tiết kỹ thuật"
print_info "C++ Server:"
echo "  - Multi-threaded với pthread"
echo "  - POSIX sockets (TCP/IP)"
echo "  - JSON protocol"
echo "  - File-based user storage"
echo ""
print_info "Node.js Middleware:"
echo "  - Express.js + Socket.IO"
echo "  - WebSocket ↔ TCP bridge"
echo "  - Message buffering"
echo ""
print_info "React Frontend:"
echo "  - React 19 với Hooks"
echo "  - Socket.IO client"
echo "  - Responsive CSS3"
echo "  - Real-time updates"

# Instructions
print_section "Hướng dẫn chạy project"
echo ""
echo -e "${YELLOW}Cách 1: Tự động (khuyến nghị)${NC}"
echo "  ./start-all.sh"
echo ""
echo -e "${YELLOW}Cách 2: Thủ công (3 terminals riêng)${NC}"
echo "  Terminal 1: cd server && ./server_full"
echo "  Terminal 2: cd node-server && npm start"
echo "  Terminal 3: cd frontend && npm run dev"
echo ""
echo -e "${YELLOW}Sau khi chạy:${NC}"
echo "  1. Mở browser: ${GREEN}http://localhost:5173${NC}"
echo "  2. Đăng ký tài khoản (username/password)"
echo "  3. Đăng nhập"
echo "  4. Mở tab mới để tạo player thứ 2"
echo "  5. Thách đấu và chơi game!"
echo ""

# Quick test
print_section "Kiểm tra port"
print_info "Checking if ports are available..."
for port in 8080 3000 5173; do
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1 ; then
        print_warning "Port $port đang được sử dụng"
        echo "    Để kill: kill -9 \$(lsof -t -i:$port)"
    else
        print_success "Port $port available"
    fi
done

# Documentation
print_section "Tài liệu"
print_info "README_COMPLETE.md    - Hướng dẫn đầy đủ"
print_info "QUICKSTART.md         - Hướng dẫn nhanh"
print_info "PROJECT_SUMMARY.md    - Tổng kết dự án"
print_info "TESTING_CHECKLIST.md  - Checklist testing"

# Final message
print_section "Sẵn sàng!"
echo ""
echo -e "${GREEN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║  ✓ Project đã sẵn sàng để chạy!                              ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Chạy lệnh sau để start all servers:${NC}"
echo -e "  ${CYAN}./start-all.sh${NC}"
echo ""
echo -e "${YELLOW}Hoặc sử dụng Makefile:${NC}"
echo -e "  ${CYAN}make run${NC}"
echo ""
echo -e "${BLUE}Chúc bạn demo thành công! 🎮🚢${NC}"
echo ""
