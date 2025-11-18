#!/bin/bash

# Test script cho WSL client

echo "╔═══════════════════════════════════════════════════╗"
echo "║  BattleShip WSL Client Test                       ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""

# Check if running in WSL
if grep -qi microsoft /proc/version; then
    echo "✅ Running in WSL"
    WSL_IP=$(hostname -I | awk '{print $1}')
    echo "   WSL IP: $WSL_IP"
else
    echo "⚠️  Not running in WSL"
fi

echo ""

# Get C++ server IP from .env or ask
if [ -f "node-server/.env" ]; then
    CPP_IP=$(grep CPP_SERVER_HOST node-server/.env | cut -d'=' -f2)
    echo "📡 C++ Server from .env: $CPP_IP"
else
    read -p "Enter C++ Server IP: " CPP_IP
fi

echo ""
echo "Testing connection to $CPP_IP:8080..."
echo ""

# Test 1: Ping
echo -n "1. Ping... "
if ping -c 2 -W 2 $CPP_IP >/dev/null 2>&1; then
    echo "✅ OK"
else
    echo "❌ Failed"
    echo "   Check if server is on same network"
fi

# Test 2: Port 8080
echo -n "2. Port 8080... "
if timeout 2 bash -c "cat < /dev/null > /dev/tcp/$CPP_IP/8080" 2>/dev/null; then
    echo "✅ OK"
    echo ""
    echo "🎉 Connection successful! You can run the game!"
    echo ""
    echo "Next steps:"
    echo "  1. cd node-server && npm start"
    echo "  2. cd frontend && npm run dev"
    echo "  3. Open browser: http://localhost:5173"
else
    echo "❌ Failed"
    echo ""
    echo "⚠️  Troubleshooting:"
    echo ""
    echo "   On Windows (PowerShell as Admin):"
    echo "   - Check Windows Firewall:"
    echo "     Get-NetFirewallProfile | Select-Object Name, Enabled"
    echo ""
    echo "   - Temporarily disable (test only):"
    echo "     Set-NetFirewallProfile -Profile Domain,Public,Private -Enabled False"
    echo ""
    echo "   On Linux server ($CPP_IP):"
    echo "   - Check if C++ server running:"
    echo "     ps aux | grep server_full"
    echo ""
    echo "   - Check firewall:"
    echo "     sudo ufw status"
    echo ""
    echo "   - Open port:"
    echo "     sudo ufw allow 8080/tcp"
fi

echo ""
