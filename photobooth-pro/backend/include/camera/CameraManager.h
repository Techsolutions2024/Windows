#pragma once

#include "ICamera.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace photobooth {

// Camera info for API responses
struct CameraInfo {
  std::string name;
  CameraType type;
  bool connected;
  int webcamIndex; // Only for webcams
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
  bool selectWebcam(int deviceIndex);
  ICamera *getActiveCamera();
  std::string getActiveCameraName() const;

  // MJPEG streaming (production live view)
  bool startMjpegStream();
  void stopMjpegStream();
  bool isMjpegStreaming() const;
  bool waitForFrame(std::vector<uint8_t> &frame, int timeoutMs);

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

private:
  std::vector<std::unique_ptr<ICamera>> cameras_;
  ICamera *activeCamera_;
  std::mutex mutex_;
  bool initialized_;

  // Frame buffer for streaming
  std::vector<uint8_t> latestFrame_;
  std::mutex frameMutex_;
  std::condition_variable frameCV_;
  uint64_t frameSeq_{0};
  std::atomic<bool> mjpegStreaming_{false};
  std::atomic<int> streamClients_{0};

  // Shared Memory for IPC (Electron)
  std::unique_ptr<SharedMemoryManager> sharedMemory_;

  void detectCanonCameras();
  void detectWebcams();
};

} // namespace photobooth
