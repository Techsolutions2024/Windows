# PhotoBooth Pro - Hướng Dẫn Tiếng Việt

## Tổng Quan Dự Án

PhotoBooth Pro là phần mềm photobooth chuyên nghiệp dành cho sự kiện, tương tự như DSLR Booth, được xây dựng với:
- **Backend C++**: Điều khiển máy ảnh Canon qua EDSDK, xử lý ảnh
- **Frontend C# WPF**: Giao diện người dùng hiện đại, dễ sử dụng

## Tính Năng Chính

### 1. Điều Khiển Máy Ảnh
- ✅ Hỗ trợ máy ảnh Canon DSLR (EOS R, 5D, 6D, 7D, 90D, v.v.)
- ✅ Kết nối tự động qua USB
- ✅ Live view thời gian thực
- ✅ Chụp ảnh từ xa
- ✅ Điều chỉnh ISO, khẩu độ, tốc độ màn trập
- ✅ Hiển thị pin và số ảnh còn lại

### 2. Chụp Ảnh
- ✅ Chế độ 1-4 ảnh liên tiếp
- ✅ Đếm ngược tùy chỉnh (0-10 giây)
- ✅ Hiệu ứng đếm ngược trực quan
- ✅ Âm thanh đếm ngược
- ✅ Tự động lưu ảnh

### 3. Template & Khung Ảnh
- ✅ Thiết kế template tùy chỉnh
- ✅ Nhiều vùng ảnh trên 1 template
- ✅ Thêm text, logo, đồ họa
- ✅ Viền và khung ảnh
- ✅ Thư viện template có sẵn

### 4. Xử Lý Ảnh
- ✅ Overlay template tự động
- ✅ Green screen / Chroma key
- ✅ Thay đổi background
- ✅ Bộ lọc ảnh (Đen trắng, Sepia, Vintage, v.v.)
- ✅ Điều chỉnh độ sáng, độ tương phản
- ✅ Tạo collage
- ✅ Tạo GIF động

### 5. Green Screen
- ✅ Tự động phát hiện màu xanh lá
- ✅ Điều chỉnh độ nhạy
- ✅ Làm mịn viền
- ✅ Background tùy chỉnh
- ✅ Xem trước thời gian thực

### 6. In Ảnh
- ✅ In trực tiếp ra máy in ảnh
- ✅ Hỗ trợ nhiều loại máy in
- ✅ Chọn khổ giấy (4x6, 5x7, 8x10, v.v.)
- ✅ Cài đặt chất lượng in
- ✅ In nhiều bản
- ✅ Tự động in
- ✅ Quản lý hàng đợi in

### 7. Chia Sẻ
- ✅ Gửi email kèm ảnh
- ✅ Tạo mã QR để tải ảnh
- ✅ Rút gọn URL
- ✅ Chia sẻ lên mạng xã hội
- ✅ Upload lên cloud
- ✅ Thu thập email khách

### 8. Giao Diện

#### Màn Hình Điều Khiển (Operator)
- ✅ Giao diện tối hiện đại
- ✅ Live view panel
- ✅ Hiển thị trạng thái máy ảnh
- ✅ Cài đặt session
- ✅ Chọn template
- ✅ Điều khiển máy ảnh
- ✅ Thống kê

#### Màn Hình Khách (Guest Display)
- ✅ Giao diện toàn màn hình cảm ứng
- ✅ Màn hình chào mời
- ✅ Đếm ngược
- ✅ Live view với overlay
- ✅ Xem trước ảnh
- ✅ Nút chức năng (In, Chia sẻ, Chụp lại)
- ✅ Màn hình chia sẻ với QR code
- ✅ Nhập email
- ✅ Màn hình cảm ơn

### 9. Quản Lý Sự Kiện
- ✅ Hồ sơ sự kiện
- ✅ Cài đặt theo sự kiện
- ✅ Branding tùy chỉnh
- ✅ Logo overlay
- ✅ Watermark
- ✅ Nhiều sự kiện

### 10. Tính Năng Nâng Cao
- ✅ Tạo GIF động
- ✅ Video message
- ✅ Thu thập dữ liệu
- ✅ Thống kê chi tiết
- ✅ Hỗ trợ 2 màn hình
- ✅ Hỗ trợ màn hình cảm ứng
- ✅ Sao lưu tự động

## Cấu Trúc Dự Án

```
PhotoBooth/
├── README.md                    # Tài liệu chính
├── BUILD.md                     # Hướng dẫn build
├── FEATURES.md                  # Danh sách tính năng
├── USER_GUIDE.md                # Hướng dẫn sử dụng
├── HUONG_DAN_TIENG_VIET.md     # File này
│
├── Backend/                     # Backend C++
│   ├── CMakeLists.txt          # Cấu hình CMake
│   ├── EDSDKWrapper/           # Wrapper EDSDK
│   │   ├── Camera.h            # Header điều khiển camera
│   │   └── Camera.cpp          # Code điều khiển camera
│   ├── ImageProcessor/         # Xử lý ảnh
│   │   ├── Overlay.h           # Template overlay
│   │   ├── GreenScreen.h       # Chroma key
│   │   └── Filter.h            # Bộ lọc ảnh
│   ├── PrintManager/           # Quản lý in
│   └── Server/                 # IPC server
│
├── Frontend/                    # Frontend C# WPF
│   └── PhotoBoothApp/
│       ├── PhotoBoothApp.csproj
│       ├── App.xaml            # Định nghĩa ứng dụng
│       ├── Views/              # Giao diện
│       │   ├── MainWindow.xaml           # Màn hình điều khiển
│       │   └── GuestDisplayWindow.xaml   # Màn hình khách
│       ├── ViewModels/         # MVVM ViewModels
│       ├── Services/           # Business logic
│       ├── Models/             # Data models
│       └── Resources/          # Tài nguyên
│
└── EDSDK/                      # Canon EDSDK
    ├── Header/                 # EDSDK headers
    ├── Dll/                    # EDSDK DLLs
    └── Library/                # EDSDK libs
```

## Công Nghệ Sử Dụng

### Backend
- **Ngôn ngữ**: C++17
- **Build**: CMake
- **Camera SDK**: Canon EDSDK 13.19.10
- **Xử lý ảnh**: OpenCV 4.x

### Frontend
- **Ngôn ngữ**: C# 10
- **Framework**: .NET 6.0
- **UI**: WPF (Windows Presentation Foundation)
- **Kiến trúc**: MVVM

## Hướng Dẫn Build

### Windows

#### Backend C++
```bash
cd PhotoBooth/Backend
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Frontend C#
```bash
cd PhotoBooth/Frontend/PhotoBoothApp
dotnet restore
dotnet build --configuration Release
dotnet run
```

### Yêu Cầu Hệ Thống

#### Tối Thiểu
- Windows 10 64-bit
- Intel Core i5
- 8GB RAM
- 256GB SSD
- USB 3.0
- Màn hình 1920x1080

#### Khuyến Nghị
- Windows 11 64-bit
- Intel Core i7
- 16GB RAM
- 512GB SSD
- 2 màn hình (1920x1080)
- Màn hình cảm ứng

## Hướng Dẫn Sử Dụng Nhanh

### 1. Cài Đặt Ban Đầu

1. **Cài đặt phần mềm**
   - Chạy file cài đặt
   - Làm theo hướng dẫn

2. **Kết nối máy ảnh**
   - Kết nối Canon DSLR qua USB
   - Bật máy ảnh
   - Đặt máy ảnh ở chế độ "PC Remote"
   - Click "Connect Camera"

3. **Cấu hình màn hình**
   - Kết nối màn hình thứ 2 (tùy chọn)
   - Vào Settings > Display
   - Chọn màn hình cho operator và guest

### 2. Tạo Sự Kiện

1. Click "Settings" > "Events"
2. Click "New Event"
3. Nhập thông tin:
   - Tên sự kiện
   - Loại sự kiện
   - Ngày
   - Địa điểm
4. Cấu hình:
   - Số ảnh (1-4)
   - Thời gian đếm ngược
   - Template
   - Bật/tắt in ảnh
   - Bật/tắt chia sẻ
5. Lưu sự kiện

### 3. Bắt Đầu Session

1. Chọn sự kiện
2. Click "Start Live View"
3. Click "Guest Display" (mở màn hình khách)
4. Click "Start Session"
5. Photobooth đã sẵn sàng!

### 4. Quy Trình Khách Hàng

1. **Màn hình chào**: Khách chạm vào màn hình
2. **Đếm ngược**: 3-2-1
3. **Chụp ảnh**: Camera chụp
4. **Xử lý**: Áp dụng template
5. **Xem trước**: Hiển thị ảnh
6. **Hành động**: In / Chia sẻ / Chụp lại
7. **Cảm ơn**: Quay về màn hình chào

## Tạo Template Tùy Chỉnh

1. Vào Settings > Templates
2. Click "New Template"
3. Chọn kích thước (4x6, 5x7, v.v.)
4. Thêm background
5. Thêm vùng ảnh (photo zones)
6. Thêm text (tên sự kiện, ngày, v.v.)
7. Thêm logo/đồ họa
8. Xem trước
9. Lưu template

## Cài Đặt Máy In

1. Cài driver máy in
2. Kết nối máy in (USB hoặc mạng)
3. Vào Settings > Printer
4. Click "Detect Printers"
5. Chọn máy in
6. Chọn khổ giấy
7. Chọn chất lượng in
8. Click "Test Print"

## Máy In Được Hỗ Trợ

- Canon SELPHY (CP1300, CP1500, v.v.)
- DNP DS series
- Mitsubishi CP series
- HiTi P series
- Máy in ảnh tương thích Windows

## Xử Lý Sự Cố

### Máy Ảnh Không Kết Nối
- Kiểm tra máy ảnh đã bật
- Kiểm tra cáp USB
- Thử cổng USB khác
- Đóng phần mềm Canon khác (EOS Utility)
- Khởi động lại PhotoBooth Pro

### Live View Không Hoạt Động
- Đặt máy ảnh ở chế độ PC Remote
- Kiểm tra mode dial (dùng P, Av, Tv, hoặc M)
- Tắt tự động tắt nguồn máy ảnh

### Máy In Không Hoạt Động
- Kiểm tra nguồn và kết nối
- Cài/cập nhật driver
- Khởi động lại máy in
- Kiểm tra giấy và mực

## Mẹo & Thủ Thuật

### Trước Sự Kiện
1. Test tất cả tính năng
2. Chuẩn bị backup (pin, cáp, giấy in)
3. Đến sớm 1-2 giờ
4. Test ánh sáng

### Trong Sự Kiện
1. Theo dõi pin máy ảnh
2. Kiểm tra giấy/mực in
3. Hỗ trợ khách hàng
4. Backup ảnh định kỳ

### Sau Sự Kiện
1. Export tất cả ảnh
2. Tạo báo cáo
3. Backup toàn bộ
4. Bảo trì thiết bị

### Green Screen
1. Ánh sáng đều trên màn xanh
2. Khoảng cách 3-6 feet
3. Tránh mặc đồ xanh lá
4. Điều chỉnh tolerance

### Thiết Kế Template
1. Giữ đơn giản
2. Màu sắc nhất quán
3. Text dễ đọc
4. Test in trước sự kiện

## Tính Năng Nổi Bật

### So Sánh Với DSLR Booth

| Tính Năng | PhotoBooth Pro | DSLR Booth |
|-----------|----------------|------------|
| Điều khiển Canon DSLR | ✅ | ✅ |
| Live View | ✅ | ✅ |
| Template tùy chỉnh | ✅ | ✅ |
| Green Screen | ✅ | ✅ |
| In ảnh | ✅ | ✅ |
| Chia sẻ QR Code | ✅ | ✅ |
| Giao diện cảm ứng | ✅ | ✅ |
| Tạo GIF | ✅ | ✅ |
| Video Message | ✅ | ✅ |
| Mã nguồn mở | ✅ | ❌ |
| Miễn phí | ✅ | ❌ |

## Phát Triển Thêm

### Tính Năng Sắp Tới (v1.1)
- [ ] AI xóa background
- [ ] Nhận diện khuôn mặt
- [ ] Tích hợp Instagram
- [ ] Thêm template

### Tính Năng Tương Lai (v2.0)
- [ ] Xử lý trên cloud
- [ ] App mobile
- [ ] AI nâng cao ảnh
- [ ] Background ảo

## Hỗ Trợ

### Tài Liệu
- README.md - Bắt đầu nhanh
- BUILD.md - Hướng dẫn build
- USER_GUIDE.md - Hướng dẫn chi tiết
- FEATURES.md - Danh sách tính năng

### Liên Hệ
- Email: support@photoboothpro.com
- Website: https://photoboothpro.com
- Forum: https://forum.photoboothpro.com

## Giấy Phép

PhotoBooth Pro sử dụng Canon EDSDK theo thỏa thuận giấy phép của Canon.

Mã nguồn ứng dụng là độc quyền. Liên hệ để biết thông tin giấy phép.

## Tác Giả

- Backend C++: Tích hợp EDSDK, xử lý ảnh
- Frontend C# WPF: Giao diện người dùng
- Design: UI/UX
- Testing: QA

## Cảm Ơn

- Canon Inc. cho EDSDK
- Cộng đồng mã nguồn mở
- Beta testers
- Người dùng đầu tiên

---

**PhotoBooth Pro** - Phần Mềm Photobooth Chuyên Nghiệp

Phiên bản 1.0.0 | © 2025 | Bảo lưu mọi quyền

## Câu Hỏi Thường Gặp (FAQ)

### 1. PhotoBooth Pro có miễn phí không?
Mã nguồn được cung cấp cho mục đích học tập và phát triển. Liên hệ để biết thông tin giấy phép thương mại.

### 2. Hỗ trợ máy ảnh nào?
Tất cả máy ảnh Canon DSLR tương thích với EDSDK 13.19.10 (EOS R series, 5D, 6D, 7D, 90D, v.v.)

### 3. Có chạy trên Mac/Linux không?
Hiện tại chủ yếu cho Windows. Mac/Linux đang trong giai đoạn thử nghiệm.

### 4. Cần internet không?
Không cần internet để chạy photobooth. Chỉ cần khi chia sẻ lên cloud hoặc mạng xã hội.

### 5. Có hỗ trợ tiếng Việt không?
Hiện tại giao diện bằng tiếng Anh. Có thể thêm tiếng Việt trong phiên bản sau.

### 6. Giá bao nhiêu?
Liên hệ để biết thông tin giá và giấy phép.

### 7. Có hỗ trợ kỹ thuật không?
Có, qua email, forum, và tài liệu chi tiết.

### 8. Có thể tùy chỉnh không?
Có, hoàn toàn có thể tùy chỉnh template, branding, và nhiều tính năng khác.

### 9. Cần máy tính cấu hình gì?
Tối thiểu: i5, 8GB RAM, Windows 10. Khuyến nghị: i7, 16GB RAM, Windows 11.

### 10. Có thể dùng cho kinh doanh không?
Có, cần giấy phép thương mại. Liên hệ để biết thêm chi tiết.
