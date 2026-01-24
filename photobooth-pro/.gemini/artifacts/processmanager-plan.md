# ProcessManager Implementation Plan

##  Đã Tạo

✅ **ProcessManager.h** - Header file với:
- Process lifecycle management (initialize, shutdown)
- Camera detection & selection (detectCameras, selectCamera)
- Live View commands (startLiveView, stopLiveView, getLiveViewFrame)
- Capture commands (takePicture, pressHalfway, etc.)
- Settings commands (setSaveTo, setISO, setTV, setAV)
- I/O helpers (stdin/stdout communication)

## 🎯 Kiến Trúc ProcessManager

```
Backend Server
    ↓
ProcessManager
    ↓ spawn subprocess
MultiCamCui.exe
    ↓ stdin (commands like "21\n")
    ↓ stdout (output text + binary data)
Canon Camera via SDK
```

### Flow Diagram:

**Initialization:**
```
1. ProcessManager::initialize()
2. CreateProcess("MultiCamCui.exe")
3. Wait for top menu prompt
4. ProcessManager::selectCamera(1)  // Send "1\n" to stdin
5. Wait for control menu prompt
6. Ready for commands
```

**Live View:**
```
1. startLiveView()  → Send "21\n" → Enable live view mode
2. Loop:
   - getLiveViewFrame() → Send "20\n" → Get one JPEG frame
   - Read binary data from stdout
   - Parse JPEG bytes
   - Return to application
3. stopLiveView() → Send "22\n" → Disable live view mode
```

**Capture:**
```
1. takePicture() → Send "3\n"
2. MultiCamCui saves to cam{N}/filename.jpg
3. Read stdout to get filename
4. Return path to application
```

## 📝 Implementation Status

### ✅ Done:
1. ProcessManager.h created
2. Command protocol defined

### ⏳ TODO:
1. **ProcessManager.cpp** - Implementation (~500 lines):
   - createProcess() - Windows CreateProcess API
   - writeToStdin() - Write commands
   - readFromStdout() - Read text output
   - getLiveViewFrame() - Read binary JPEG data
   - All public method implementations

2. **Integration với HTTPServer** - Update endpoints:
   - Replace CameraManager calls với ProcessManager
   - Update handleStartLiveView()
   - Update handleLiveViewSSE() để dùng ProcessManager

3. **Build MultiCamCui.exe** - Canon SDK sample:
   ```bash
   cd backend/MultiCamCui
   cmake --build . --config Release
   ```

4. **Update CMakeLists.txt**:
   - Add ProcessManager.cpp to sources
   - Add post-build copy MultiCamCui.exe

## ⚠️ Challenges

### Challenge 1: Binary Data Parsing
MultiCamCui outputs:
- Text (prompts, menus)
- Binary JPEG data (for live view frames)

Need to:
- Detect when binary data starts
- Read exact JPEG size
- Parse JPEG markers (0xFFD8 start, 0xFFD9 end)

### Challenge 2: Process Synchronization  
- Need to wait for prompts before sending next command
- Timeout handling for unresponsive camera
- Thread-safe I/O operations

### Challenge 3: Error Handling
- MultiCamCui may fail to find camera
- Camera may disconnect mid-operation
- Need graceful fallback

## 💡 Estimated Implementation Time

| Task | Time |
|------|------|
| ProcessManager.cpp basic | 1 hour |
| Binary JPEG parsing | 30 min |
| Error handling & timeouts | 30 min |
| HTTPServer integration | 30 min |
| Build & test MultiCamCui | 30 min |
| End-to-end testing | 1 hour |
| **Total** | **4 hours** |

## 🚀 Alternative: Quick Prototype

### Simplified Approach (30 minutes):
1. **Skip ProcessManager for now**
2. **Just call MultiCamCui.exe directly** from HTTPServer:
   ```cpp
   system("cd MultiCamCui && MultiCamCui.exe < commands.txt");
   ```
3. **Use file-based communication**:
   - HTTP endpoint writes command to file
   - MultiCamCui reads file, executes command
   - Saves output to another file
   - HTTP endpoint reads result file

**Pros:**
- Very fast to implement
- Can test Canon SDK integration immediately
- No complex process management

**Cons:**
- Not production-ready
- Slow (file I/O overhead)
- No live view streaming support

## ❓ Quyết Định

Bạn muốn:

**Option A: Implement Full ProcessManager** (4 hours)
- Production-ready
- Live view streaming works
- Proper error handling
- Best performance

**Option B: Quick File-Based Prototype** (30 min)
- Test Canon SDK ngay
- Verify MultiCamCui.exe hoạt động
- SAU ĐÓ implement ProcessManager đầy đủ

**Option C: Hybrid** (2 hours)
- Implement basic ProcessManager (chỉ text commands)
- Skip binary JPEG parsing (dùng file-based cho live view)
- Test capture functionality trước

---

**Khuyến nghị của tôi:** **Option C - Hybrid**
1. Implement basic ProcessManager với text commands (1 hour)
2. Test camera detection và capture (30 min)
3. SAU ĐÓ add binary parsing cho live view (30 min)

Này cho phép iterate nhanh và test từng phần.

Bạn chọn option nào? 🎯
