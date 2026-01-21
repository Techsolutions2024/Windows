#include "camera/CameraManager.h"
#include "EDSDK.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include "camera/CanonCamera.h"
#include "camera/WebcamCamera.h"
#endif

// ...

// Remove duplicate definition if it was here.
// The previous "void CameraManager::detectWebcams() {}" was likely around line
// 205. We will target the end of file area to remove it if it exists, or just
// ensure proper structure.

// Since I cannot see the exact lines 200+ anymore easily without view_file, and
// I know I added one at the top. Let's first add the include.

namespace photobooth {
// ...

CameraManager::CameraManager() : activeCamera_(nullptr), initialized_(false) {}

CameraManager::~CameraManager() { shutdown(); }

bool CameraManager::initialize() {
  if (initialized_)
    return true;

  EdsError err = EdsInitializeSDK();
  if (err == EDS_ERR_OK) {
    initialized_ = true;
    std::cout << "EDSDK initialized successfully." << std::endl;
    return true;
  } else {
    std::cerr << "Failed to initialize EDSDK. Error: " << err << std::endl;
    return false;
  }
}

void CameraManager::shutdown() {
  if (activeCamera_) {
    activeCamera_->disconnect();
    delete activeCamera_;
    activeCamera_ = nullptr;
  }

  if (initialized_) {
    EdsTerminateSDK();
    initialized_ = false;
    std::cout << "EDSDK terminated." << std::endl;
  }
}

void CameraManager::detectWebcams() {
  // TODO: Implement DirectShow enumeration
}

// ... (previous content)

std::vector<std::string> CameraManager::detectCameras() {
  if (!initialized_)
    return {};

  EdsCameraListRef cameraList = nullptr;
  EdsUInt32 count = 0;
  std::vector<std::string> cameras;

  // 1. Detect Canon Cameras
  EdsError err = EdsGetCameraList(&cameraList);
  if (err == EDS_ERR_OK) {
    err = EdsGetChildCount(cameraList, &count);
    if (err == EDS_ERR_OK) {
      for (EdsUInt32 i = 0; i < count; i++) {
        EdsCameraRef camRef = nullptr;
        err = EdsGetChildAtIndex(cameraList, i, &camRef);
        if (err == EDS_ERR_OK) {
          EdsDataType dataType;
          EdsUInt32 dataSize;
          char name[256];
          err = EdsGetPropertySize(camRef, kEdsPropID_ProductName, 0, &dataType,
                                   &dataSize);
          if (err == EDS_ERR_OK) {
            EdsGetPropertyData(camRef, kEdsPropID_ProductName, 0, dataSize,
                               name);
            cameras.push_back(std::string(name));
          }
          EdsRelease(camRef);
        }
      }
    }
    EdsRelease(cameraList);
  }

  // 2. Add Webcam (Mock for now, or DShow list)
  cameras.push_back("Webcam");

  return cameras;
}

bool CameraManager::selectCamera(const std::string &cameraName) {
  if (!initialized_)
    return false;

  // Close existing
  if (activeCamera_) {
    activeCamera_->disconnect();
    delete activeCamera_;
    activeCamera_ = nullptr;
  }

  if (cameraName == "Webcam") {
    activeCamera_ = new WebcamCamera(0, "Webcam");
    return activeCamera_->connect();
  }

  // Canon selection logic
  EdsCameraListRef cameraList = nullptr;
  EdsUInt32 count = 0;
  bool found = false;

  EdsError err = EdsGetCameraList(&cameraList);
  if (err == EDS_ERR_OK) {
    err = EdsGetChildCount(cameraList, &count);
    if (err == EDS_ERR_OK) {
      for (EdsUInt32 i = 0; i < count; i++) {
        EdsCameraRef camRef = nullptr;
        err = EdsGetChildAtIndex(cameraList, i, &camRef);
        if (err == EDS_ERR_OK) {
          EdsDataType dataType;
          EdsUInt32 dataSize;
          char name[256];
          EdsGetPropertySize(camRef, kEdsPropID_ProductName, 0, &dataType,
                             &dataSize);
          EdsGetPropertyData(camRef, kEdsPropID_ProductName, 0, dataSize, name);

          if (cameraName.empty() || std::string(name) == cameraName ||
              cameraName == "Auto") {
            activeCamera_ = new CanonCamera(camRef);
            if (activeCamera_->connect()) {
              found = true;
              break;
            } else {
              delete activeCamera_;
              activeCamera_ = nullptr;
              EdsRelease(camRef);
            }
          } else {
            EdsRelease(camRef);
          }
        }
      }
    }
    EdsRelease(cameraList);
  }

  return found;
}

ICamera *CameraManager::getActiveCamera() { return activeCamera_; }

std::string CameraManager::getActiveCameraName() const {
  if (activeCamera_)
    return activeCamera_->getName();
  return "";
}

std::vector<CameraInfo> CameraManager::getAvailableCameras() const {
  std::vector<CameraInfo> cameras;

  if (!initialized_)
    return cameras;

  // Detect Canon cameras
  EdsCameraListRef cameraList = nullptr;
  EdsUInt32 count = 0;

  EdsError err = EdsGetCameraList(&cameraList);
  if (err == EDS_ERR_OK) {
    err = EdsGetChildCount(cameraList, &count);
    if (err == EDS_ERR_OK) {
      for (EdsUInt32 i = 0; i < count; i++) {
        EdsCameraRef camRef = nullptr;
        err = EdsGetChildAtIndex(cameraList, i, &camRef);
        if (err == EDS_ERR_OK) {
          EdsDataType dataType;
          EdsUInt32 dataSize;
          char name[256];
          err = EdsGetPropertySize(camRef, kEdsPropID_ProductName, 0, &dataType,
                                   &dataSize);
          if (err == EDS_ERR_OK) {
            EdsGetPropertyData(camRef, kEdsPropID_ProductName, 0, dataSize,
                               name);
            CameraInfo info;
            info.name = std::string(name);
            info.type = CameraType::Canon;
            info.connected = (activeCamera_ && activeCamera_->getName() == info.name);
            info.webcamIndex = -1;
            cameras.push_back(info);
          }
          EdsRelease(camRef);
        }
      }
    }
    EdsRelease(cameraList);
  }

  return cameras;
}

bool CameraManager::selectWebcam(int deviceIndex) {
  // Close existing
  if (activeCamera_) {
    activeCamera_->disconnect();
    delete activeCamera_;
    activeCamera_ = nullptr;
  }

  std::string name = "Webcam " + std::to_string(deviceIndex);
  activeCamera_ = new WebcamCamera(deviceIndex, name);
  return activeCamera_->connect();
}

bool CameraManager::startLiveView(LiveViewCallback callback) {
  if (activeCamera_)
    return activeCamera_->startLiveView(callback);
  return false;
}

void CameraManager::stopLiveView() {
  if (activeCamera_)
    activeCamera_->stopLiveView();
}

void CameraManager::capture(CaptureMode mode, CaptureCallback callback) {
  if (activeCamera_) {
    activeCamera_->capture(mode, callback);
  } else {
    if (callback) {
      CaptureResult res;
      res.success = false;
      res.errorMessage = "No active camera";
      callback(res);
    }
  }
}

void CameraManager::captureWithCountdown(int seconds, CaptureMode mode,
                                         CaptureCallback callback) {
  if (activeCamera_) {
    activeCamera_->captureWithCountdown(seconds, mode, callback);
  } else {
    if (callback) {
      CaptureResult res;
      res.success = false;
      res.errorMessage = "No active camera";
      callback(res);
    }
  }
}

bool CameraManager::setSettings(const CameraSettings &settings) {
  if (activeCamera_)
    return activeCamera_->setSettings(settings);
  return false;
}

CameraSettings CameraManager::getSettings() const {
  if (activeCamera_)
    return activeCamera_->getSettings();
  return CameraSettings();
}

// ... (previous content)
void CameraManager::detectCanonCameras() {}
// detectWebcams implemented earlier

std::vector<int> CameraManager::getSupportedISO() const {
  if (activeCamera_)
    return activeCamera_->getSupportedISO();
  return {};
}

std::vector<std::string> CameraManager::getSupportedApertures() const {
  if (activeCamera_)
    return activeCamera_->getSupportedApertures();
  return {};
}

std::vector<std::string> CameraManager::getSupportedShutterSpeeds() const {
  if (activeCamera_)
    return activeCamera_->getSupportedShutterSpeeds();
  return {};
}

std::vector<std::string> CameraManager::getSupportedWhiteBalances() const {
  if (activeCamera_)
    return activeCamera_->getSupportedWhiteBalances();
  return {};
}

} // namespace photobooth
