import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import dotenv from 'dotenv';
import tailwindcss from '@tailwindcss/vite';
dotenv.config();
// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), tailwindcss()],
  server: {
    host: 'localhost', // Chỉ local, không public LAN
    port: process.env.PORT_VITE || 5173,
    strictPort: true,
  }
})
