# Script để chạy Node.js Server
Write-Host "Starting Node.js Server on port 3000..." -ForegroundColor Green
Set-Location -Path "$PSScriptRoot\node-server"
npm start
