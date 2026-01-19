# Photobooth Pro - Setup Guide

## Tổng quan

Photobooth Pro là hệ thống photobooth thương mại với:
- **Backend**: C++ với Canon EDSDK
- **Frontend**: React + TypeScript + Tailwind CSS
- **Communication**: WebSocket + REST API

---

## Yêu cầu hệ thống

### Windows
- Windows 10/11 (64-bit)
- Visual Studio 2019 hoặc mới hơn (với C++ Desktop Development)
- CMake 3.15+
- Node.js 18+ và npm
- Canon EDSDK 13.19.10 (đã có sẵn)

### Tùy chọn
- OpenCV 4.x (cho image processing nâng cao)
- Git

---

## Cài đặt Dependencies

### 1. Backend Dependencies

#### Tải các thư viện C++ cần thiết:

```bash
# Tạo thư mục lib trong backend
cd photobooth-pro/backend
mkdir lib
cd lib
```

#### cpp-httplib (HTTP Server)
```bash
git clone https://github.com/yhirose/cpp-httplib.git
```

#### websocketpp (WebSocket Server)
```bash
git clone https://github.com/zaphoyd/websocketpp.git
```

#### nlohmann/json (JSON parsing)
```bash
git clone https://github.com/nlohmann/json.git
```

#### spdlog (Logging)
```bash
git clone https://github.com/gabime/spdlog.git
```

#### SQLite3
- Download từ: https://www.sqlite.org/download.html
- Giải nén vào `backend/lib/sqlite3/`

### 2. Frontend Dependencies

```bash
cd photobooth-pro/frontend
npm install
```

---

## Build Backend (C++)

### Bước 1: Tạo build directory

```bash
cd photobooth-pro/backend
mkdir build
cd build
```

### Bước 2: Generate với CMake

```bash
# Với Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64

# Với Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64
```

### Bước 3: Build

```bash
# Debug build
cmake --build . --config Debug

# Release build
cmake --build . --config Release
```

### Bước 4: Chạy server

```bash
# Debug
.\Debug\photobooth-server.exe

# Release
.\Release\photobooth-server.exe
```

Server sẽ chạy tại:
- HTTP API: `http://localhost:8080`
- WebSocket: `ws://localhost:8081`

---

## Run Frontend (React)

### Development Mode

```bash
cd photobooth-pro/frontend
npm run dev
```

Frontend sẽ chạy tại: `http://localhost:3000`

### Production Build

```bash
npm run build
npm run preview
```

---

## Cấu trúc dự án

```
photobooth-pro/
├── backend/                 # C++ Backend
│   ├── src/                # Source files
│   │   ├── main.cpp
│   │   ├── core/
│   │   ├── camera/
│   │   ├── api/
│   │   ├── image/
│   │   ├── storage/
│   │   └── print/
│   ├── include/            # Header files
│   ├── lib/                # Third-party libraries
│   ├── build/              # Build output
│   └── CMakeLists.txt
│
├── frontend/               # React Frontend
│   ├── src/
│   │   ├── pages/         # Pages
│   │   ├── components/    # Components
│   │   ├── store/         # State management
│   │   ├── types/         # TypeScript types
│   │   └── services/      # API services
│   ├── public/
│   └── package.json
│
└── docs/                   # Documentation
```

---

## Kết nối Canon Camera

### Canon R100 qua USB

1. Kết nối camera qua USB
2. Bật camera
3. Chọn chế độ "PC Connection" trên camera
4. Backend sẽ tự động phát hiện camera

### Webcam Fallback

Nếu không có Canon camera, hệ thống sẽ tự động sử dụng webcam:
1. Kết nối webcam
2. Backend sẽ liệt kê trong danh sách cameras
3. Chọn webcam từ UI

---

## API Endpoints

### REST API (Port 8080)

```
GET  /api/cameras              # Danh sách cameras
POST /api/camera/select        # Chọn camera
GET  /api/camera/settings      # Lấy settings
POST /api/camera/settings      # Cập nhật settings
POST /api/capture              # Chụp ảnh
GET  /api/events               # Danh sách events
POST /api/events               # Tạo event mới
GET  /api/photos/:eventId      # Lấy photos của event
POST /api/print                # In ảnh
POST /api/share                # Chia sẻ ảnh
```

### WebSocket (Port 8081)

```
Events:
- liveview         # Live view frames
- countdown        # Countdown timer
- capture_complete # Ảnh đã chụp xong
- camera_status    # Trạng thái camera
```

---

## Troubleshooting

### Backend không build được

1. Kiểm tra Visual Studio đã cài C++ Desktop Development
2. Kiểm tra CMake version: `cmake --version`
3. Kiểm tra đường dẫn EDSDK trong CMakeLists.txt
4. Đảm bảo tất cả thư viện trong `backend/lib/` đã được clone

### Frontend không chạy

1. Xóa `node_modules` và `package-lock.json`
2. Chạy lại `npm install`
3. Kiểm tra Node.js version: `node --version` (cần >= 18)

### Camera không kết nối

1. Kiểm tra USB cable
2. Bật camera và chọn PC Connection mode
3. Cài Canon EOS Utility (nếu chưa có)
4. Kiểm tra Windows Device Manager

### WebSocket không kết nối

1. Kiểm tra backend đang chạy
2. Kiểm tra firewall không block port 8081
3. Xem console log trong browser

---

## Development Workflow

### 1. Start Backend
```bash
cd photobooth-pro/backend/build
.\Release\photobooth-server.exe
```

### 2. Start Frontend
```bash
cd photobooth-pro/frontend
npm run dev
```

### 3. Open Browser
```
http://localhost:3000
```

---

## Next Steps

1. ✅ Cấu trúc dự án đã tạo
2. ✅ Backend headers đã tạo
3. ✅ Frontend boilerplate đã tạo
4. ⏳ Implement backend source files
5. ⏳ Implement frontend components
6. ⏳ Tích hợp Canon SDK
7. ⏳ Implement image processing
8. ⏳ Implement printing
9. ⏳ Testing & optimization

---

## Support

Để được hỗ trợ, vui lòng:
1. Kiểm tra TODO.md để xem tiến độ
2. Xem logs trong console
3. Kiểm tra file README.md

---

## License

Commercial License - Photobooth Pro Team
