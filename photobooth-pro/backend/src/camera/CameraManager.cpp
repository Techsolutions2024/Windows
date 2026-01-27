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
#include "core/SharedMemoryManager.h"
#endif

namespace photobooth {

CameraManager::CameraManager() : activeCamera_(nullptr), initialized_(false) {
#ifdef _WIN32
  sharedMemory_ = std::make_unique<SharedMemoryManager>();
  // 20MB buffer to be safe for 24MP+ images if raw, but high quality JPEG is usually 5-10MB max.
  // We use "Local\\CanonLiveView" as the map name.
  sharedMemory_->initialize("Local\\CanonLiveView", 20 * 1024 * 1024);
#endif
}

CameraManager::~CameraManager() { shutdown(); }

bool CameraManager::initialize() {
  if (initialized_)
    return true;

  EdsError err = EdsInitializeSDK();
  if (err == EDS_ERR_OK) {
    initialized_ = true;
    std::cout << "EDSDK initialized successfully." << std::endl;
    // Wait for SDK to detect USB-connected cameras
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
  // Removed: Production system uses Canon EDSDK only
}

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
          EdsDeviceInfo deviceInfo;
          err = EdsGetDeviceInfo(camRef, &deviceInfo);
          if (err == EDS_ERR_OK) {
            cameras.push_back(std::string(deviceInfo.szDeviceDescription));
          }
          EdsRelease(camRef);
        }
      }
    }
    EdsRelease(cameraList);
  }

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
          EdsDeviceInfo deviceInfo;
          EdsGetDeviceInfo(camRef, &deviceInfo);

          if (cameraName.empty() || std::string(deviceInfo.szDeviceDescription) == cameraName ||
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
          EdsDeviceInfo deviceInfo;
          err = EdsGetDeviceInfo(camRef, &deviceInfo);
          if (err == EDS_ERR_OK) {
            CameraInfo info;
            info.name = std::string(deviceInfo.szDeviceDescription);
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
  // Webcam support removed - Canon EDSDK only
  std::cerr << "Webcam not supported in production build. Please use Canon camera." << std::endl;
  return false;
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

bool CameraManager::startMjpegStream() {
  if (!activeCamera_) return false;
  if (mjpegStreaming_) {
    streamClients_++;
    return true;
  }

  // Stop any existing live view
  activeCamera_->stopLiveView();

  // Start live view with callback that feeds the frame buffer
  auto callback = [this](const std::vector<uint8_t> &data, int w, int h) {
    {
      std::lock_guard<std::mutex> lock(frameMutex_);
      latestFrame_ = data;
      frameSeq_++;
      
      // IPC: Write to Shared Memory for Electron
#ifdef _WIN32
      if (sharedMemory_) {
          sharedMemory_->writeFrame(data, w, h);
      }
#endif
    }
    frameCV_.notify_all();
  };

  if (activeCamera_->startLiveView(callback)) {
    mjpegStreaming_ = true;
    streamClients_++;
    std::cout << "MJPEG stream started" << std::endl;
    return true;
  }
  return false;
}

void CameraManager::stopMjpegStream() {
  int clients = --streamClients_;
  if (clients <= 0) {
    streamClients_ = 0;
    mjpegStreaming_ = false;
    stopLiveView();
    // Wake any waiting readers
    frameCV_.notify_all();
    std::cout << "MJPEG stream stopped" << std::endl;
  }
}

bool CameraManager::isMjpegStreaming() const {
  return mjpegStreaming_;
}

bool CameraManager::waitForFrame(std::vector<uint8_t> &frame, int timeoutMs) {
  std::unique_lock<std::mutex> lock(frameMutex_);
  uint64_t currentSeq = frameSeq_;
  if (frameCV_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                        [this, currentSeq] { return frameSeq_ > currentSeq || !mjpegStreaming_; })) {
    if (!mjpegStreaming_) return false;
    frame = latestFrame_;
    return !frame.empty();
  }
  return false; // timeout
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

void CameraManager::detectCanonCameras() {}

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
