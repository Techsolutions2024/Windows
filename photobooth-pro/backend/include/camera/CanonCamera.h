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

  // Live view
  std::atomic<bool> liveViewActive_;
  std::thread liveViewThread_;
  LiveViewCallback liveViewCallback_;

  // Capture
  CaptureCallback captureCallback_;
  CameraSettings settings_;

  // Helper methods
  void liveViewLoop();
  bool downloadImage(EdsDirectoryItemRef dirItem, CaptureResult &result);
  bool setProperty(EdsPropertyID propertyID, EdsUInt32 value);
  bool getProperty(EdsPropertyID propertyID, EdsUInt32 &value);

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
