# High-Performance Live View Implementation

## Overview
Successfully implemented a production-grade Live View system for the Photobooth Pro application using a three-tier architecture with high-performance binary streaming.

## Architecture

### Backend (C++ with uWebSockets)
**Port:** 8081

#### Components Created:
1. **LiveViewServer** (`backend/src/server/LiveViewServer.cpp`)
   - Singleton pattern for global access
   - uWebSockets for ultra-low latency binary streaming
   - Flow control mechanism with "READY" signal
   - Runs on dedicated thread to avoid blocking main loop

2. **CanonSDKCamera Integration** (`backend/src/camera/CanonSDKCamera.cpp`)
   - Modified `liveViewLoop()` to implement optimized pipeline:
     1. Copy raw buffer from Canon EDSDK
     2. **Immediately release** `EdsImageRef` and `EdsStreamRef` (critical for FPS)
     3. Decode JPEG with OpenCV `cv::imdecode()`
     4. Resize to 720p (1280x720) with `cv::resize()`
     5. Re-encode to JPEG with quality 70 using `cv::imencode()`
     6. Send to LiveViewServer via `std::move()` (zero-copy)

3. **Dependencies Added** (`backend/CMakeLists.txt`)
   - **libuv** v1.48.0 - Event loop for uSockets on Windows
   - **uSockets** v0.8.8 - Low-level socket abstraction
   - **uWebSockets** v20.48.0 - High-performance WebSocket library
   - **OpenCV** 4.9.0 - Image processing (already present)

### Frontend (React + Web Worker)

#### Components Created:
1. **Web Worker** (`frontend/src/liveview.worker.ts`)
   - Dedicated thread for WebSocket communication
   - Binary mode: `binaryType = 'arraybuffer'`
   - Off-thread JPEG decoding using `createImageBitmap()`
   - **Transferable Objects** for zero-copy transfer to main thread
   - Auto-reconnect after 2 seconds on disconnect
   - Flow control: sends "READY" signal after each frame processed

2. **LiveViewCanvas Component** (`frontend/src/components/LiveViewCanvas.tsx`)
   - Uses `<canvas>` instead of `<img>` for hardware-accelerated rendering
   - Receives `ImageBitmap` from worker (GPU-decoded)
   - Draws with `ctx.drawImage(bitmap, ...)`
   - **Critical:** Calls `bitmap.close()` immediately after drawing to release GPU memory
   - Real-time FPS counter
   - Desynchronized canvas context for reduced latency

3. **CapturePage Integration** (`frontend/src/pages/CapturePage.tsx`)
   - Replaced old `LiveViewDisplay` with `LiveViewCanvas`
   - Clean integration with existing UI

## Performance Optimizations

### Backend
✅ **Immediate Resource Release:** EDSDK resources freed before processing  
✅ **720p Downscaling:** Reduces bandwidth by ~70% vs full resolution  
✅ **JPEG Quality 70:** Balance between quality and file size  
✅ **std::move Semantics:** Zero-copy buffer transfers  
✅ **Flow Control:** Only send when frontend is ready (prevents lag buildup)  
✅ **Mutex-Protected Buffer:** Thread-safe frame storage  
✅ **Reduced Wait Time:** 20ms vs 50ms on `EDS_ERR_OBJECT_NOTREADY`

### Frontend
✅ **Web Worker:** Offloads network I/O from main thread  
✅ **ImageBitmap:** Hardware-accelerated JPEG decoding  
✅ **Transferable Objects:** Zero-copy transfer between threads  
✅ **Canvas Rendering:** GPU-accelerated vs DOM-based `<img>`  
✅ **Desynchronized Context:** Reduces vsync blocking  
✅ **Immediate Bitmap Cleanup:** Prevents GPU memory leaks  
✅ **Flow Control:** Backpressure prevents frame queue buildup

## Data Flow

```
Canon Camera (Live View)
    ↓ (EDSDK)
CanonSDKCamera::liveViewLoop()
    ↓ (Copy raw buffer)
Release EDSDK resources ⚡
    ↓
OpenCV Decode → Resize 720p → Encode JPEG Q70
    ↓ (std::move)
LiveViewServer (uWebSockets)
    ↓ (Binary WebSocket on port 8081)
liveview.worker.ts (Web Worker)
    ↓ (createImageBitmap - GPU decode)
LiveViewCanvas.tsx (React Component)
    ↓ (ctx.drawImage)
User sees live preview at ~30 FPS
    ↓
Send "READY" signal ← Flow Control Loop
```

## Port Configuration

| Service | Port | Protocol | Purpose |
|---------|------|----------|---------|
| HTTP API | 8080 | HTTP | REST endpoints |
| WebSocket | 8082 | WS | General events (changed from 8081) |
| **LiveView** | **8081** | **WS Binary** | **High-performance video stream** |

## Files Modified/Created

### Backend
- ✅ `backend/include/server/LiveViewServer.h` (NEW)
- ✅ `backend/src/server/LiveViewServer.cpp` (NEW)
- ✅ `backend/src/camera/CanonSDKCamera.cpp` (UPDATED)
- ✅ `backend/src/core/Application.cpp` (UPDATED)
- ✅ `backend/CMakeLists.txt` (UPDATED)

### Frontend
- ✅ `frontend/src/liveview.worker.ts` (NEW)
- ✅ `frontend/src/components/LiveViewCanvas.tsx` (NEW)
- ✅ `frontend/src/pages/CapturePage.tsx` (UPDATED)

### Documentation
- ✅ `backend/TODO.md` (UPDATED)

## Build Instructions

### Backend
```bash
cd backend
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Frontend
```bash
cd frontend
npm install
npm run dev
```

## Testing Checklist

- [ ] Verify uWebSockets server starts on port 8081
- [ ] Confirm Canon camera connects and starts live view
- [ ] Check FPS counter shows ~30 FPS
- [ ] Verify no memory leaks (GPU and CPU)
- [ ] Test auto-reconnect on disconnect
- [ ] Validate 720p resolution in browser DevTools
- [ ] Confirm flow control prevents lag buildup
- [ ] Test with multiple start/stop cycles

## Known Limitations

1. **Single Client Support:** Current implementation assumes one active viewer. Multiple clients will share the "ready" state.
2. **No Mirror/Filter Support:** Removed from LiveViewCanvas (can be added back if needed)
3. **Fixed Resolution:** Hardcoded to 720p (can be made configurable)
4. **Windows Only:** uSockets/libuv configuration is Windows-specific

## Future Enhancements

- [ ] Add mirror/flip support in LiveViewCanvas
- [ ] Make resolution configurable
- [ ] Support multiple simultaneous clients with per-socket flow control
- [ ] Add WebRTC for even lower latency
- [ ] Implement adaptive quality based on network conditions
- [ ] Add recording capability directly from live stream

## Performance Metrics (Expected)

- **Latency:** ~100-150ms end-to-end
- **FPS:** 25-30 FPS sustained
- **Bandwidth:** ~500KB/s - 1MB/s (720p JPEG Q70)
- **CPU Usage:** ~15-25% (encoding + decoding)
- **Memory:** Stable (no leaks with proper cleanup)

---

**Status:** ✅ Implementation Complete  
**Date:** 2026-01-26  
**Next Steps:** Build and test the complete system
