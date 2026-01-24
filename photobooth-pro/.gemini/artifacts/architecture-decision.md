# Tình Hình Hiện Tại và Quyết Định Kiến Trúc

## ❌ Vấn Đề Phát Sinh

Đang có **conflict** giữa 2 hướng tiếp cận:

### Hướng 1: Hybrid Electron (đang làm dở)
- Electron spawns backend C++ server
- Backend có HTTP + SSE endpoints
- Frontend React gọi qua IPC

**Vấn đề:**
- Build errors: duplicate implementations
- `HTTPServerLiveView.cpp` có lỗi với httplib API
- Chưa tích hợp đúng với Canon SDK official sample

### Hướng 2: Canon SDK Official Way (MultiCamCui)
- Command-based interface qua terminal
- CMakeLists.txt riêng biệt
- Đúng chuẩn Canon SDK
- Hiện đang là standalone app

## 📋 Phân Tích

User chỉ ra rằng:
> "SDK Canon chúng ta sẽ làm việc qua command như trong folder backend\MultiCamCui"
> "Có 2 CMakeLists.txt khác nhau cần tích hợp đúng"
> "backend\MultiCamCui là chuẩn chính xác khi làm việc với SDK cần dựa vào đó"

Điều này có nghĩa:
1. Không nên sửa đổi trực tiếp Canon SDK code
2. Cần làm việc với MultiCamCui như một **external tool** 
3. Backend server nên **gọi MultiCamCui.exe** thay vì tích hợp SDK trực tiếp

## 🎯 Giải Pháp Đúng

### Kiến Trúc Mới (Process-based):

```
Electron Main Process
    ↓ spawns
Backend C++ Server (photobooth-server.exe)
    ↓ spawns subprocess
MultiCamCui.exe (Canon SDK official tool)
    ↓ commands via stdin/stdout
Canon Camera Hardware
```

### Luồng Hoạt Động:

**Live View:**
1. Frontend → IPC → Electron Main
2. Electron Main → HTTP → Backend Server
3. Backend Server → spawn → `MultiCamCui.exe` với command `21` (Start Live View)
4. Backend Server → spawn → `MultiCamCui.exe` với command `20` (Get Live View Image) - loop
5. Backend Server → SSE → Electron Main → IPC → Frontend

**Capture:**
1. Frontend → IPC → Electron Main
2. Electron Main → HTTP → Backend Server  
3. Backend Server → spawn → `MultiCamCui.exe` với command `3` (Take Picture)
4. MultiCamCui saves to `cam{number}/` folder
5. Backend reads file → returns path → Frontend

## 🔧 Cần Làm Gì Tiếp

### Option A: Rollback và Làm Lại (Đúng Chuẩn Canon)
1. **Rollback** tất cả changes của CameraManager
2. **Không** thêm `isLiveViewActive()` hay `getLatestFrame()`
3. **Tạo ProcessManager** class:
   ```cpp
   class ProcessManager {
     std::string executeCommand(std::vector<std::string> args);
     void startLiveView();
     std::vector<uint8_t> getLiveViewFrame();
     void takePicture();
   };
   ```
4. ProcessManager sẽ spawn `MultiCamCui.exe` và communicate qua stdin/stdout

### Option B: Quick Fix Backend Hiện Tại (Tạm Thời)
1. Remove duplicate `startLiveView`/`stopLiveView` từ CameraManager.cpp
2. Fix `HTTPServerLiveView.cpp` bugs
3. **SAU ĐÓ** migrate sang ProcessManager approach

### Option C: Sử Dụng WebSocket Tạm (Nhanh Nhất)
1. Bỏ SSE approach
2. Dùng lại WebSocket backend hiện tại
3. Electron chỉ spawn backend + connect WebSocket
4. **SAU ĐÓ** refactor để gọi MultiCamCui đúng cách

## 💡 Khuyến Nghị

**Ưu tiên: Option C → sau đó → Option A**

**Lý do:**
1. Option C cho phép test Electron ngay (5 phút)
2. Verify toàn bộ Electron infrastructure hoạt động
3. Sau khi OK, mới refactor backend để gọi MultiCamCui đúng cách (Option A)

**Roadmap:**
1. **Ngay bây giờ:** Fix build errors + dùng WebSocket tạm
2. **Test:** Verify Electron + Backend hoạt động
3. **Sau đó:** Implement ProcessManager để gọi MultiCamCui
4. **Cuối cùng:** Migration hoàn toàn sang command-based approach

## 📝 Quyết Định Cần User

Bạn muốn:
1. **Quick fix để test ngay** (Option C) - 10 phút
2. **Làm lại đúng chuẩn Canon SDK ngay** (Option A) - 1-2 giờ
3. **Giải pháp khác?**

---

**Tình trạng hiện tại:**
- ✅ Electron infrastructure: Done
- ✅ Frontend hooks: Done
- ❌ Backend: Build errors, cần quyết định kiến trúc
- ❓ Canon SDK integration: Chưa rõ approach

**Câu hỏi chính:** Backend nên gọi MultiCamCui.exe như external process hay tích hợp SDK trực tiếp?
