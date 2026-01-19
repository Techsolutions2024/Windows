# Photobooth Pro

A professional commercial photobooth system competing with DSLRBooth.

## Features

- 📷 **Multi-Camera Support**: Canon DSLR (via EDSDK) + Webcam fallback
- 🎬 **Capture Modes**: Photo, GIF, Boomerang, Video
- 🎨 **Real-time Effects**: Filters, overlays, beauty filters
- 🖨️ **Print Management**: Queue system, multiple layouts
- 📱 **Sharing**: Email, QR codes, social media
- 🎯 **Event Management**: Multiple sessions, templates
- 📊 **Analytics**: Usage statistics, reports

## Tech Stack

### Backend (C++)
- Canon EDSDK 13.19.10
- cpp-httplib (HTTP server)
- websocketpp (WebSocket)
- OpenCV 4.x (Image processing)
- SQLite3 (Database)
- spdlog (Logging)

### Frontend (React)
- React 18 + TypeScript
- Vite
- Tailwind CSS
- Zustand (State)
- Socket.io (WebSocket)
- Framer Motion (Animations)

## Project Structure

```
photobooth-pro/
├── backend/           # C++ Backend
│   ├── src/          # Source files
│   ├── include/      # Header files
│   ├── lib/          # Libraries (EDSDK)
│   ├── tests/        # Unit tests
│   └── build/        # Build output
├── frontend/         # React Frontend
│   ├── src/
│   ├── public/
│   └── dist/
├── shared/           # Shared types/protocols
├── docs/             # Documentation
└── scripts/          # Build scripts
```

## Quick Start

### Prerequisites
- Windows 10/11
- Visual Studio 2019+ (C++ build tools)
- Node.js 18+
- CMake 3.20+
- Canon camera (R100 recommended) or Webcam

### Backend Setup
```bash
cd backend
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Frontend Setup
```bash
cd frontend
npm install
npm run dev
```

## Development

### Backend Development
- Language: C++17
- Build: CMake
- IDE: Visual Studio / VS Code

### Frontend Development
- Language: TypeScript
- Framework: React 18
- Build: Vite
- Dev Server: http://localhost:5173

## Camera Support

### Canon DSLR (via EDSDK)
- Canon R100 ✓
- Canon R50 ✓
- Canon R6/R5 ✓
- Other EOS R series ✓

### Webcam
- DirectShow compatible webcams
- USB cameras
- Built-in laptop cameras

## License

Commercial - All Rights Reserved

## Author

Photobooth Pro Team
