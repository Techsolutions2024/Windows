#include "camera/WebcamCamera.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

namespace photobooth {

WebcamCamera::WebcamCamera(int deviceIndex, const std::string &name)
    : deviceIndex_(deviceIndex), name_(name), connected_(false),
      liveViewActive_(false) {
  settings_.mirror = true; // Default mirror for webcam (selfie mode)
}

WebcamCamera::~WebcamCamera() { disconnect(); }

bool WebcamCamera::connect() {
  if (connected_) {
    return true;
  }

#ifdef USE_OPENCV
  std::lock_guard<std::mutex> lock(captureMutex_);
  capture_.open(deviceIndex_, cv::CAP_DSHOW); // Use DirectShow on Windows

  if (!capture_.isOpened()) {
    // Try default backend
    capture_.open(deviceIndex_);
  }

  if (capture_.isOpened()) {
    // Set resolution
    capture_.set(cv::CAP_PROP_FRAME_WIDTH, frameWidth_);
    capture_.set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight_);
    capture_.set(cv::CAP_PROP_FPS, 30);

    // Get actual resolution (camera may not support requested)
    frameWidth_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    frameHeight_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));

    connected_ = true;
    std::cout << "Webcam connected: " << name_ << " (" << frameWidth_ << "x"
              << frameHeight_ << ")" << std::endl;
    return true;
  }

  std::cerr << "Failed to open webcam at index " << deviceIndex_ << std::endl;
  return false;
#else
  // Fallback without OpenCV - simulate connection
  connected_ = true;
  std::cout << "Webcam connected (simulation mode): " << name_ << std::endl;
  return true;
#endif
}

void WebcamCamera::disconnect() {
  stopLiveView();

#ifdef USE_OPENCV
  std::lock_guard<std::mutex> lock(captureMutex_);
  if (capture_.isOpened()) {
    capture_.release();
  }
#endif

  connected_ = false;
  std::cout << "Webcam disconnected: " << name_ << std::endl;
}

bool WebcamCamera::isConnected() const { return connected_; }

bool WebcamCamera::startLiveView(LiveViewCallback callback) {
  if (!connected_) {
    std::cerr << "Cannot start live view: webcam not connected" << std::endl;
    return false;
  }

  if (liveViewActive_) {
    return true;
  }

  liveViewCallback_ = callback;
  liveViewActive_ = true;
  liveViewThread_ = std::thread(&WebcamCamera::liveViewLoop, this);

  std::cout << "Webcam live view started" << std::endl;
  return true;
}

void WebcamCamera::stopLiveView() {
  liveViewActive_ = false;
  if (liveViewThread_.joinable()) {
    liveViewThread_.join();
  }
  liveViewCallback_ = nullptr;
  std::cout << "Webcam live view stopped" << std::endl;
}

bool WebcamCamera::isLiveViewActive() const { return liveViewActive_; }

void WebcamCamera::liveViewLoop() {
  while (liveViewActive_) {
    auto frameStart = std::chrono::steady_clock::now();

#ifdef USE_OPENCV
    cv::Mat frame;
    {
      std::lock_guard<std::mutex> lock(captureMutex_);
      if (!capture_.isOpened() || !capture_.read(frame)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        continue;
      }
    }

    if (frame.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
      continue;
    }

    // Apply mirror if enabled
    if (settings_.mirror) {
      cv::flip(frame, frame, 1);
    }

    // Apply rotation
    if (settings_.rotation != 0) {
      int rotateCode = -1;
      switch (settings_.rotation) {
      case 90:
        rotateCode = cv::ROTATE_90_CLOCKWISE;
        break;
      case 180:
        rotateCode = cv::ROTATE_180;
        break;
      case 270:
        rotateCode = cv::ROTATE_90_COUNTERCLOCKWISE;
        break;
      }
      if (rotateCode >= 0) {
        cv::rotate(frame, frame, rotateCode);
      }
    }

    // Encode to JPEG
    std::vector<uint8_t> jpegData;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
    cv::imencode(".jpg", frame, jpegData, params);

    if (liveViewCallback_ && !jpegData.empty()) {
      liveViewCallback_(jpegData, frame.cols, frame.rows);
    }
#else
    // Simulation mode - generate gradient test pattern
    int width = frameWidth_;
    int height = frameHeight_;
    std::vector<uint8_t> testFrame(width * height * 3);

    static int frameCounter = 0;
    frameCounter++;

    // Generate a moving gradient pattern
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int idx = (y * width + x) * 3;
        testFrame[idx] = static_cast<uint8_t>((x + frameCounter) % 256);     // R
        testFrame[idx + 1] = static_cast<uint8_t>((y + frameCounter) % 256); // G
        testFrame[idx + 2] = 128;                                             // B
      }
    }

    if (liveViewCallback_) {
      liveViewCallback_(testFrame, width, height);
    }
#endif

    // Maintain ~30 FPS
    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        frameEnd - frameStart);
    if (elapsed.count() < 33) {
      std::this_thread::sleep_for(std::chrono::milliseconds(33 - elapsed.count()));
    }
  }
}

void WebcamCamera::capture(CaptureMode mode, CaptureCallback callback) {
  if (!connected_) {
    if (callback) {
      callback({false, "", {}, 0, 0, "Webcam not connected"});
    }
    return;
  }

  // Capture in separate thread
  std::thread([this, mode, callback]() {
#ifdef USE_OPENCV
    cv::Mat frame;
    {
      std::lock_guard<std::mutex> lock(captureMutex_);
      if (!capture_.isOpened() || !capture_.read(frame)) {
        if (callback) {
          callback({false, "", {}, 0, 0, "Failed to capture frame"});
        }
        return;
      }
    }

    if (frame.empty()) {
      if (callback) {
        callback({false, "", {}, 0, 0, "Empty frame captured"});
      }
      return;
    }

    // Apply mirror if enabled
    if (settings_.mirror) {
      cv::flip(frame, frame, 1);
    }

    // Apply rotation
    if (settings_.rotation != 0) {
      int rotateCode = -1;
      switch (settings_.rotation) {
      case 90:
        rotateCode = cv::ROTATE_90_CLOCKWISE;
        break;
      case 180:
        rotateCode = cv::ROTATE_180;
        break;
      case 270:
        rotateCode = cv::ROTATE_90_COUNTERCLOCKWISE;
        break;
      }
      if (rotateCode >= 0) {
        cv::rotate(frame, frame, rotateCode);
      }
    }

    // Generate filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();
    std::string filename =
        "data/captures/webcam_" + std::to_string(timestamp) + ".jpg";

    // Save to disk
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    if (cv::imwrite(filename, frame, params)) {
      // Also encode to memory for immediate use
      std::vector<uint8_t> jpegData;
      cv::imencode(".jpg", frame, jpegData, params);

      if (callback) {
        callback({true, filename, jpegData, frame.cols, frame.rows, ""});
      }
    } else {
      if (callback) {
        callback({false, "", {}, 0, 0, "Failed to save image"});
      }
    }
#else
    // Simulation mode
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (callback) {
      callback({true, "simulated_capture.jpg", {}, frameWidth_, frameHeight_, ""});
    }
#endif
  }).detach();
}

void WebcamCamera::captureWithCountdown(int seconds, CaptureMode mode,
                                        CaptureCallback callback) {
  std::thread([this, seconds, mode, callback]() {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    this->capture(mode, callback);
  }).detach();
}

bool WebcamCamera::setSettings(const CameraSettings &settings) {
  settings_ = settings;

#ifdef USE_OPENCV
  std::lock_guard<std::mutex> lock(captureMutex_);
  if (capture_.isOpened()) {
    // Webcams typically don't support ISO/aperture/shutter
    // But we can adjust brightness, contrast, etc. if needed
    // capture_.set(cv::CAP_PROP_BRIGHTNESS, ...);
  }
#endif

  return true;
}

CameraSettings WebcamCamera::getSettings() const { return settings_; }

std::vector<int> WebcamCamera::getSupportedISO() const {
  // Webcams don't have ISO control
  return {};
}

std::vector<std::string> WebcamCamera::getSupportedApertures() const {
  // Webcams have fixed aperture
  return {};
}

std::vector<std::string> WebcamCamera::getSupportedShutterSpeeds() const {
  // Webcams have auto exposure
  return {};
}

std::vector<std::string> WebcamCamera::getSupportedWhiteBalances() const {
  return {"Auto"};
}

void WebcamCamera::setResolution(int width, int height) {
  frameWidth_ = width;
  frameHeight_ = height;

#ifdef USE_OPENCV
  std::lock_guard<std::mutex> lock(captureMutex_);
  if (capture_.isOpened()) {
    capture_.set(cv::CAP_PROP_FRAME_WIDTH, width);
    capture_.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    frameWidth_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    frameHeight_ = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
  }
#endif
}

std::vector<std::pair<int, std::string>> WebcamCamera::listAvailableWebcams() {
  std::vector<std::pair<int, std::string>> webcams;

#ifdef USE_OPENCV
  // Try to open cameras at indices 0-9
  for (int i = 0; i < 10; i++) {
    cv::VideoCapture cap(i, cv::CAP_DSHOW);
    if (cap.isOpened()) {
      std::string name = "Webcam " + std::to_string(i);
      webcams.push_back({i, name});
      cap.release();
    }
  }
#else
  // Return a dummy webcam in simulation mode
  webcams.push_back({0, "Simulated Webcam"});
#endif

  return webcams;
}

std::vector<uint8_t>
WebcamCamera::encodeFrameToJpeg(const std::vector<uint8_t> &rgbData, int width,
                                 int height) {
#ifdef USE_OPENCV
  cv::Mat frame(height, width, CV_8UC3, const_cast<uint8_t *>(rgbData.data()));
  std::vector<uint8_t> jpegData;
  std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
  cv::imencode(".jpg", frame, jpegData, params);
  return jpegData;
#else
  return rgbData;
#endif
}

void WebcamCamera::applyMirrorAndRotation(std::vector<uint8_t> &imageData,
                                          int &width, int &height) {
  // Handled in liveViewLoop and capture methods using OpenCV
}

} // namespace photobooth
