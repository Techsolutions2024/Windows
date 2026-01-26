#pragma once

#include "ICamera.h"
#include "media/GifCreator.h"
#include <vector>
<parameter name = "string">
#include <atomic>
#include <functional>
#include <thread>

    namespace photobooth {

  /**
   * BurstCaptureManager - Quản lý chụp burst cho GIF/Boomerang
   */
  class BurstCaptureManager {
  public:
    struct BurstOptions {
      int frameCount = 10;      // Số frame cần chụp
      int frameInterval = 200;  // Khoảng cách giữa các frame (ms)
      bool useHighSpeed = true; // Dùng high-speed continuous mode
      std::string saveDirectory = "data/burst";
    };

    struct BurstResult {
      bool success = false;
      std::vector<std::string> framePaths;
      std::string errorMessage;
      int capturedFrames = 0;
    };

    using ProgressCallback = std::function<void(int current, int total)>;
    using CompletionCallback = std::function<void(const BurstResult &)>;

    BurstCaptureManager(ICamera *camera);
    ~BurstCaptureManager();

    /**
     * Bắt đầu chụp burst
     */
    void startBurst(const BurstOptions &options,
                    ProgressCallback progressCb = nullptr,
                    CompletionCallback completionCb = nullptr);

    /**
     * Dừng chụp burst
     */
    void stopBurst();

    /**
     * Kiểm tra có đang chụp không
     */
    bool isCapturing() const { return capturing_; }

    /**
     * Tạo GIF từ burst đã chụp
     */
    std::string createGifFromBurst(
        const BurstResult &burstResult,
        const GifCreator::GifOptions &gifOptions = GifCreator::GifOptions());

    /**
     * Tạo Boomerang từ burst đã chụp
     */
    std::string createBoomerangFromBurst(
        const BurstResult &burstResult,
        const GifCreator::GifOptions &gifOptions = GifCreator::GifOptions());

  private:
    ICamera *camera_;
    std::atomic<bool> capturing_{false};
    std::atomic<bool> shouldStop_{false};
    std::thread captureThread_;

    BurstOptions currentOptions_;
    ProgressCallback progressCallback_;
    CompletionCallback completionCallback_;

    /**
     * Thread chụp burst
     */
    void burstCaptureLoop();

    /**
     * Chụp một frame
     */
    bool captureFrame(const std::string &outputPath);

    /**
     * Tạo sequence boomerang từ danh sách ảnh
     */
    std::vector<std::string>
    createBoomerangSequence(const std::vector<std::string> &frames,
                            bool smoothReverse = true);
  };

} // namespace photobooth
