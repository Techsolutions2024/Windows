#include "camera/CameraManager.h"
#include <iostream>

namespace photobooth {

CameraManager::CameraManager() : activeCamera_(nullptr), initialized_(false) {}

CameraManager::~CameraManager() { shutdown(); }

bool CameraManager::initialize() {
  initialized_ = true;
  return true;
}

void CameraManager::shutdown() { initialized_ = false; }

std::vector<std::string> CameraManager::detectCameras() {
  return {"Mock Camera"};
}

bool CameraManager::selectCamera(const std::string &cameraName) { return true; }

ICamera *CameraManager::getActiveCamera() { return nullptr; }

std::string CameraManager::getActiveCameraName() const { return "Mock Camera"; }

bool CameraManager::startLiveView(LiveViewCallback callback) { return true; }

void CameraManager::stopLiveView() {}

void CameraManager::capture(CaptureMode mode, CaptureCallback callback) {
  if (callback) {
    CaptureResult result;
    result.success = true;
    result.filePath = "mock_image.jpg";
    callback(result);
  }
}

void CameraManager::captureWithCountdown(int seconds, CaptureMode mode,
                                         CaptureCallback callback) {
  if (callback) {
    CaptureResult result;
    result.success = true;
    result.filePath = "mock_image_countdown.jpg";
    callback(result);
  }
}

bool CameraManager::setSettings(const CameraSettings &settings) { return true; }

CameraSettings CameraManager::getSettings() const { return CameraSettings(); }

void CameraManager::detectCanonCameras() {}
void CameraManager::detectWebcams() {}

} // namespace photobooth
