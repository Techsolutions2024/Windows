#pragma once

#include "EDSDK.h"
#include "ICamera.h"
#include <atomic>
#include <thread>

namespace photobooth {

class CanonCamera : public ICamera {
public:
  CanonCamera(EdsCameraRef camera);
  ~CanonCamera() override;

  // ICamera implementation
  bool connect() override;
  void disconnect() override;
  bool isConnected() const override;

  std::string getName() const override;
  CameraType getType() const override { return CameraType::Canon; }

  bool startLiveView(LiveViewCallback callback) override;
  void stopLiveView() override;
  bool isLiveViewActive() const override;

  void capture(CaptureMode mode, CaptureCallback callback) override;
  void captureWithCountdown(int seconds, CaptureMode mode,
                            CaptureCallback callback) override;

  bool setSettings(const CameraSettings &settings) override;
  CameraSettings getSettings() const override;
  std::vector<int> getSupportedISO() const override;
  std::vector<std::string> getSupportedApertures() const override;
  std::vector<std::string> getSupportedShutterSpeeds() const override;
  std::vector<std::string> getSupportedWhiteBalances() const override;

private:
  EdsCameraRef camera_;
  std::string name_;
  bool connected_;
  int lockCount_ = 0; // Track UI lock state

  // Live view
  std::atomic<bool> liveViewActive_;
  std::thread liveViewThread_;
  LiveViewCallback liveViewCallback_;

  // Capture
  CaptureCallback captureCallback_;
  CameraSettings settings_;

  // Production-grade Helper methods (derived from CameraModel.cpp)
  void liveViewLoop();
  bool downloadImage(EdsDirectoryItemRef dirItem, CaptureResult &result);

  // Robust Property Accessors
  EdsError getPropertySize(EdsPropertyID propertyID, EdsDataType &dataType, EdsUInt32 &dataSize) const;
  EdsError getPropertyData(EdsPropertyID propertyID, EdsVoid *data, EdsUInt32 dataSize) const;
  EdsError setPropertyData(EdsPropertyID propertyID, const EdsVoid *data, EdsUInt32 dataSize);

  // Typed Setters
  EdsError setPropertyUInt32(EdsPropertyID propertyID, EdsUInt32 value);
  EdsError setPropertyString(EdsPropertyID propertyID, const std::string &value);

  // Command Helpers
  EdsError sendCommand(EdsCameraCommand command, EdsInt32 param = 0);
  EdsError sendStatusCommand(EdsCameraStatusCommand command, EdsInt32 param = 0);

  // Critical Safety Methods
  EdsError uiLock();
  EdsError uiUnlock();
  EdsError setCapacity(); // Tell camera we have space on PC

  // EDSDK callbacks
  static EdsError EDSCALLBACK handleObjectEvent(EdsObjectEvent event,
                                                EdsBaseRef object,
                                                EdsVoid *context);
  static EdsError EDSCALLBACK handlePropertyEvent(EdsPropertyEvent event,
                                                  EdsPropertyID property,
                                                  EdsUInt32 param,
                                                  EdsVoid *context);
  static EdsError EDSCALLBACK handleStateEvent(EdsStateEvent event,
                                               EdsUInt32 param,
                                               EdsVoid *context);
};

} // namespace photobooth
