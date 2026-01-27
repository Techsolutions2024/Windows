# Backend Implementation TODO

## Module 1: Events API + Database ✅ COMPLETED

### Database
- [x] Update DatabaseManager.h với EventConfig struct
- [x] Implement DatabaseManager.cpp với SQLite
- [x] Create database schema (events, event_configs, photos tables)
- [x] Add CRUD operations for events
- [x] Add event config management
- [x] Add photo management

### Events API
- [x] GET /api/events - Lấy danh sách events
- [x] POST /api/events - Tạo event mới
- [x] GET /api/events/:id - Lấy chi tiết event
- [x] PUT /api/events/:id - Cập nhật event
- [x] DELETE /api/events/:id - Xóa event
- [x] POST /api/events/:id/duplicate - Nhân bản event
- [x] GET /api/events/:id/config - Lấy cấu hình event
- [x] PUT /api/events/:id/config - Cập nhật cấu hình event
- [x] GET /api/events/search - Tìm kiếm events

### Files Created/Updated
- [x] backend/include/storage/DatabaseManager.h
- [x] backend/src/storage/DatabaseManager.cpp
- [x] backend/include/api/HTTPServer.h
- [x] backend/src/api/HTTPServer.cpp
- [x] backend/include/core/Application.h
- [x] backend/CMakeLists.txt (added SQLite)
- [x] backend/lib/sqlite/README.md

---

## Module 2: Camera API (PENDING)

### Endpoints
- [ ] GET /api/cameras - Danh sách cameras
- [ ] POST /api/cameras/select - Chọn camera
- [ ] GET /api/cameras/settings - Lấy settings
- [ ] PUT /api/cameras/settings - Cập nhật settings
- [ ] POST /api/cameras/liveview/start - Bắt đầu live view
- [ ] POST /api/cameras/liveview/stop - Dừng live view

### Files to Update
- [ ] backend/src/camera/CameraManager.cpp
- [ ] backend/src/camera/CanonCamera.cpp
- [ ] backend/src/camera/WebcamCamera.cpp

---

## Module 3: Capture API (PENDING)

### Endpoints
- [ ] POST /api/capture/photo - Chụp ảnh
- [ ] POST /api/capture/gif - Chụp GIF
- [ ] POST /api/capture/boomerang - Chụp Boomerang
- [ ] POST /api/capture/video/start - Bắt đầu quay video
- [ ] POST /api/capture/video/stop - Dừng quay video

---

## Module 4: Gallery API (PENDING)

### Endpoints
- [x] GET /api/gallery/:eventId - Lấy photos của event
- [x] GET /api/gallery/photo/:id - Lấy chi tiết photo
- [x] DELETE /api/gallery/photo/:id - Xóa photo
- [ ] POST /api/gallery/collage - Tạo collage

---

## Module 5: Print API (PENDING)

### Endpoints
- [ ] GET /api/printers - Danh sách printers
- [ ] POST /api/print - In ảnh
- [ ] GET /api/print/status - Trạng thái in
- [ ] GET /api/print/settings - Cài đặt in
- [ ] PUT /api/print/settings - Cập nhật cài đặt in

### Files to Update
- [ ] backend/src/print/PrintManager.cpp

---

## Module 6: Share API (PENDING)

### Endpoints
- [ ] POST /api/share/email - Gửi email
- [ ] POST /api/share/sms - Gửi SMS
- [ ] POST /api/share/qrcode - Tạo QR code

### Files to Create
- [ ] backend/src/share/EmailService.cpp
- [ ] backend/src/share/SMSService.cpp
- [ ] backend/src/share/QRCodeGenerator.cpp

---

## Module 7: WebSocket Events (PENDING)

### Events
- [ ] liveview:frame - Frame live view
- [ ] capture:countdown - Đếm ngược
- [ ] capture:complete - Chụp xong
- [ ] print:status - Trạng thái in
- [ ] camera:status - Trạng thái camera

### Files to Update
- [ ] backend/src/api/WebSocketServer.cpp

---

## Module 8: Settings API (PENDING)

### Endpoints
- [x] GET /api/settings - Lấy tất cả settings (stub)
- [x] PUT /api/settings - Cập nhật settings (stub)
- [x] GET /api/settings/storage - Thông tin storage (stub)
- [x] GET /api/settings/network - Trạng thái network (stub)

### Files to Create
- [ ] backend/include/settings/SettingsManager.h
- [ ] backend/src/settings/SettingsManager.cpp

---

## Dependencies Required

### SQLite
- Download from: https://www.sqlite.org/download.html
- Place sqlite3.c and sqlite3.h in backend/lib/sqlite/

### cpp-httplib
- Download from: https://github.com/yhirose/cpp-httplib
- Place httplib.h in backend/lib/cpp-httplib/

### nlohmann/json
- Download from: https://github.com/nlohmann/json
- Place json.hpp in backend/lib/json/include/nlohmann/

---

## Build Instructions

```bash
cd photobooth-pro/backend
mkdir build && cd build
cmake ..
cmake --build .
```

## Run Server

```bash
./bin/photobooth-server
```

Server will start on:
- HTTP: http://localhost:8080
- WebSocket: ws://localhost:8081
