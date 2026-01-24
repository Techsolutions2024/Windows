# Phân Tích Tích Hợp Canon EDSDK (Từ MultiCamCui)

## 1. Nguyên Tắc Cốt Lõi (Critical)
*   **Threading**: Canon EDSDK yêu cầu chạy trên **Single Thread** (STA).
    *   `EdsInitializeSDK`, `EdsGetEvent`, và mọi lệnh điều khiển Camera (Take Picture, Live View) **PHẢI** được gọi từ cùng một thread (thường là Main Thread).
    *   Nếu dùng HTTP Server đa luồng (multi-threaded), ta phải dùng cơ chế **Command Queue** để đẩy yêu cầu từ các thread xử lý HTTP về Main Thread xử lý, sau đó trả kết quả lại.
*   **Keep-Alive**: Phải gọi `EdsGetEvent` liên tục (ví dụ mỗi 200ms) trong vòng lặp chính để:
    *   Giữ kết nối USB ổn định.
    *   Nhận sự kiện từ camera (nút chụp vật lý được ấn, ngắt kết nối, ảnh đã lưu xong).
    *   Ngăn camera tự tắt (Auto Power Off).

## 2. Kiến Trúc Class `CameraModel`
Mẫu `MultiCamCui` sử dụng class `CameraModel` rất tốt để đóng gói logic. Chúng ta sẽ tái sử dụng cấu trúc này cho Backend chính thức.

### Các hàm quan trọng cần port sang Backend:
*   **Quản lý Session**:
    *   `OpenSessionCommand()`: Kết nối, cài đặt property ban đầu (Lưu vào PC, Time, v.v.).
    *   `CloseSessionCommand()`: Ngắt kết nối an toàn.
*   **Chụp Ảnh**:
    *   `TakePicture()`: Gửi lệnh `kEdsCameraCommand_PressShutterButton`.
    *   Lưu ý: Cần xử lý kịch bản `kEdsCameraCommand_ShutterButton_Completely_NonAF` để chụp nhanh hơn nếu đã lấy nét.
*   **Live View (EVF)**:
    *   `StartEvfCommand()`: Bật màn trập, cho phép stream.
    *   `DownloadEvfCommand()`: Lấy dữ liệu ảnh raw từ buffer camera.
        *   *Cải tiến cho Prod*: Convert buffer này sang **Base64** hoặc **JPEG Blob** ngay lập tức để gửi qua WebSocket.
*   **Settings (Get/Set)**:
    *   Dùng `EdsGetPropertyData` / `EdsSetPropertyData` cho ISO, Av, Tv.
    *   Cần map các ID Property (`kEdsPropID_...`) sang JSON để Frontend dễ hiểu.

## 3. Quy Trình Xử Lý Ảnh (Workflow)
1.  **Start**: Init SDK -> Get Camera List -> Open Session.
2.  **Loop**:
    *   `EdsGetEvent()`
    *   Check Command Queue (từ WebSocket/HTTP).
    *   Nếu có lệnh chụp -> `TakePicture` -> Chờ sự kiện `kEdsObjectEvent_DirItemCreated`.
    *   Khi có ảnh mới -> Tải ảnh về (`EdsDownload`) -> Bắn signal cho Frontend.
3.  **Live View Loop**:
    *   Nếu đang ở màn hình chụp -> Gọi `DownloadEvfCommand` liên tục -> Gửi frame qua WebSocket :8081.

## 4. Các Lệnh Quan Trọng (Command Mapping)
Dựa theo `command.md`:
*   `Start Live View`: 21
*   `Take Picture`: 3
*   `Set Save To`: 1 (Quan trọng: set là `kEdsSaveTo_Host` để ảnh bay thẳng về PC, không lưu thẻ nhớ làm chậm).

## 5. Lưu Ý
*   Không được block Main Thread quá lâu.
*   Xử lý lỗi `EDS_ERR_DEVICE_BUSY`: Camera đang bận (ví dụ đang ghi thẻ), cần retry nhẹ nhàng.
