#pragma once

#include "ICamera.h"
#include <memory>
#include <vector>
#include <mutex>

namespace photobooth {

class CameraManager {
public:
    CameraManager();
    ~CameraManager();

    // Camera discovery
    bool initialize();
    void shutdown();
    std::vector<std::string> detectCameras();

    // Camera selection
    bool selectCamera(const std::string& cameraName);
    ICamera* getActiveCamera();
    std::string getActiveCameraName() const;

    // Quick access methods (delegates to active camera)
    bool startLiveView(LiveViewCallback callback);
    void stopLiveView();
    void capture(CaptureMode mode, CaptureCallback callback);
    void captureWithCountdown(int seconds, CaptureMode mode, CaptureCallback callback);
    bool setSettings(const CameraSettings& settings);
    CameraSettings getSettings() const;

private:
    std::vector<std::unique_ptr<ICamera>> cameras_;
    ICamera* activeCamera_;
    std::mutex mutex_;
    bool initialized_;

    void detectCanonCameras();
    void detectWebcams();
};

} // namespace photobooth
