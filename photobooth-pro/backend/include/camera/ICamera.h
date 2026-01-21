#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace photobooth {

enum class CameraType { Canon, Webcam, Unknown };

enum class CaptureMode { Single, Burst, Video, GIF, Boomerang };

struct CameraSettings {
  int iso = 400;
  std::string aperture = "f/5.6";
  std::string shutterSpeed = "1/125";
  std::string whiteBalance = "Auto";
  bool mirror = false;
  int rotation = 0; // 0, 90, 180, 270
};

struct CaptureResult {
  bool success = false;
  std::string filePath;
  std::vector<uint8_t> imageData;
  int width = 0;
  int height = 0;
  std::string errorMessage;
};

using LiveViewCallback =
    std::function<void(const std::vector<uint8_t> &, int width, int height)>;
using CaptureCallback = std::function<void(const CaptureResult &)>;

class ICamera {
public:
  virtual ~ICamera() = default;

  // Connection
  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual bool isConnected() const = 0;

  // Camera info
  virtual std::string getName() const = 0;
  virtual CameraType getType() const = 0;

  // Live view
  virtual bool startLiveView(LiveViewCallback callback) = 0;
  virtual void stopLiveView() = 0;
  virtual bool isLiveViewActive() const = 0;

  // Capture
  virtual void capture(CaptureMode mode, CaptureCallback callback) = 0;
  virtual void captureWithCountdown(int seconds, CaptureMode mode,
                                    CaptureCallback callback) = 0;

  // Settings
  virtual bool setSettings(const CameraSettings &settings) = 0;
  virtual CameraSettings getSettings() const = 0;
  virtual std::vector<int> getSupportedISO() const = 0;
  virtual std::vector<std::string> getSupportedApertures() const = 0;
  virtual std::vector<std::string> getSupportedShutterSpeeds() const = 0;
  virtual std::vector<std::string> getSupportedWhiteBalances() const = 0;
};

} // namespace photobooth
