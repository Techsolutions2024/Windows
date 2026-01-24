#pragma once

#include "CameraModel.h"
#include <memory>
#include <string>
#include <vector>


namespace photobooth {

/**
 * CanonSDKWrapper - Wrapper around MultiCamCui Canon SDK functions
 * Provides simple API to call Canon SDK commands (1-83)
 */
class CanonSDKWrapper {
public:
  CanonSDKWrapper();
  ~CanonSDKWrapper();

  // Lifecycle
  bool initialize();
  void shutdown();
  bool isInitialized() const { return initialized_; }

  // Camera detection & selection
  int detectCameras();          // Returns count
  bool selectCamera(int index); // 0-based index
  bool selectCameraRange(int startIndex, int endIndex);
  bool selectAllCameras();

  // Command 1: Set Save To
  bool setSaveTo(int option);

  // Command 2: Set Image Quality
  bool setImageQuality(int quality);

  // Command 3: Take Picture and Download
  bool takePicture();

  // Command 4-6: Shutter Control
  bool pressHalfway();
  bool pressCompletely();
  bool pressOff();

  // Command 7-9: Basic Settings
  bool setTV(int value);
  bool setAV(int value);
  bool setISO(int value);

  // Command 10: White Balance
  bool setWhiteBalance(int value);

  // Command 11: Drive Mode
  bool setDriveMode(int value);

  // Command 12: Exposure Compensation
  bool setExposureCompensation(int value);

  // Command 20-22: Live View
  bool getLiveViewImage(); // Get one frame
  bool startLiveView();
  bool stopLiveView();

  // Get camera info
  std::string getCameraName(int index = 0) const;
  int getCameraCount() const { return static_cast<int>(openedCameras_.size()); }

private:
  bool initialized_;
  std::vector<CameraModel *> detectedCameras_;
  std::vector<CameraModel *> openedCameras_;

  // Helper to execute commands on all opened cameras
  bool executeOnAllCameras(std::function<bool(CameraModel *)> func);
};

} // namespace photobooth
