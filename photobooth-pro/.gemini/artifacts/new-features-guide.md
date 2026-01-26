# Hướng Dẫn Sử Dụng & Test Tính Năng Mới (GIF/Boomerang/Video)

## 1. Cài Đặt Dependencies

Để tính năng tạo GIF và Video hoạt động, bạn cần cài đặt các công cụ sau trên máy Windows:

### ImageMagick (Cho GIF/Boomerang)
- Tải về: https://imagemagick.org/script/download.php#windows
- Cài đặt và tích chọn "Add application directory to your system path".
- Kiểm tra bằng lệnh: `magick -version`

### FFmpeg (Cho Video - nếu dùng)
- Tải về: https://ffmpeg.org/download.html
- Thêm thư mục `bin` vào PATH.
- Kiểm tra bằng lệnh: `ffmpeg -version`

## 2. API Endpoints

### Chụp GIF
**URL:** `POST /api/capture/gif`
**Body:**
```json
{
  "eventId": 1,
  "frameCount": 10,
  "frameDelay": 10,
  "width": 800,
  "height": 600
}
```
**Mô tả:** Chụp 10 ảnh liên tiếp (burst), mỗi ảnh cách nhau 100ms (cho delay 10), sau đó ghép thành file GIF.

### Chụp Boomerang
**URL:** `POST /api/capture/boomerang`
**Body:**
```json
{
  "eventId": 1,
  "frameCount": 10
}
```
**Mô tả:** Chụp 10 ảnh, sau đó tạo GIF lặp lại theo thứ tự xuôi -> ngược -> xuôi (hiệu ứng boomerang).

## 3. Cách Test Trên Frontend

1. Khởi động Backend:
   ```bash
   cd backend/build
   ./bin/photobooth-server
   ```

2. Khởi động Frontend:
   ```bash
   cd frontend
   npm run dev
   ```

3. Vào trang chủ -> Chọn Event -> Chọn Mode (GIF hoặc Boomerang).
4. Bấm chụp và chờ đợi kết quả.

## 4. Troubleshooting

**Lỗi: "Camera busy or unavailable"**
- Kiểm tra xem LiveView có đang chạy không. Burst mode yêu cầu quyền kiểm soát camera.
- Đảm bảo camera Canon đã kết nối qua USB.

**Lỗi: GIF không tạo được (Response success=false)**
- Kiểm tra log của backend server.
- Đảm bảo ImageMagick đã cài đặt và lệnh `magick` có thể chạy từ command line.
- Kiểm tra thư mục `data/captures` có quyền ghi không.

**Hiệu năng chậm?**
- Giảm `resolution` (width/height) trong request body.
- Giảm `frameCount`.
- Tăng `frameInterval` (delay).

---
**Note:** Tính năng Video hiện tại đang ở chế độ Simulation (Shim) trả về thành công giả lập. Để quay video thật cần tích hợp FFmpeg sâu hơn hoặc dùng chức năng Movie của Canon SDK (phức tạp hơn).
