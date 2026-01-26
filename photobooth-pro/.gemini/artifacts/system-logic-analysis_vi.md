# Logic Hoạt Động Hệ Thống Photobooth Pro

## 1. Mô Hình Tổng Quan (The "Event" Concept)

Trong hệ thống này, **Event** (Sự kiện) đóng vai trò trung tâm. Nó hoạt động như một "hồ sơ cấu hình" (Configuration Profile) độc lập.

**Ví dụ thực tế:**
- **Sáng:** Bạn chạy sự kiện "Đám Cưới Minh & Lan". Cấu hình: Chụp 3 ảnh, in template hoa hồng, có GIF.
- **Chiều:** Bạn chạy sự kiện "Sinh Nhật Run Club". Cấu hình: Chụp 1 ảnh thể thao, in template logo CLB, có Boomerang.

-> Bạn chỉ cần chuyển đổi Event trên phần mềm, toàn bộ hành vi hệ thống (từ giao diện, cách chụp, template in, folder lưu ảnh) sẽ thay đổi theo ngay lập tức.

---

## 2. Luồng Dữ Liệu Chi Tiết (Data Flow)

### A. Giai Đoạn Khởi Tạo (Setup Phase)

1. **User tạo Event:**
   - API: `POST /api/events`
   - **Backend:**
     - Tạo record trong bảng `events` (SQLite).
     - Tạo record mặc định trong bảng `event_configs`.
     - Tạo folder vật lý: `D:/PhotoboothPro/Data/Events/{Event_ID}/`.

2. **User Cấu Hình (Config):**
   - API: `PUT /api/events/{id}/config`
   - User chọn: "Bật GIF", "Template 2.png", "Countdown 5s".
   - **Backend:** Cập nhật bảng `event_configs`.

### B. Giai Đoạn Chụp Ảnh/GIF (Runtime Logic)

Khi khách hàng nhấn nút "Chụp GIF":

1. **Frontend:**
   - Gọi `POST /api/capture/gif` với body `{ "eventId": 123 }`.
   - Hiển thị màn hình đếm ngược (Countdown).

2. **Backend (Controller):**
   - Nhận request, kiểm tra trạng thái Camera.
   - Nếu Camera đang bận (đang chụp cho khách khác) -> Trả lỗi 409 Busy.
   - Nếu Rảnh -> Khóa Camera (Lock Mutex).

3. **Backend (Xử Lý Chụp - BurstCaptureManager):**
   - Tạm dừng luồng Live View (để ưu tiên băng thông USB cho chụp full-res).
   - Gửi lệnh xuống Canon SDK: "Switch to High-Speed Continuous Drive Mode".
   - Kích hoạt Shutter -> Chụp liên tiếp 10 tấm (Burst).
   - Canon SDK đẩy 10 file JPEG thô về RAM.
   - Backend lưu tạm vào `Temp/Burst_{Timestamp}/`.

4. **Backend (Xử Lý Ảnh - Post Processing):**
   - **ImageMagick/OpenCV:** Đọc 10 file ảnh.
   - Resize về kích thước optimal (ví dụ 800x600 cho GIF nhẹ).
   - Ghép thành file `.gif` động.
   - Lưu kết quả cuối cùng vào `Data/Events/{Event_ID}/GIFs/`.
   - Lưu thông tin file vào database bảng `photos`.

5. **Hoàn Tất:**
   - Backend trả về URL của file GIF (ví dụ: `http://localhost:8080/images/events/123/gif_001.gif`).
   - Mở lại Live View.
   - **Frontend:** Nhận URL -> Hiển thị GIF lên màn hình -> Chuyển sang màn hình Share/In.

---

## 3. Kiến Trúc Xử Lý Đa Luồng (Multi-threading Strategy)

Để hệ thống chạy mượt mà 24/7, backend sử dụng kiến trúc đa luồng thông minh:

| Luồng (Thread) | Nhiệm Vụ | Đặc Điểm |
| :--- | :--- | :--- |
| **Main Thread** | HTTP Server, API Handling | Nhận request, trả response nhanh. Không được block. |
| **LiveView Thread** | Lấy stream từ Canon -> WebSocket | Chạy liên tục 30 lần/giây. Ưu tiên thấp hơn chụp. |
| **Capture Thread** | Điều khiển chụp ảnh | Chỉ chạy khi có lệnh chụp. Độ ưu tiên cao nhất (Real-time). |
| **Processing Thread** | Resize ảnh, Tạo GIF, Render Filter | Chạy ngầm (Background). Tốn CPU nhưng không làm đơ giao diện. |

**Tại sao cần chia luồng?**
Nếu không chia luồng, khi đang tạo file GIF (mất khoảng 2-3 giây), giao diện Live View sẽ bị đứng hình, khách hàng sẽ tưởng máy bị treo. Với kiến trúc này, khách vẫn thấy giao diện mượt mà trong khi máy đang âm thầm xử lý ảnh.

---

## 4. Quản Lý Trạng Thái (State Management)

Hệ thống luôn duy trì một máy trạng thái (State Machine) để đảm bảo không bị lỗi logic:

- **IDLE:** Máy rảnh, đang hiện Live View.
- **COUNTDOWN:** Đang đếm ngược, khóa các nút bấm khác.
- **CAPTURING:** Đang chụp (Shutter mở), tắt Live View tạm thời.
- **PROCESSING:** Đang xử lý ảnh (tạo GIF/Filter), Camera đã rảnh để Live View lại, nhưng chưa cho chụp tiếp.
- **PRINTING:** Đang gửi lệnh in.

*Logic an toàn:* Nếu khách bấm nút chụp liên tục 10 lần, hệ thống chỉ nhận lệnh đầu tiên (chuyển từ IDLE -> CAPTURING) và từ chối 9 lệnh sau cho đến khi quay lại IDLE.

---

## 5. Tích Hợp Phần Cứng (Hardware Integration)

Logic kết nối thiết bị khi khởi động Event:

1. **Quét thiết bị:** Tìm tất cả Camera Canon và Webcam.
2. **So khớp Config:** Event này yêu cầu "Canon EOS 700D"? Hệ thống tìm đúng ID của máy đó để kết nối.
3. **Fail-safe:** Nếu không tìm thấy Canon, tự động fallback (dự phòng) sang Webcam để sự kiện không bị gián đoạn.
