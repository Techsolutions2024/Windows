#pragma once

#include "ICamera.h"
#include "CameraModel.h"
#include "Property.h"
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>

namespace photobooth {

// Extended camera settings with SDK property codes
struct CanonCameraSettings {
    // Exposure Control
    EdsUInt32 isoCode = 0x48;           // ISO 100
    EdsUInt32 apertureCode = 0x30;       // f/5.6
    EdsUInt32 shutterSpeedCode = 0x70;   // 1/125
    EdsUInt32 exposureCompCode = 0x00;   // 0
    EdsUInt32 meteringModeCode = 0;
    EdsUInt32 aeModeCode = 0;            // P mode

    // White Balance & Color
    EdsUInt32 whiteBalanceCode = 0;      // Auto
    EdsUInt32 pictureStyleCode = 0x81;   // Standard

    // Focus & AF
    EdsUInt32 afModeCode = 0;            // One-Shot AF

    // Image Quality
    EdsUInt32 imageQualityCode = 0;      // Jpeg Large

    // Drive Mode
    EdsUInt32 driveModeCode = 0;         // Single shooting

    // Live View
    EdsUInt32 evfOutputDevice = 2;       // Output to PC
    EdsUInt32 evfZoom = 1;               // 1x zoom

    // App-level settings (not SDK)
    bool mirror = false;
    int rotation = 0;                     // 0, 90, 180, 270
};

// SDK Option for UI dropdowns
struct SDKOption {
    EdsUInt32 code;
    std::string label;
};

// All supported values from camera for UI population
struct CanonSupportedValues {
    std::vector<SDKOption> iso;
    std::vector<SDKOption> aperture;
    std::vector<SDKOption> shutterSpeed;
    std::vector<SDKOption> exposureComp;
    std::vector<SDKOption> whiteBalance;
    std::vector<SDKOption> pictureStyle;
    std::vector<SDKOption> afMode;
    std::vector<SDKOption> imageQuality;
    std::vector<SDKOption> driveMode;
    std::vector<SDKOption> aeMode;
};

class CanonSDKCamera : public ICamera {
public:
    CanonSDKCamera(EdsCameraRef camera, EdsUInt32 bodyID);
    ~CanonSDKCamera() override;

    // ICamera interface implementation
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    std::string getName() const override;
    CameraType getType() const override { return CameraType::Canon; }

    bool startLiveView(LiveViewCallback callback) override;
    void stopLiveView() override;
    bool isLiveViewActive() const override;

    void capture(CaptureMode mode, CaptureCallback callback) override;
    void captureWithCountdown(int seconds, CaptureMode mode, CaptureCallback callback) override;

    bool setSettings(const CameraSettings& settings) override;
    CameraSettings getSettings() const override;

    std::vector<int> getSupportedISO() const override;
    std::vector<std::string> getSupportedApertures() const override;
    std::vector<std::string> getSupportedShutterSpeeds() const override;
    std::vector<std::string> getSupportedWhiteBalances() const override;

    // Extended SDK methods
    bool setPropertyByCode(EdsPropertyID propertyID, EdsUInt32 code);
    EdsUInt32 getPropertyCode(EdsPropertyID propertyID) const;
    CanonSupportedValues getAllSupportedValues();
    CanonCameraSettings getExtendedSettings() const;
    bool setExtendedSettings(const CanonCameraSettings& settings);

    // Direct access to underlying CameraModel
    CameraModel* getCameraModel() { return cameraModel_.get(); }

    // Get last captured image path
    std::string getLastCapturedPath() const { return lastCapturedPath_; }

    // Set save directory for captures
    void setSaveDirectory(const std::string& dir) { saveDirectory_ = dir; }

private:
    std::unique_ptr<CameraModel> cameraModel_;
    EdsCameraRef cameraRef_;
    std::string name_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> liveViewActive_{false};
    std::atomic<bool> liveViewRunning_{false};

    LiveViewCallback liveViewCallback_;
    CaptureCallback captureCallback_;
    std::thread liveViewThread_;
    std::mutex mutex_;

    CanonCameraSettings extendedSettings_;
    std::string saveDirectory_ = "data/captures";
    std::string lastCapturedPath_;

    // Event callback context
    static CanonSDKCamera* currentInstance_;

    // Live view thread loop
    void liveViewLoop();

    // Download image from camera
    bool downloadImage(EdsDirectoryItemRef dirItem, CaptureResult& result);

    // Event handlers (static for EDSDK callback)
    static EdsError EDSCALLBACK handleObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid* context);
    static EdsError EDSCALLBACK handlePropertyEvent(EdsPropertyEvent event, EdsPropertyID property, EdsUInt32 param, EdsVoid* context);
    static EdsError EDSCALLBACK handleStateEvent(EdsStateEvent event, EdsUInt32 param, EdsVoid* context);

    // Utility methods
    std::vector<SDKOption> getPropertyDescWithLabels(
        EdsPropertyID propertyID,
        const std::map<EdsUInt32, const char*>& table);

    std::string generateFilename(const std::string& prefix);

    // Convert between basic CameraSettings and extended CanonCameraSettings
    void syncFromBasicSettings(const CameraSettings& settings);
    CameraSettings syncToBasicSettings() const;
};

} // namespace photobooth
