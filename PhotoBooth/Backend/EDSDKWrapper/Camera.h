#pragma once

#include "../../../EDSDK/Header/EDSDK.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace PhotoBooth {
namespace EDSDK {

// Camera event callbacks
using CameraConnectedCallback = std::function<void(EdsCameraRef camera)>;
using CameraDisconnectedCallback = std::function<void()>;
using PhotoCapturedCallback = std::function<void(const std::string& filePath)>;
using LiveViewFrameCallback = std::function<void(const unsigned char* data, size_t size, int width, int height)>;
using ErrorCallback = std::function<void(EdsError error, const std::string& message)>;

// Camera settings structure
struct CameraSettings {
    std::string iso;
    std::string aperture;
    std::string shutterSpeed;
    std::string whiteBalance;
    std::string imageQuality;
    int saveTo; // kEdsSaveTo_Host, kEdsSaveTo_Camera, kEdsSaveTo_Both
};

// Camera information
struct CameraInfo {
    std::string modelName;
    std::string serialNumber;
    std::string firmwareVersion;
    int batteryLevel;
    std::string lensName;
};

class Camera {
public:
    Camera();
    ~Camera();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Camera connection
    bool ConnectCamera(int cameraIndex = 0);
    bool DisconnectCamera();
    bool IsConnected() const;
    
    // Camera list
    std::vector<std::string> GetCameraList();
    int GetCameraCount();

    // Camera information
    CameraInfo GetCameraInfo();
    int GetBatteryLevel();
    int GetAvailableShots();

    // Session management
    bool OpenSession();
    bool CloseSession();

    // Camera settings
    bool SetISO(const std::string& iso);
    bool SetAperture(const std::string& aperture);
    bool SetShutterSpeed(const std::string& shutterSpeed);
    bool SetWhiteBalance(const std::string& wb);
    bool SetImageQuality(const std::string& quality);
    bool SetSaveDestination(int saveTo);
    
    CameraSettings GetCurrentSettings();
    bool ApplySettings(const CameraSettings& settings);

    // Property lists (available values)
    std::vector<std::string> GetAvailableISO();
    std::vector<std::string> GetAvailableApertures();
    std::vector<std::string> GetAvailableShutterSpeeds();
    std::vector<std::string> GetAvailableWhiteBalance();

    // Live view
    bool StartLiveView();
    bool StopLiveView();
    bool IsLiveViewActive() const;
    bool DownloadLiveViewFrame();
    
    // Live view settings
    bool SetLiveViewZoom(int zoomLevel); // 1x, 5x, 10x
    bool SetLiveViewZoomPosition(int x, int y);
    bool EnableLiveViewHistogram(bool enable);

    // Capture
    bool TakePicture();
    bool StartBulbCapture();
    bool EndBulbCapture();
    
    // Focus control
    bool AutoFocus();
    bool SetFocusMode(int mode);
    bool DriveLens(int direction); // Near/Far
    
    // File operations
    bool DownloadFile(EdsDirectoryItemRef fileRef, const std::string& savePath);
    std::vector<std::string> GetCameraFiles();
    bool DeleteFile(const std::string& fileName);

    // Callbacks
    void SetCameraConnectedCallback(CameraConnectedCallback callback);
    void SetCameraDisconnectedCallback(CameraDisconnectedCallback callback);
    void SetPhotoCapturedCallback(PhotoCapturedCallback callback);
    void SetLiveViewFrameCallback(LiveViewFrameCallback callback);
    void SetErrorCallback(ErrorCallback callback);

    // Error handling
    std::string GetLastError() const;
    static std::string ErrorToString(EdsError error);

private:
    // EDSDK objects
    EdsCameraListRef cameraList_;
    EdsCameraRef camera_;
    EdsEvfImageRef evfImage_;
    
    // State
    bool initialized_;
    bool connected_;
    bool sessionOpen_;
    bool liveViewActive_;
    std::string lastError_;
    
    // Settings
    CameraSettings currentSettings_;
    CameraInfo cameraInfo_;
    
    // Callbacks
    CameraConnectedCallback onCameraConnected_;
    CameraDisconnectedCallback onCameraDisconnected_;
    PhotoCapturedCallback onPhotoCaptured_;
    LiveViewFrameCallback onLiveViewFrame_;
    ErrorCallback onError_;
    
    // Download path
    std::string downloadPath_;

    // Internal methods
    bool InitializeSDK();
    bool GetCameraList();
    bool LoadCameraInfo();
    bool SetProperty(EdsPropertyID propertyID, EdsUInt32 value);
    bool GetProperty(EdsPropertyID propertyID, EdsUInt32& value);
    bool GetPropertyDesc(EdsPropertyID propertyID, EdsPropertyDesc& desc);
    
    // Static callbacks for EDSDK
    static EdsError EDSCALLBACK HandleObjectEvent(
        EdsObjectEvent event,
        EdsBaseRef object,
        EdsVoid* context);
    
    static EdsError EDSCALLBACK HandlePropertyEvent(
        EdsPropertyEvent event,
        EdsPropertyID propertyID,
        EdsUInt32 param,
        EdsVoid* context);
    
    static EdsError EDSCALLBACK HandleStateEvent(
        EdsStateEvent event,
        EdsUInt32 param,
        EdsVoid* context);
    
    static EdsError EDSCALLBACK HandleCameraAddedEvent(
        EdsVoid* context);

    // Helper methods
    void NotifyError(EdsError error, const std::string& message);
    std::string PropertyValueToString(EdsPropertyID propertyID, EdsUInt32 value);
    EdsUInt32 StringToPropertyValue(EdsPropertyID propertyID, const std::string& str);
};

} // namespace EDSDK
} // namespace PhotoBooth
