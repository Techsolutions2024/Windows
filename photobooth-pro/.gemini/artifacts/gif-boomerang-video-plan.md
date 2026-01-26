# Kế Hoạch Triển Khai GIF, Boomerang & Video Capture

## Tổng Quan

Triển khai 3 chế độ chụp nâng cao cho Photobooth Pro:
1. **GIF Capture** - Chụp liên tiếp và tạo ảnh GIF động
2. **Boomerang** - Chụp và phát ngược lại (loop)
3. **Video Capture** - Quay video ngắn

## Kiến Trúc Tổng Thể

```
Frontend (React)
    ↓ Chọn mode (GIF/Boomerang/Video)
    ↓ Gửi yêu cầu capture
Backend (C++)
    ↓ Điều khiển Canon camera
    ↓ Chụp burst/quay video
    ↓ Xử lý frames với OpenCV
    ↓ Tạo GIF với ImageMagick hoặc gifsicle
    ↓ Trả về file cho Frontend
Frontend
    ↓ Hiển thị preview
    ↓ Cho phép share/print
```

---

## 1. GIF Capture

### Backend Implementation

#### A. Cấu Hình Canon Camera (CanonSDKCamera.cpp)

**Burst Mode Settings:**
```cpp
// Set drive mode to High-Speed Continuous
EdsUInt32 driveMode = 0x00000004; // High-Speed Continuous
EdsSetPropertyData(cameraRef_, kEdsPropID_DriveMode, 0, sizeof(driveMode), &driveMode);
```

**Capture Strategy:**
- Chụp 10-15 ảnh liên tiếp với tốc độ cao
- Mỗi ảnh cách nhau ~100-200ms
- Lưu tạm vào buffer

#### B. GIF Creation Module

**File:** `backend/src/media/GifCreator.h` & `GifCreator.cpp`

**Phương pháp:**
1. **Option 1: ImageMagick (Recommended)**
   - Pros: Chất lượng cao, nhiều tùy chọn
   - Cons: Cần cài đặt ImageMagick
   - Command: `convert -delay 10 -loop 0 frame*.jpg output.gif`

2. **Option 2: gifsicle**
   - Pros: Nhẹ, nhanh
   - Cons: Cần convert PNG trước
   
3. **Option 3: giflib (C library)**
   - Pros: Tích hợp trực tiếp vào code
   - Cons: Phức tạp hơn

**Recommended: ImageMagick via system call**

```cpp
class GifCreator {
public:
    struct GifOptions {
        int frameDelay = 10;      // 100ms per frame
        int loopCount = 0;        // 0 = infinite
        int width = 800;
        int height = 600;
        int quality = 80;
    };
    
    std::string createGif(
        const std::vector<std::string>& imagePaths,
        const std::string& outputPath,
        const GifOptions& options
    );
    
private:
    bool resizeImages(const std::vector<std::string>& paths, int width, int height);
    std::string buildImageMagickCommand(const GifOptions& options);
};
```

#### C. API Endpoint

**File:** `backend/src/api/HTTPServer.cpp`

```cpp
POST /api/capture/gif
{
    "eventId": 123,
    "frameCount": 10,
    "frameDelay": 10,
    "quality": 80
}

Response:
{
    "success": true,
    "gifPath": "/data/captures/event_123/gif_20260126_123456.gif",
    "frameCount": 10,
    "fileSize": 2048576
}
```

### Frontend Implementation

#### A. GIF Capture Flow

**File:** `frontend/src/pages/CapturePage.tsx`

```typescript
const captureGif = async () => {
    setStatus('Preparing...');
    
    // 1. Countdown
    await runCountdown(3);
    
    // 2. Show "Smile and hold still!"
    setStatus('Capturing frames...');
    
    // 3. Call API
    const response = await api.captureGif({
        eventId,
        frameCount: 10,
        frameDelay: 10
    });
    
    // 4. Show progress (backend sẽ gửi qua WebSocket)
    // Frame 1/10... 2/10... etc.
    
    // 5. Show preview
    setGifUrl(response.gifPath);
    setStatus('Done!');
};
```

#### B. Progress Tracking via WebSocket

```typescript
// Listen for capture progress
wsClient.on('capture:progress', (data) => {
    setProgress(data.current / data.total * 100);
    setStatusMessage(`Capturing ${data.current}/${data.total}`);
});

wsClient.on('capture:processing', () => {
    setStatusMessage('Creating GIF...');
});

wsClient.on('capture:complete', (data) => {
    setGifUrl(data.gifPath);
    navigate('/preview');
});
```

---

## 2. Boomerang Capture

### Backend Implementation

**Concept:** Chụp N frames → Tạo sequence: [1,2,3,4,5,4,3,2,1]

```cpp
class BoomerangCreator {
public:
    struct BoomerangOptions {
        int frameCount = 5;       // Số frame gốc
        int frameDelay = 8;       // Delay giữa các frame (80ms)
        int loopCount = 0;        // Infinite loop
        bool smoothReverse = true; // Bỏ frame đầu/cuối khi reverse
    };
    
    std::string createBoomerang(
        const std::vector<std::string>& imagePaths,
        const std::string& outputPath,
        const BoomerangOptions& options
    );
    
private:
    std::vector<std::string> createBoomerangSequence(
        const std::vector<std::string>& paths,
        bool smoothReverse
    );
};
```

**Implementation:**
```cpp
std::vector<std::string> BoomerangCreator::createBoomerangSequence(
    const std::vector<std::string>& paths,
    bool smoothReverse
) {
    std::vector<std::string> sequence;
    
    // Forward: [1,2,3,4,5]
    for (const auto& path : paths) {
        sequence.push_back(path);
    }
    
    // Reverse: [4,3,2] (skip first and last to avoid stutter)
    if (smoothReverse && paths.size() > 2) {
        for (int i = paths.size() - 2; i > 0; i--) {
            sequence.push_back(paths[i]);
        }
    } else {
        for (int i = paths.size() - 1; i >= 0; i--) {
            sequence.push_back(paths[i]);
        }
    }
    
    return sequence;
}
```

### Frontend Implementation

```typescript
const captureBoomerang = async () => {
    setStatus('Get ready for Boomerang!');
    await runCountdown(3);
    
    setStatus('Recording... Keep moving!');
    
    const response = await api.captureBoomerang({
        eventId,
        frameCount: 5,
        frameDelay: 8
    });
    
    setBoomerangUrl(response.gifPath);
    navigate('/preview');
};
```

---

## 3. Video Capture

### Backend Implementation

#### A. Video Recording với Canon SDK

**Challenges:**
- Canon EDSDK không hỗ trợ trực tiếp video recording qua API
- Phải dùng Movie Mode và trigger qua remote control

**Alternative Approach: Frame-to-Video**
- Chụp burst với tốc độ cao (10-30 FPS)
- Dùng FFmpeg để tạo video từ frames

```cpp
class VideoCreator {
public:
    struct VideoOptions {
        int fps = 30;
        int duration = 5;         // seconds
        int width = 1920;
        int height = 1080;
        std::string codec = "libx264";
        int quality = 23;         // CRF value (lower = better)
    };
    
    std::string createVideo(
        const std::vector<std::string>& imagePaths,
        const std::string& outputPath,
        const VideoOptions& options
    );
    
private:
    std::string buildFFmpegCommand(const VideoOptions& options);
};
```

**FFmpeg Command:**
```bash
ffmpeg -framerate 30 -i frame_%04d.jpg -c:v libx264 -crf 23 -pix_fmt yuv420p output.mp4
```

#### B. Real-time Video Recording (Alternative)

**Using OpenCV VideoWriter:**
```cpp
cv::VideoWriter writer;
writer.open("output.mp4", 
            cv::VideoWriter::fourcc('m','p','4','v'),
            30.0,  // FPS
            cv::Size(1920, 1080));

// In live view loop
while (recording && duration < maxDuration) {
    cv::Mat frame = getLiveViewFrame();
    writer.write(frame);
}

writer.release();
```

### Frontend Implementation

```typescript
const captureVideo = async () => {
    setStatus('Preparing camera...');
    
    // Start recording
    await api.startVideoRecording({
        eventId,
        duration: 5,
        fps: 30
    });
    
    // Show countdown timer
    setStatus('Recording...');
    setRecordingTime(0);
    
    const timer = setInterval(() => {
        setRecordingTime(prev => prev + 1);
    }, 1000);
    
    // Wait for duration
    await new Promise(r => setTimeout(r, 5000));
    
    // Stop recording
    clearInterval(timer);
    const response = await api.stopVideoRecording();
    
    setVideoUrl(response.videoPath);
    navigate('/preview');
};
```

---

## Dependencies Cần Thêm

### Backend

1. **ImageMagick** (cho GIF)
   ```bash
   # Windows
   choco install imagemagick
   
   # Hoặc download từ: https://imagemagick.org/script/download.php
   ```

2. **FFmpeg** (cho Video)
   ```bash
   # Windows
   choco install ffmpeg
   
   # Hoặc download từ: https://ffmpeg.org/download.html
   ```

3. **gifsicle** (optional, alternative cho GIF)
   ```bash
   choco install gifsicle
   ```

### CMakeLists.txt Updates

```cmake
# Find ImageMagick (optional)
find_package(ImageMagick COMPONENTS convert)

# Find FFmpeg (optional)
find_package(FFmpeg COMPONENTS avcodec avformat avutil swscale)

# If not found, use system calls
if(NOT ImageMagick_FOUND)
    message(STATUS "ImageMagick not found - will use system calls")
endif()
```

---

## File Structure

```
backend/
├── include/
│   └── media/
│       ├── GifCreator.h
│       ├── BoomerangCreator.h
│       └── VideoCreator.h
├── src/
│   └── media/
│       ├── GifCreator.cpp
│       ├── BoomerangCreator.cpp
│       └── VideoCreator.cpp

frontend/
└── src/
    ├── components/
    │   ├── GifPreview.tsx
    │   ├── BoomerangPreview.tsx
    │   └── VideoPreview.tsx
    └── hooks/
        └── useMediaCapture.ts
```

---

## API Endpoints Summary

```
POST /api/capture/gif
POST /api/capture/boomerang
POST /api/capture/video/start
POST /api/capture/video/stop
GET  /api/capture/status/:captureId
```

---

## WebSocket Events

```
capture:countdown     - Đếm ngược
capture:progress      - Tiến trình chụp (frame x/y)
capture:processing    - Đang xử lý (tạo GIF/video)
capture:complete      - Hoàn thành
capture:error         - Lỗi
```

---

## Implementation Priority

### Phase 1: GIF Capture (Ưu tiên cao)
1. ✅ Tạo GifCreator class
2. ✅ Implement burst capture trong CanonSDKCamera
3. ✅ Tạo API endpoint
4. ✅ Frontend integration
5. ✅ Testing

### Phase 2: Boomerang (Ưu tiên trung bình)
1. ✅ Tạo BoomerangCreator (reuse GifCreator)
2. ✅ API endpoint
3. ✅ Frontend integration

### Phase 3: Video Capture (Ưu tiên thấp)
1. ✅ Research Canon SDK video capabilities
2. ✅ Implement frame-to-video approach
3. ✅ FFmpeg integration
4. ✅ Frontend integration

---

## Performance Considerations

### GIF/Boomerang
- **Frame count:** 5-15 frames (balance quality vs file size)
- **Resolution:** 800x600 hoặc 1024x768 (đủ cho social media)
- **File size target:** < 5MB
- **Processing time:** < 3 seconds

### Video
- **Duration:** 3-10 seconds
- **FPS:** 24-30
- **Resolution:** 1080p
- **Codec:** H.264 (tương thích rộng)
- **File size target:** < 20MB

---

## Testing Checklist

- [ ] GIF: Chụp 10 frames, tạo GIF loop
- [ ] GIF: Kiểm tra file size < 5MB
- [ ] Boomerang: Smooth reverse effect
- [ ] Boomerang: No stutter at loop point
- [ ] Video: 5s recording at 30 FPS
- [ ] Video: Playback smooth trên mobile
- [ ] All: WebSocket progress updates
- [ ] All: Error handling (camera disconnect, disk full)

---

**Next Steps:**
1. Implement GifCreator class
2. Add burst capture to CanonSDKCamera
3. Create API endpoints
4. Frontend integration
5. Test with real Canon camera
