#pragma once

#include "ICamera.h"
#include <atomic>
#include <mutex>
#include <thread>

#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

namespace photobooth {

class WebcamCamera : public ICamera {
public:
  WebcamCamera(int deviceIndex = 0, const std::string &name = "Webcam");
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
  void captureWithCountdown(int seconds, CaptureMode mode,
                            CaptureCallback callback) override;

  bool setSettings(const CameraSettings &settings) override;
  CameraSettings getSettings() const override;
  std::vector<int> getSupportedISO() const override;
  std::vector<std::string> getSupportedApertures() const override;
  std::vector<std::string> getSupportedShutterSpeeds() const override;
  std::vector<std::string> getSupportedWhiteBalances() const override;

  // Webcam specific
  void setResolution(int width, int height);
  static std::vector<std::pair<int, std::string>> listAvailableWebcams();

private:
  int deviceIndex_;
  std::string name_;
  std::atomic<bool> connected_;
  std::atomic<bool> liveViewActive_;

#ifdef USE_OPENCV
  cv::VideoCapture capture_;
  std::mutex captureMutex_;
#endif

  std::thread liveViewThread_;
  LiveViewCallback liveViewCallback_;
  CameraSettings settings_;

  int frameWidth_ = 1280;
  int frameHeight_ = 720;

  void liveViewLoop();
  std::vector<uint8_t> encodeFrameToJpeg(const std::vector<uint8_t> &rgbData,
                                          int width, int height);
  void applyMirrorAndRotation(std::vector<uint8_t> &imageData, int &width,
                              int &height);
};

} // namespace photobooth
