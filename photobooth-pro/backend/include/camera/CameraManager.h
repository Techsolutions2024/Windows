#pragma once

#include "ICamera.h"
#include "camera/CanonSDKCamera.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace photobooth {

// Camera info for API responses
struct CameraInfo {
  std::string name;
  CameraType type;
  bool connected;
  int index;  // Camera index in list
};

class CameraManager {
public:
  CameraManager();
  ~CameraManager();

  // Camera discovery
  bool initialize();
  void shutdown();
  std::vector<std::string> detectCameras();
  std::vector<CameraInfo> getAvailableCameras() const;

  // Camera selection
  bool selectCamera(const std::string &cameraName);
  ICamera *getActiveCamera();
  std::string getActiveCameraName() const;

  // Quick access methods (delegates to active camera)
  bool startLiveView(LiveViewCallback callback);
  void stopLiveView();
  void capture(CaptureMode mode, CaptureCallback callback);
  void captureWithCountdown(int seconds, CaptureMode mode,
                            CaptureCallback callback);
  bool setSettings(const CameraSettings &settings);
  CameraSettings getSettings() const;
  std::vector<int> getSupportedISO() const;
  std::vector<std::string> getSupportedApertures() const;
  std::vector<std::string> getSupportedShutterSpeeds() const;
  std::vector<std::string> getSupportedWhiteBalances() const;

  // Extended SDK methods (Canon-specific)
  CanonSupportedValues getAllSupportedCameraValues();
  CanonCameraSettings getExtendedCameraSettings() const;
  bool setExtendedCameraSettings(const CanonCameraSettings& settings);
  bool setCameraPropertyByCode(EdsPropertyID propertyID, EdsUInt32 code);

  // Get Canon SDK camera (for direct access)
  CanonSDKCamera* getCanonCamera();

  // Set save directory for captures
  void setSaveDirectory(const std::string& dir);

private:
  std::unique_ptr<CanonSDKCamera> activeCamera_;
  std::mutex mutex_;
  bool initialized_;
  std::string saveDirectory_;

  void detectCanonCameras();
};

} // namespace photobooth
