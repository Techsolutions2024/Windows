# PhotoBooth (prototype)

Prototype photobooth cho sự kiện (tương tự DSLRBooth) với backend C#/.NET 8.

Mục tiêu của prototype:

- Chạy được không cần camera thật (`MockCamera`).
- Có API để tạo session / chụp / render / “print”.
- Có kiosk UI đơn giản chạy ngay trong ASP.NET (`wwwroot/index.html`).
- Có adapter `gphoto2` (Linux) để chụp camera thật nếu có.

## Yêu cầu

- .NET SDK 8
- (Tùy chọn) `gphoto2` nếu dùng camera thật

## Chạy nhanh

```bash
cd photobooth
dotnet run --project src/PhotoBooth.Api
```

Mở: `http://localhost:5000/`

## API chính

- `POST /api/sessions` tạo session
- `POST /api/sessions/{id}/capture` chụp ảnh (mock/gphoto2)
- `POST /api/sessions/{id}/render` ghép ảnh ra layout 2x2 (JPEG 1200x1800)
- `POST /api/sessions/{id}/print` print (prototype: xuất file vào thư mục `prints/` của app)
- `GET /api/sessions/{id}/files/{fileName}` lấy ảnh đã chụp/render

## Chọn driver camera

Mặc định dùng `MockCamera`. Để dùng `gphoto2`:

```bash
cd photobooth
Camera__Driver=gphoto2 dotnet run --project src/PhotoBooth.Api
```

## Nơi lưu file

- Ảnh chụp + ảnh render lưu tạm theo session trong: `TMPDIR/photobooth/sessions/{sessionId}`
- File “print” xuất ra: `src/PhotoBooth.Api/bin/.../prints/`

## Gợi ý mở rộng lên sản phẩm (roadmap)

- Adapter Canon EDSDK (đã có sample trong repo `sample/CSharp`) để chụp tethered ổn định.
- Hàng đợi job (capture/render/print) + retry + telemetry.
- Template engine: PSD/PNG overlay, nhiều layout (strip 2x6, 4x6…), text dynamic, QR.
- Live view + countdown + chọn filter realtime.
- In thật: CUPS (Linux) / Windows Spooler / SDK máy in (DNP/HiTi).
- Offline-first kiosk + auto-start + full-screen + lock-down.

