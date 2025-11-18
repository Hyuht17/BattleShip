# Script để chạy Frontend
Write-Host "Starting Frontend on port 5173..." -ForegroundColor Green
Set-Location -Path "$PSScriptRoot\frontend"
npm run dev
