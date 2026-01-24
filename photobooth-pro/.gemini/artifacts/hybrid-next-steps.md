# Hybrid Electron Implementation - Next Steps

## ✅ Completed (15 minutes)

1. **Updated `electron/main.js`**
   - Spawns backend server as child process
   - HTTP communication instead of N-API
   - SSE connection for live view frames
   - Proper cleanup on app quit

2. **Updated `electron/preload.js`**
   - Same API surface as before
   - Uses IPC to communicate with main process
   - Main process talks to backend via HTTP

## ⏳ Remaining Tasks (45 minutes)

### Task 1: Add SSE Endpoint to Backend (20 min)

**File:** `backend/src/api/HTTPServer.cpp`

Add new route in `setupRoutes()`:
```cpp
// Around line 170, after existing liveview routes
server_->Get("/api/cameras/liveview/stream",
            [this](const httplib::Request &req, httplib::Response &res) {
              handleLiveViewSSE(req, res);
            });
```

Add new handler method (add to HTTPServer.cpp):
```cpp
void HTTPServer::handleLiveViewSSE(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);
  
  // Set SSE headers
  res.set_header("Content-Type", "text/event-stream");
  res.set_header("Cache-Control", "no-cache");
  res.set_header("Connection", "keep-alive");
  
  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    return;
  }
  
  // Stream frames while live view is active
  while (camMgr->isLiveViewActive()) {
    auto frame = camMgr->getLatestFrame();
    
    if (!frame.empty()) {
      // Encode to Base64
      std::string base64 = encodeBase64(frame);
      
      // Send as SSE event
      std::string event = "data: " + base64 + "\n\n";
      res.write(event.c_str(), event.length());
    }
    
    // 20 FPS
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}
```

Add to `HTTPServer.h`:
```cpp
void handleLiveViewSSE(const httplib::Request &req, httplib::Response &res);
```

### Task 2: Add Base64 Encoding Function (5 min)

Add to `HTTPServer.cpp` (if not exists):
```cpp
std::string HTTPServer::encodeBase64(const std::vector<uint8_t> &data) {
  static const char* base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";
  
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  size_t in_len = data.size();
  const unsigned char* bytes_to_encode = data.data();

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';
  }

  return ret;
}
```

### Task 3: Update Frontend to Use Electron API (20 min)

**File:** `frontend/src/hooks/useElectronCamera.ts` (NEW)

```typescript
import { useEffect, useState, useCallback } from 'react';

export function useElectronCamera() {
  const [cameras, setCameras] = useState([]);
  const [liveViewFrame, setLiveViewFrame] = useState<string | null>(null);
  const [isConnected, setIsConnected] = useState(true);
  
  useEffect(() => {
    // Check if running in Electron
    if (!window.electronAPI) {
      console.warn('Not running in Electron');
      return;
    }
    
    // Setup live view frame listener
    const cleanup = window.electronAPI.onLiveViewFrame((base64Data: string) => {
      // Convert Base64 to data URL
      setLiveViewFrame(`data:image/jpeg;base64,${base64Data}`);
    });
    
    // Setup backend error listener
    const errorCleanup = window.electronAPI.onBackendError((error) => {
      console.error('Backend error:', error);
      setIsConnected(false);
    });
    
    return () => {
      cleanup();
      errorCleanup();
    };
  }, []);
  
  const listCameras = useCallback(async () => {
    if (!window.electronAPI) return;
    
    const response = await window.electronAPI.listCameras();
    if (response.success) {
      setCameras(response.data);
    }
  }, []);
  
  const selectCamera = useCallback(async (type: string, index: number) => {
    if (!window.electronAPI) return { success: false };
    
    return await window.electronAPI.selectCamera({ type, index });
  }, []);
  
  const startLiveView = useCallback(async () => {
    if (!window.electronAPI) return { success: false };
    
    return await window.electronAPI.startLiveView();
  }, []);
  
  const stopLiveView = useCallback(async () => {
    if (!window.electronAPI) return { success: false };
    
    return await window.electronAPI.stopLiveView();
  }, []);
  
  const capture = useCallback(async () => {
    if (!window.electronAPI) return { success: false };
    
    return await window.electronAPI.capture();
  }, []);
  
  return {
    cameras,
    liveViewFrame,
    isConnected,
    listCameras,
    selectCamera,
    startLiveView,
    stopLiveView,
    capture
  };
}
```

**File:** `frontend/src/types/electron.d.ts` (NEW)

```typescript
export interface ElectronAPI {
  listCameras: () => Promise<{ success: boolean; data: any[] }>;
  selectCamera: (config: { type: string; index?: number }) => Promise<{ success: boolean }>;
  startLiveView: () => Promise<{ success: boolean }>;
  stopLiveView: () => Promise<{ success: boolean }>;
  onLiveViewFrame: (callback: (base64Data: string) => void) => () => void;
  capture: () => Promise<{ success: boolean; data?: { path: string } }>;
  onBackendError: (callback: (error: any) => void) => () => void;
  isElectron: () => boolean;
  platform: string;
  getBackendURL: () => string;
}

declare global {
  interface Window {
    electronAPI: ElectronAPI;
  }
}
```

**Update:** `frontend/src/components/LiveViewDisplay.tsx`

Replace `useLiveView` with `useElectronCamera`:
```typescript
import { useElectronCamera } from '../hooks/useElectronCamera';

export default function LiveViewDisplay() {
  const { liveViewFrame, isConnected, startLiveView, stopLiveView } = useElectronCamera();
  
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
      {!isConnected && <div className="error">Backend disconnected</div>}
    </div>
  );
}
```

## 🚀 Build & Test

### 1. Rebuild Backend with SSE Support
```bash
cd backend/build
cmake --build . --config Release
```

### 2. Test Electron App
```bash
# Terminal 1: Start Vite dev server
cd frontend
npm run dev

# Terminal 2: Start Electron (will spawn backend automatically)
cd ..
npm run dev:electron
```

## ✅ Expected Result

- ✅ Electron window opens
- ✅ Backend server spawns automatically
- ✅ Frontend loads in Electron
- ✅ Camera selection works via HTTP
- ✅ Live View streams via SSE (no WebSocket!)
- ✅ No disconnect issues
- ✅ Smooth 20 FPS stream

## 📊 Progress

- [x] Electron main process (15 min)
- [x] Preload script (5 min)
- [ ] Backend SSE endpoint (20 min) ⬅️ **NEXT**
- [ ] Frontend hook (15 min)
- [ ] Testing (10 min)

**Total Time:** 1 hour
**Completed:** 20 minutes
**Remaining:** 40 minutes
