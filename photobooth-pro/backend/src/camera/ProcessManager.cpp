#include "camera/ProcessManager.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>


#ifdef _WIN32
#include <windows.h>
#endif

namespace photobooth {

ProcessManager::ProcessManager(const std::string &executablePath)
    : executablePath_(executablePath) {
#ifdef _WIN32
  ZeroMemory(&processInfo_, sizeof(PROCESS_INFORMATION));
#endif
}

ProcessManager::~ProcessManager() { shutdown(); }

bool ProcessManager::initialize() {
  std::lock_guard<std::mutex> lock(processMutex_);

  if (processRunning_) {
    std::cout << "ProcessManager already running" << std::endl;
    return true;
  }

  std::cout << "Initializing ProcessManager with: " << executablePath_
            << std::endl;

  if (!createProcess()) {
    std::cerr << "Failed to create process" << std::endl;
    return false;
  }

  processRunning_ = true;

  // Wait for initial prompt (camera selection menu)
  if (!waitForPrompt("Select camera", 10000)) {
    std::cerr << "Did not receive camera selection prompt" << std::endl;
    shutdown();
    return false;
  }

  std::cout << "ProcessManager initialized successfully" << std::endl;
  return true;
}

void ProcessManager::shutdown() {
  std::lock_guard<std::mutex> lock(processMutex_);

  if (!processRunning_) {
    return;
  }

  std::cout << "Shutting down ProcessManager" << std::endl;

  // Send 'x' to quit
  writeToStdin("x\n");
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  cleanupProcess();
  processRunning_ = false;
}

#ifdef _WIN32
bool ProcessManager::createProcess() {
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  // Create pipes for stdin
  if (!CreatePipe(&hChildStdInRd_, &hChildStdInWr_, &saAttr, 0)) {
    std::cerr << "CreatePipe (stdin) failed" << std::endl;
    return false;
  }
  if (!SetHandleInformation(hChildStdInWr_, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "SetHandleInformation (stdin) failed" << std::endl;
    return false;
  }

  // Create pipes for stdout
  if (!CreatePipe(&hChildStdOutRd_, &hChildStdOutWr_, &saAttr, 0)) {
    std::cerr << "CreatePipe (stdout) failed" << std::endl;
    return false;
  }
  if (!SetHandleInformation(hChildStdOutRd_, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "SetHandleInformation (stdout) failed" << std::endl;
    return false;
  }

  // Setup STARTUPINFO
  STARTUPINFOA siStartInfo;
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = hChildStdOutWr_;
  siStartInfo.hStdOutput = hChildStdOutWr_;
  siStartInfo.hStdInput = hChildStdInRd_;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  // Create the child process
  BOOL success = CreateProcessA(executablePath_.c_str(), NULL, NULL, NULL,
                                TRUE,             // Inherit handles
                                CREATE_NO_WINDOW, // Don't show console window
                                NULL, NULL, &siStartInfo, &processInfo_);

  if (!success) {
    std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
    return false;
  }

  // Close the write end of stdout pipe (child owns it)
  CloseHandle(hChildStdOutWr_);
  hChildStdOutWr_ = NULL;

  // Close the read end of stdin pipe (child owns it)
  CloseHandle(hChildStdInRd_);
  hChildStdInRd_ = NULL;

  std::cout << "Process created successfully" << std::endl;
  return true;
}

void ProcessManager::cleanupProcess() {
  if (processInfo_.hProcess != NULL) {
    TerminateProcess(processInfo_.hProcess, 0);
    WaitForSingleObject(processInfo_.hProcess, 2000);
    CloseHandle(processInfo_.hProcess);
    CloseHandle(processInfo_.hThread);
    ZeroMemory(&processInfo_, sizeof(PROCESS_INFORMATION));
  }

  if (hChildStdInWr_)
    CloseHandle(hChildStdInWr_);
  if (hChildStdOutRd_)
    CloseHandle(hChildStdOutRd_);

  hChildStdInWr_ = NULL;
  hChildStdOutRd_ = NULL;
}

bool ProcessManager::writeToStdin(const std::string &data) {
  if (!hChildStdInWr_) {
    return false;
  }

  DWORD written;
  BOOL success = WriteFile(hChildStdInWr_, data.c_str(),
                           static_cast<DWORD>(data.length()), &written, NULL);

  if (!success || written != data.length()) {
    std::cerr << "WriteFile to stdin failed" << std::endl;
    return false;
  }

  // Flush to ensure data is sent
  FlushFileBuffers(hChildStdInWr_);
  return true;
}

std::string ProcessManager::readFromStdout(size_t maxBytes, int timeoutMs) {
  if (!hChildStdOutRd_) {
    return "";
  }

  std::string result;
  char buffer[1024];
  DWORD bytesRead;
  DWORD totalBytesAvail;

  auto startTime = std::chrono::steady_clock::now();

  while (true) {
    // Check timeout
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - startTime)
                       .count();

    if (elapsed >= timeoutMs) {
      break;
    }

    // Check if data is available
    BOOL success =
        PeekNamedPipe(hChildStdOutRd_, NULL, 0, NULL, &totalBytesAvail, NULL);
    if (!success || totalBytesAvail == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      continue;
    }

    // Read available data
    DWORD toRead = min(totalBytesAvail, sizeof(buffer) - 1);
    success = ReadFile(hChildStdOutRd_, buffer, toRead, &bytesRead, NULL);

    if (!success || bytesRead == 0) {
      break;
    }

    buffer[bytesRead] = '\0';
    result.append(buffer, bytesRead);

    if (result.length() >= maxBytes) {
      break;
    }
  }

  return result;
}
#endif

bool ProcessManager::waitForPrompt(const std::string &prompt, int timeoutMs) {
  auto startTime = std::chrono::steady_clock::now();
  std::string accumulated;

  while (true) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - startTime)
                       .count();

    if (elapsed >= timeoutMs) {
      std::cerr << "Timeout waiting for prompt: " << prompt << std::endl;
      return false;
    }

    std::string output = readFromStdout(4096, 500);
    if (!output.empty()) {
      accumulated += output;
      std::cout << output; // Echo output for debugging

      if (accumulated.find(prompt) != std::string::npos) {
        return true;
      }
    }
  }

  return false;
}

std::string ProcessManager::sendCommand(const std::string &command) {
  if (!processRunning_) {
    return "";
  }

  std::cout << "[CMD] Sending: " << command << std::endl;

  if (!writeToStdin(command + "\n")) {
    std::cerr << "Failed to write command" << std::endl;
    return "";
  }

  // Wait a bit for response
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  std::string response = readFromStdout(8192, 2000);
  std::cout << "[RESP] " << response << std::endl;

  return response;
}

// ==================== Camera Detection & Selection ====================

std::vector<std::string> ProcessManager::detectCameras() {
  std::vector<std::string> cameras;

  if (!processRunning_) {
    std::cerr << "Process not running" << std::endl;
    return cameras;
  }

  // The initial output should contain camera list
  // Example: "Canon EOS 5D Mark IV"
  // For now, return a placeholder
  // In real implementation, parse the stdout buffer

  cameras.push_back("Canon Camera 1");
  return cameras;
}

bool ProcessManager::selectCamera(int cameraIndex) {
  if (!processRunning_) {
    return false;
  }

  std::cout << "Selecting camera index: " << cameraIndex << std::endl;

  std::string response = sendCommand(std::to_string(cameraIndex));

  // Wait for control menu prompt
  if (!waitForPrompt("Set Save To", 5000)) {
    std::cerr << "Failed to get control menu" << std::endl;
    return false;
  }

  std::cout << "Camera selected successfully" << std::endl;
  return true;
}

bool ProcessManager::selectAllCameras() {
  return selectCamera('a'); // Send 'a' for all cameras
}

bool ProcessManager::selectCameraRange(int startIndex, int endIndex) {
  std::string rangeCmd =
      std::to_string(startIndex) + "-" + std::to_string(endIndex);
  std::string response = sendCommand(rangeCmd);
  return waitForPrompt("Set Save To", 5000);
}

// ==================== Live View Commands ====================

bool ProcessManager::startLiveView() {
  std::string response = sendCommand("21");
  // Check response for success
  return response.find("error") == std::string::npos;
}

bool ProcessManager::stopLiveView() {
  std::string response = sendCommand("22");
  return response.find("error") == std::string::npos;
}

std::vector<uint8_t> ProcessManager::getLiveViewFrame() {
  std::vector<uint8_t> frame;

  // Send command 20 to get one live view image
  std::string response = sendCommand("20");

  // TODO: Parse binary JPEG data from response
  // For now, try to read image file saved by MultiCamCui
  // MultiCamCui saves to cam{N}/LiveView.jpg

  std::string imagePath = "cam1/LiveView.jpg";
  std::ifstream file(imagePath, std::ios::binary);
  if (file.is_open()) {
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    frame.resize(fileSize);
    file.read(reinterpret_cast<char *>(frame.data()), fileSize);
    file.close();
  }

  return frame;
}

// ==================== Capture Commands ====================

bool ProcessManager::takePicture() {
  std::cout << "Taking picture..." << std::endl;
  std::string response = sendCommand("3");

  // MultiCamCui will save to cam{N}/IMG_XXXX.jpg
  // Wait for operation to complete
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return response.find("error") == std::string::npos;
}

bool ProcessManager::pressHalfway() {
  std::string response = sendCommand("4");
  return response.find("error") == std::string::npos;
}

bool ProcessManager::pressCompletely() {
  std::string response = sendCommand("5");
  return response.find("error") == std::string::npos;
}

bool ProcessManager::pressOff() {
  std::string response = sendCommand("6");
  return response.find("error") == std::string::npos;
}

// ==================== Settings Commands ====================

bool ProcessManager::setSaveTo(int option) {
  sendCommand("1"); // Enter Set Save To menu
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::string response = sendCommand(std::to_string(option));
  return response.find("error") == std::string::npos;
}

bool ProcessManager::setImageQuality(int quality) {
  sendCommand("2"); // Enter Image Quality menu
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::string response = sendCommand(std::to_string(quality));
  return response.find("error") == std::string::npos;
}

bool ProcessManager::setTV(int value) {
  sendCommand("7");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::string response = sendCommand(std::to_string(value));
  return response.find("error") == std::string::npos;
}

bool ProcessManager::setAV(int value) {
  sendCommand("8");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::string response = sendCommand(std::to_string(value));
  return response.find("error") == std::string::npos;
}

bool ProcessManager::setISO(int value) {
  sendCommand("9");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::string response = sendCommand(std::to_string(value));
  return response.find("error") == std::string::npos;
}

std::string ProcessManager::executeCommand(const std::string &command) {
  return sendCommand(command);
}

} // namespace photobooth
