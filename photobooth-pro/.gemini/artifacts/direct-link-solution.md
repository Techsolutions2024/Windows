# Solution: Direct Link MultiCamCui Source Files  

## 🎯 Correct Approach (User Feedback)

User chỉ ra: "**Bạn hãy gọi trực tiếp từ đó ra**" - không cần spawn subprocess!

### ✅ ĐÚNG: Link source files
```cmake
# In CMakeLists.txt
set(MULTICAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MultiCamCui")

# Add MultiCamCui source files directly
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    
    # Add MultiCamCui sources
    ${MULTICAM_DIR}/src/CameraModel.cpp
    ${MULTICAM_DIR}/src/CameraEvent.cpp
    ${MULTICAM_DIR}/src/FileControl.cpp
    ${MULTICAM_DIR}/src/PreSetting.cpp
    ${MULTICAM_DIR}/src/utility.cpp
    # Skip MultiCamCui.cpp (has its own main())
    
    src/camera/CameraManager.cpp
    src/api/HTTPServer.cpp
    ...
)

# Add MultiCamCui includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MULTICAM_DIR}/include
    ${EDSDK_INCLUDE_DIR}
)
```

### ❌ SAI: Spawn subprocess (đã làm)
```cpp
// ProcessManager - TOO COMPLEX!
ProcessManager pm("MultiCamCui.exe");
pm.initialize();  // Spawn process
pm.sendCommand("3\n");  // Communicate via pipes
```

## 📝 Implementation Plan

### Step 1: Extract Command Handlers from MultiCamCui.cpp

MultiCamCui.cpp có main() loop với các if blocks:
```cpp
// Line 320-357: Command 1 - Set Save To
if (control_number == "1") { ... }

// Line 360-382: Command 2 - Set Image Quality  
if (control_number == "2") { ... }

// Line 385-401: Command 3 - Take Picture
else if (control_number == "3") {
    for (i = 0; i < _openedCamerasArray.size(); i++) {
        _openedCamerasArray[i]->TakePicture(kEdsCameraCommand_ShutterButton_Completely_NonAF);
    }
}
```

### Step 2: Wrap in Reusable Functions

Create `CanonSDKWrapper.h/cpp`:
```cpp
#include "CameraModel.h"

class CanonSDKWrapper {
private:
    std::vector<CameraModel*> openedCameras_;
    
public:
    bool initialize();
    bool detectCameras();
    bool selectCamera(int index);
    
    // Command wrappers
    bool setSaveTo(int option);              // Command 1
    bool setImageQuality(int quality);       // Command 2
    bool takePicture();                      // Command 3
    bool pressHalfway();                     // Command 4
    bool pressCompletely();                  // Command 5
    bool pressOff();                         // Command 6
    bool setTV(int value);                   // Command 7
    bool setAV(int value);                   // Command 8
    bool setISO(int value);                  // Command 9
    // ... commands 10-83
    
    // Live view
    bool getLiveViewImage();                 // Command 20
    bool startLiveView();                    // Command 21
    bool stopLiveView();                     // Command 22
};
```

### Step 3: Implement Using MultiCamCui Code

```cpp
// CanonSDKWrapper.cpp
#include "CanonSDKWrapper.h"

bool CanonSDKWrapper::takePicture() {
    // Copy from MultiCamCui.cpp line 385-401
    EdsBool pause_flg = 0;
    for (size_t i = 0; i < openedCameras_.size(); i++) {
        EdsError err = openedCameras_[i]->TakePicture(
            kEdsCameraCommand_ShutterButton_Completely_NonAF
        );
        pause_flg = pause_flg | (err != EDS_ERR_OK);
    }
    return !pause_flg;
}

bool CanonSDKWrapper::startLiveView() {
    // Find command 21 implementation in MultiCamCui.cpp
    // Copy that code here
    for (auto* camera : openedCameras_) {
        camera->StartLiveView();  // Or whatever the actual method is
    }
    return true;
}
```

### Step 4: Update CMakeLists.txt

```cmake
# Remove ProcessManager
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/camera/CameraManager.cpp
    src/camera/WebcamCamera.cpp
    # Remove: src/camera/ProcessManager.cpp
    
    # Add Canon SDK Wrapper
    src/camera/CanonSDKWrapper.cpp
    
    # Add MultiCamCui sources (EXCEPT MultiCamCui.cpp which has main())
    MultiCamCui/src/CameraModel.cpp
    MultiCamCui/src/CameraEvent.cpp
    MultiCamCui/src/FileControl.cpp
    MultiCamCui/src/PreSetting.cpp
    MultiCamCui/src/utility.cpp
    
    src/api/HTTPServer.cpp
    src/storage/DatabaseManager.cpp
)

# Add MultiCamCui includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MultiCamCui/include
    ${EDSDK_INCLUDE_DIR}
)
```

### Step 5: Remove Duplicate Handlers

**Fix linker error:**
```cmake
# REMOVE this file from sources:
# src/api/HTTPServerLiveView.cpp

# Keep implementations in HTTPServer.cpp only
```

## 🚀 Quick Win Actions

### Immediate (10 minutes):
1. **Remove HTTPServerLiveView.cpp** from CMakeLists.txt
2. **Add MultiCamCui source files** to CMakeLists.txt  
3. **Build** - should compile!
4. **Test** basic camera detection

### Next (30 minutes):
1. Create `CanonSDKWrapper` class
2. Extract command implementations from MultiCamCui.cpp
3. Wire up to HTTP endpoints
4. Test!

## 📊 Benefits Over ProcessManager

| Approach | Speed | Complexity | Maintenance |
|----------|-------|------------|-------------|
| **ProcessManager (subprocess)** | Slow (IPC overhead) | High (pipes, parsing) | Hard (2 processes) |
| **Direct Link (library)** | Fast (function calls) | Low (just wrap) | Easy (single process) |

## 💡 Why This is Better

1. **No IPC overhead** - Direct function calls
2. **No parsing** - No stdin/stdout text parsing
3. **No process management** - No spawn/kill logic
4. **Easier debugging** - Single process
5. **Faster** - No subprocess latency
6. **Simpler** - Just wrap existing functions

## ✅ Next Steps

**Bạn muốn tôi:**
1. **Implement CanonSDKWrapper** with direct linking (30 min)
2. **Fix linker error first** + build (5 min) then decide
3. **Create detailed extraction guide** for all 83 commands

Chọn gì? 🎯
