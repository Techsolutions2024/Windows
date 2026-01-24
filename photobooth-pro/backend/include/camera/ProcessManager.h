#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


#ifdef _WIN32
#include <windows.h>
#endif

namespace photobooth {

/**
 * ProcessManager - Manages MultiCamCui.exe process communication
 * Uses Canon SDK official sample (MultiCamCui) as external process
 * Communicates via stdin/stdout using command protocol
 */
class ProcessManager {
public:
  ProcessManager(const std::string &executablePath);
  ~ProcessManager();

  // Process lifecycle
  bool initialize();
  void shutdown();
  bool isRunning() const { return processRunning_; }

  // Camera detection & selection
  std::vector<std::string> detectCameras();
  bool selectCamera(int cameraIndex); // 1-based index
  bool selectCameraRange(int startIndex, int endIndex);
  bool selectAllCameras();

  // Live View commands
  bool startLiveView();                    // Command 21
  bool stopLiveView();                     // Command 22
  std::vector<uint8_t> getLiveViewFrame(); // Command 20 - Get one frame

  // Capture commands
  bool takePicture();     // Command 3 - Take picture and download
  bool pressHalfway();    // Command 4
  bool pressCompletely(); // Command 5
  bool pressOff();        // Command 6

  // Settings commands
  bool setSaveTo(int option);        // Command 1
  bool setImageQuality(int quality); // Command 2
  bool setTV(int value);             // Command 7
  bool setAV(int value);             // Command 8
  bool setISO(int value);            // Command 9

  // Raw command execution
  std::string executeCommand(const std::string &command);

  // Frame callback for live view streaming
  using FrameCallback = std::function<void(const std::vector<uint8_t> &)>;
  void setFrameCallback(FrameCallback callback) { frameCallback_ = callback; }

private:
  std::string executablePath_;

#ifdef _WIN32
  HANDLE hChildStdInWr_ = NULL;
  HANDLE hChildStdInRd_ = NULL;
  HANDLE hChildStdOutWr_ = NULL;
  HANDLE hChildStdOutRd_ = NULL;
  PROCESS_INFORMATION processInfo_;
#endif

  std::atomic<bool> processRunning_{false};
  std::mutex processMutex_;

  FrameCallback frameCallback_;

  // Process management
  bool createProcess();
  void cleanupProcess();

  // I/O operations
  bool writeToStdin(const std::string &data);
  std::string readFromStdout(size_t maxBytes = 4096, int timeoutMs = 1000);

  // Command helpers
  std::string sendCommand(const std::string &command);
  bool waitForPrompt(const std::string &prompt, int timeoutMs = 5000);
};

} // namespace photobooth
