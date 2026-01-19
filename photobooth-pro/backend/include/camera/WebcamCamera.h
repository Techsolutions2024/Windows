#pragma once

#include "ICamera.h"
#include <thread>
#include <atomic>
#include <windows.h>
#include <dshow.h>

namespace photobooth {

class WebcamCamera : public ICamera {
public:
    WebcamCamera(int deviceIndex, const std::string& name);
    ~WebcamCamera() override;

    // ICamera implementation
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;

    std::string getName() const override { return name_; }
    CameraType getType() const override { return CameraType::Webcam; }

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

private:
    int deviceIndex_;
    std::string name_;
    bool connected_;
    
    // DirectShow interfaces
    IGraphBuilder* graphBuilder_;
    ICaptureGraphBuilder2* captureBuilder_;
    IMediaControl* mediaControl_;
    IBaseFilter* sourceFilter_;
    
    // Live view
    std::atomic<bool> liveViewActive_;
    std::thread liveViewThread_;
    LiveViewCallback liveViewCallback_;
    
    // Capture
    CaptureCallback captureCallback_;
    CameraSettings settings_;

    // Helper methods
    void liveViewLoop();
    bool initializeDirectShow();
    void cleanupDirectShow();
    bool captureFrame(std::vector<uint8_t>& frameData, int& width, int& height);
    void applyMirrorAndRotation(std::vector<uint8_t>& imageData, int& width, int& height);
};

} // namespace photobooth
