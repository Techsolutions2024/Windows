#include "media/BurstCaptureManager.h"
#include "media/GifCreator.h"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>


namespace fs = std::filesystem;

namespace photobooth {

BurstCaptureManager::BurstCaptureManager(ICamera *camera) : camera_(camera) {
  if (!camera_) {
    throw std::invalid_argument("Camera cannot be null");
  }
}

BurstCaptureManager::~BurstCaptureManager() { stopBurst(); }

void BurstCaptureManager::startBurst(const BurstOptions &options,
                                     ProgressCallback progressCb,
                                     CompletionCallback completionCb) {
  if (capturing_) {
    std::cerr << "BurstCapture: Already capturing" << std::endl;
    return;
  }

  if (!camera_->isConnected()) {
    BurstResult result;
    result.success = false;
    result.errorMessage = "Camera not connected";
    if (completionCb)
      completionCb(result);
    return;
  }

  currentOptions_ = options;
  progressCallback_ = progressCb;
  completionCallback_ = completionCb;
  shouldStop_ = false;
  capturing_ = true;

  // Start capture thread
  captureThread_ = std::thread(&BurstCaptureManager::burstCaptureLoop, this);
  captureThread_.detach();
}

void BurstCaptureManager::stopBurst() {
  if (!capturing_)
    return;

  shouldStop_ = true;

  // Wait for thread to finish (with timeout)
  auto start = std::chrono::steady_clock::now();
  while (capturing_ && std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - start)
                               .count() < 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  capturing_ = false;
}

void BurstCaptureManager::burstCaptureLoop() {
  BurstResult result;
  result.success = false;

  try {
    // Create save directory
    fs::create_directories(currentOptions_.saveDirectory);

    // Generate timestamp for this burst session
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();

    std::string sessionDir =
        currentOptions_.saveDirectory + "/burst_" + std::to_string(timestamp);
    fs::create_directories(sessionDir);

    std::cout << "BurstCapture: Starting burst capture - "
              << currentOptions_.frameCount << " frames at "
              << currentOptions_.frameInterval << "ms interval" << std::endl;

    // Capture frames
    for (int i = 0; i < currentOptions_.frameCount && !shouldStop_; i++) {
      // Generate frame filename
      std::ostringstream oss;
      oss << sessionDir << "/frame_" << std::setfill('0') << std::setw(4) << i
          << ".jpg";
      std::string framePath = oss.str();

      // Capture frame
      std::cout << "BurstCapture: Capturing frame " << (i + 1) << "/"
                << currentOptions_.frameCount << std::endl;

      if (captureFrame(framePath)) {
        result.framePaths.push_back(framePath);
        result.capturedFrames++;

        // Progress callback
        if (progressCallback_) {
          progressCallback_(i + 1, currentOptions_.frameCount);
        }
      } else {
        std::cerr << "BurstCapture: Failed to capture frame " << (i + 1)
                  << std::endl;
        // Continue trying next frames instead of failing completely
      }

      // Wait interval before next frame (except last frame)
      if (i < currentOptions_.frameCount - 1 && !shouldStop_) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(currentOptions_.frameInterval));
      }
    }

    // Check if we got at least some frames
    if (result.capturedFrames > 0) {
      result.success = true;
      std::cout << "BurstCapture: Successfully captured "
                << result.capturedFrames << " frames" << std::endl;
    } else {
      result.errorMessage = "Failed to capture any frames";
      std::cerr << "BurstCapture: " << result.errorMessage << std::endl;
    }

  } catch (const std::exception &e) {
    result.success = false;
    result.errorMessage =
        std::string("Exception during burst capture: ") + e.what();
    std::cerr << "BurstCapture: " << result.errorMessage << std::endl;
  }

  capturing_ = false;

  // Completion callback
  if (completionCallback_) {
    completionCallback_(result);
  }
}

bool BurstCaptureManager::captureFrame(const std::string &outputPath) {
  if (!camera_ || !camera_->isConnected()) {
    return false;
  }

  bool captureSuccess = false;
  std::string capturedPath;

  // Set up capture callback
  auto captureCallback = [&](const CaptureResult &result) {
    if (result.success && !result.filePath.empty()) {
      capturedPath = result.filePath;
      captureSuccess = true;
    }
  };

  // Trigger capture
  camera_->capture(CaptureMode::Photo, captureCallback);

  // Wait for capture to complete (with timeout)
  auto start = std::chrono::steady_clock::now();
  const int timeoutSeconds = 10;

  while (!captureSuccess && std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::steady_clock::now() - start)
                                    .count() < timeoutSeconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (!captureSuccess) {
    std::cerr << "BurstCapture: Capture timeout" << std::endl;
    return false;
  }

  // Move/copy file to desired location if different
  if (capturedPath != outputPath) {
    try {
      fs::copy_file(capturedPath, outputPath,
                    fs::copy_options::overwrite_existing);
      // Optionally delete original
      // fs::remove(capturedPath);
    } catch (const std::exception &e) {
      std::cerr << "BurstCapture: Failed to copy file: " << e.what()
                << std::endl;
      return false;
    }
  }

  return true;
}

std::string BurstCaptureManager::createGifFromBurst(
    const BurstResult &burstResult, const GifCreator::GifOptions &gifOptions) {
  if (!burstResult.success || burstResult.framePaths.empty()) {
    std::cerr << "BurstCapture: Cannot create GIF from failed burst"
              << std::endl;
    return "";
  }

  // Generate output path
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch())
                       .count();

  std::string outputPath = currentOptions_.saveDirectory + "/gif_" +
                           std::to_string(timestamp) + ".gif";

  // Create GIF
  GifCreator creator;
  std::string gifPath =
      creator.createGif(burstResult.framePaths, outputPath, gifOptions);

  if (gifPath.empty()) {
    std::cerr << "BurstCapture: Failed to create GIF" << std::endl;
    return "";
  }

  std::cout << "BurstCapture: GIF created successfully: " << gifPath
            << std::endl;
  return gifPath;
}

std::string BurstCaptureManager::createBoomerangFromBurst(
    const BurstResult &burstResult, const GifCreator::GifOptions &gifOptions) {
  if (!burstResult.success || burstResult.framePaths.empty()) {
    std::cerr << "BurstCapture: Cannot create Boomerang from failed burst"
              << std::endl;
    return "";
  }

  // Create boomerang sequence
  std::vector<std::string> boomerangSequence =
      createBoomerangSequence(burstResult.framePaths,
                              true // smooth reverse
      );

  // Generate output path
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch())
                       .count();

  std::string outputPath = currentOptions_.saveDirectory + "/boomerang_" +
                           std::to_string(timestamp) + ".gif";

  // Create GIF with boomerang sequence
  GifCreator creator;

  // Use faster frame delay for boomerang effect
  GifCreator::GifOptions boomerangOptions = gifOptions;
  boomerangOptions.frameDelay =
      std::min(boomerangOptions.frameDelay, 8); // Max 80ms

  std::string gifPath =
      creator.createGif(boomerangSequence, outputPath, boomerangOptions);

  if (gifPath.empty()) {
    std::cerr << "BurstCapture: Failed to create Boomerang" << std::endl;
    return "";
  }

  std::cout << "BurstCapture: Boomerang created successfully: " << gifPath
            << std::endl;
  return gifPath;
}

std::vector<std::string> BurstCaptureManager::createBoomerangSequence(
    const std::vector<std::string> &frames, bool smoothReverse) {
  std::vector<std::string> sequence;

  if (frames.empty()) {
    return sequence;
  }

  // Forward sequence
  for (const auto &frame : frames) {
    sequence.push_back(frame);
  }

  // Reverse sequence
  if (smoothReverse && frames.size() > 2) {
    // Skip first and last frame to avoid stutter
    for (int i = frames.size() - 2; i > 0; i--) {
      sequence.push_back(frames[i]);
    }
  } else {
    // Include all frames in reverse
    for (int i = frames.size() - 1; i >= 0; i--) {
      sequence.push_back(frames[i]);
    }
  }

  std::cout << "BurstCapture: Created boomerang sequence with "
            << sequence.size() << " frames (from " << frames.size()
            << " original frames)" << std::endl;

  return sequence;
}

} // namespace photobooth
