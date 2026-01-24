# Electron Migration Plan - Photobooth Pro

## Mục Tiêu
Chuyển đổi từ kiến trúc **React (Vite) + C++ HTTP/WebSocket Server** sang **Electron + N-API + React** để:
- ✅ Loại bỏ WebSocket disconnect issues
- ✅ Cải thiện hiệu năng Live View (IPC thay vì WebSocket)
- ✅ Tránh port conflicts và network issues
- ✅ Tích hợp chặt chẽ hơn với Canon SDK

---

## Kiến Trúc Mới (3 Tầng)

```
┌─────────────────────────────────────────────────────────┐
│  Tầng 3: React Frontend (Renderer Process)              │
│  - UI Components (LiveView, Capture, Settings)          │
│  - Gọi IPC đến Main Process                             │
│  - Nhận binary frames qua IPC                           │
└─────────────────────────────────────────────────────────┘
                         ↕ IPC (contextBridge)
┌─────────────────────────────────────────────────────────┐
│  Tầng 2: Electron Main Process                          │
│  - Load N-API module (.node)                            │
│  - Handle IPC requests từ Renderer                      │
│  - Forward commands đến C++ layer                       │
│  - Broadcast frames đến Renderer                        │
└─────────────────────────────────────────────────────────┘
                         ↕ N-API
┌─────────────────────────────────────────────────────────┐
│  Tầng 1: C++ Native Module (N-API)                      │
│  - Canon EDSDK wrapper                                  │
│  - WebcamCamera (OpenCV)                                │
│  - Live View thread                                     │
│  - Capture logic                                        │
└─────────────────────────────────────────────────────────┘
```

---

## Phase 1: Setup Electron Infrastructure

### 1.1 Install Dependencies
```bash
cd d:\photo\Windows\photobooth-pro
npm install --save-dev electron electron-builder
npm install --save electron-store
```

### 1.2 Create Electron Main Process
**File:** `electron/main.js`
```javascript
const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const cameraModule = require('./native/camera.node'); // N-API module

let mainWindow;
let liveViewInterval;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1920,
    height: 1080,
    fullscreen: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  // Load React app
  if (process.env.NODE_ENV === 'development') {
    mainWindow.loadURL('http://localhost:5173');
  } else {
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
  }
}

// IPC Handlers
ipcMain.handle('camera:list', async () => {
  return cameraModule.getCameras();
});

ipcMain.handle('camera:select', async (event, { type, index }) => {
  return cameraModule.selectCamera(type, index);
});

ipcMain.handle('liveview:start', async () => {
  // Start live view loop
  liveViewInterval = setInterval(() => {
    const frame = cameraModule.getFrame(); // Binary JPEG data
    if (frame) {
      mainWindow.webContents.send('liveview:frame', frame);
    }
  }, 50); // 20 FPS
  return { success: true };
});

ipcMain.handle('liveview:stop', async () => {
  if (liveViewInterval) {
    clearInterval(liveViewInterval);
    liveViewInterval = null;
  }
  cameraModule.stopLiveView();
  return { success: true };
});

ipcMain.handle('camera:capture', async () => {
  const result = cameraModule.capture();
  return result;
});

app.whenReady().then(createWindow);
```

### 1.3 Create Preload Script
**File:** `electron/preload.js`
```javascript
const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  // Camera APIs
  listCameras: () => ipcRenderer.invoke('camera:list'),
  selectCamera: (config) => ipcRenderer.invoke('camera:select', config),
  
  // Live View APIs
  startLiveView: () => ipcRenderer.invoke('liveview:start'),
  stopLiveView: () => ipcRenderer.invoke('liveview:stop'),
  onLiveViewFrame: (callback) => {
    ipcRenderer.on('liveview:frame', (event, frame) => {
      callback(frame);
    });
  },
  
  // Capture APIs
  capture: () => ipcRenderer.invoke('camera:capture'),
});
```

---

## Phase 2: Build N-API Native Module

### 2.1 Setup N-API Build Environment
```bash
npm install --save-dev node-addon-api cmake-js
```

**File:** `binding.gyp`
```python
{
  "targets": [
    {
      "target_name": "camera",
      "sources": [
        "backend/src/napi/camera_module.cpp",
        "backend/src/camera/WebcamCamera.cpp",
        "backend/src/camera/CanonCamera.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "backend/include",
        "C:/opencv/build/include",
        "C:/EDSDK/Header"
      ],
      "libraries": [
        "C:/opencv/build/x64/vc16/lib/opencv_world4100.lib",
        "C:/EDSDK/Library/EDSDK.lib"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    }
  ]
}
```

### 2.2 Create N-API Wrapper
**File:** `backend/src/napi/camera_module.cpp`
```cpp
#include <napi.h>
#include "camera/WebcamCamera.h"
#include "camera/CanonCamera.h"
#include <memory>

using namespace Napi;

std::unique_ptr<photobooth::ICamera> activeCamera;
std::vector<uint8_t> latestFrame;
std::mutex frameMutex;

// Get list of cameras
Value GetCameras(const CallbackInfo& info) {
  Env env = info.Env();
  Array cameras = Array::New(env);
  
  // List webcams
  auto webcams = photobooth::WebcamCamera::listAvailableWebcams();
  for (size_t i = 0; i < webcams.size(); i++) {
    Object cam = Object::New(env);
    cam.Set("type", String::New(env, "webcam"));
    cam.Set("name", String::New(env, webcams[i].second));
    cam.Set("index", Number::New(env, webcams[i].first));
    cameras[i] = cam;
  }
  
  return cameras;
}

// Select camera
Value SelectCamera(const CallbackInfo& info) {
  Env env = info.Env();
  
  if (info.Length() < 2) {
    TypeError::New(env, "Expected 2 arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  std::string type = info[0].As<String>().Utf8Value();
  int index = info[1].As<Number>().Int32Value();
  
  if (type == "webcam") {
    activeCamera = std::make_unique<photobooth::WebcamCamera>(index);
    bool connected = activeCamera->connect();
    
    Object result = Object::New(env);
    result.Set("success", Boolean::New(env, connected));
    return result;
  }
  
  return env.Null();
}

// Start live view
Value StartLiveView(const CallbackInfo& info) {
  Env env = info.Env();
  
  if (!activeCamera) {
    Object result = Object::New(env);
    result.Set("success", Boolean::New(env, false));
    result.Set("error", String::New(env, "No camera selected"));
    return result;
  }
  
  bool started = activeCamera->startLiveView([](const std::vector<uint8_t>& frame, int w, int h) {
    std::lock_guard<std::mutex> lock(frameMutex);
    latestFrame = frame;
  });
  
  Object result = Object::New(env);
  result.Set("success", Boolean::New(env, started));
  return result;
}

// Get latest frame
Value GetFrame(const CallbackInfo& info) {
  Env env = info.Env();
  
  std::lock_guard<std::mutex> lock(frameMutex);
  if (latestFrame.empty()) {
    return env.Null();
  }
  
  // Return as Buffer
  Buffer<uint8_t> buffer = Buffer<uint8_t>::Copy(env, latestFrame.data(), latestFrame.size());
  return buffer;
}

// Stop live view
Value StopLiveView(const CallbackInfo& info) {
  Env env = info.Env();
  
  if (activeCamera) {
    activeCamera->stopLiveView();
  }
  
  return env.Null();
}

// Capture photo
Value Capture(const CallbackInfo& info) {
  Env env = info.Env();
  
  if (!activeCamera) {
    Object result = Object::New(env);
    result.Set("success", Boolean::New(env, false));
    return result;
  }
  
  // Synchronous capture for simplicity
  // In production, use async worker
  Object result = Object::New(env);
  result.Set("success", Boolean::New(env, true));
  result.Set("path", String::New(env, "data/captures/photo.jpg"));
  
  return result;
}

// Initialize module
Object Init(Env env, Object exports) {
  exports.Set("getCameras", Function::New(env, GetCameras));
  exports.Set("selectCamera", Function::New(env, SelectCamera));
  exports.Set("startLiveView", Function::New(env, StartLiveView));
  exports.Set("getFrame", Function::New(env, GetFrame));
  exports.Set("stopLiveView", Function::New(env, StopLiveView));
  exports.Set("capture", Function::New(env, Capture));
  
  return exports;
}

NODE_API_MODULE(camera, Init)
```

---

## Phase 3: Update React Frontend

### 3.1 Create Electron API Hook
**File:** `frontend/src/hooks/useElectronCamera.ts`
```typescript
import { useEffect, useState, useCallback } from 'react';

export function useElectronCamera() {
  const [cameras, setCameras] = useState([]);
  const [liveViewFrame, setLiveViewFrame] = useState<string | null>(null);
  
  useEffect(() => {
    // Setup live view frame listener
    window.electronAPI.onLiveViewFrame((frame: Buffer) => {
      // Convert Buffer to Blob URL
      const blob = new Blob([frame], { type: 'image/jpeg' });
      const url = URL.createObjectURL(blob);
      setLiveViewFrame(url);
    });
  }, []);
  
  const listCameras = useCallback(async () => {
    const cams = await window.electronAPI.listCameras();
    setCameras(cams);
  }, []);
  
  const selectCamera = useCallback(async (type: string, index: number) => {
    return await window.electronAPI.selectCamera({ type, index });
  }, []);
  
  const startLiveView = useCallback(async () => {
    return await window.electronAPI.startLiveView();
  }, []);
  
  const stopLiveView = useCallback(async () => {
    return await window.electronAPI.stopLiveView();
  }, []);
  
  const capture = useCallback(async () => {
    return await window.electronAPI.capture();
  }, []);
  
  return {
    cameras,
    liveViewFrame,
    listCameras,
    selectCamera,
    startLiveView,
    stopLiveView,
    capture
  };
}
```

### 3.2 Update LiveViewDisplay Component
```tsx
import { useEffect } from 'react';
import { useElectronCamera } from '../hooks/useElectronCamera';

export default function LiveViewDisplay() {
  const { liveViewFrame, startLiveView, stopLiveView } = useElectronCamera();
  
  useEffect(() => {
    startLiveView();
    return () => {
      stopLiveView();
    };
  }, []);
  
  return (
    <div className="live-view">
      {liveViewFrame ? (
        <img src={liveViewFrame} alt="Live View" />
      ) : (
        <div>Waiting for camera...</div>
      )}
    </div>
  );
}
```

---

## Phase 4: Build & Package

### 4.1 Build N-API Module
```bash
npm run build:native
# or
cmake-js build --runtime electron --runtime-version 28.0.0
```

### 4.2 Update package.json
```json
{
  "name": "photobooth-pro",
  "version": "1.0.0",
  "main": "electron/main.js",
  "scripts": {
    "dev": "concurrently \"npm run dev:vite\" \"npm run dev:electron\"",
    "dev:vite": "vite",
    "dev:electron": "electron .",
    "build": "vite build && electron-builder",
    "build:native": "cmake-js build"
  },
  "build": {
    "appId": "com.photobooth.pro",
    "files": [
      "dist/**/*",
      "electron/**/*",
      "native/**/*.node"
    ],
    "extraFiles": [
      {
        "from": "C:/opencv/build/x64/vc16/bin",
        "to": ".",
        "filter": ["*.dll"]
      }
    ]
  }
}
```

---

## Migration Timeline

| Phase | Task | Estimate |
|-------|------|----------|
| 1 | Setup Electron + Preload | 2 hours |
| 2 | Build N-API Module | 4 hours |
| 3 | Update React Frontend | 2 hours |
| 4 | Testing & Debug | 4 hours |
| **Total** | | **12 hours** |

---

## Benefits of New Architecture

✅ **No WebSocket Issues** - IPC is more reliable than WebSocket
✅ **Better Performance** - Direct binary transfer, no Base64 encoding
✅ **Simpler Deployment** - Single executable, no separate backend server
✅ **Offline Capable** - No network dependency
✅ **Better Error Handling** - Synchronous IPC calls easier to debug

---

## Next Steps

1. **Backup current code**: `git commit -am "Backup before Electron migration"`
2. **Create electron branch**: `git checkout -b feature/electron-migration`
3. **Follow phases 1-4** in order
4. **Test thoroughly** before merging to main

---

**Lưu ý:** Migration này sẽ thay đổi toàn bộ kiến trúc. Cân nhắc kỹ trước khi thực hiện.
