#include "core/Application.h"
#include "server/LiveViewServer.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace photobooth {

Application::Application() : running_(false), configPath_("config.json") {}

Application::~Application() { shutdown(); }

bool Application::initialize() {
  std::cout << "Initializing Camera Manager..." << std::endl;
  cameraManager_ = std::make_unique<CameraManager>();
  if (!cameraManager_->initialize()) {
    std::cerr << "Failed to initialize Camera Manager!" << std::endl;
    return false;
  }

  // Detect cameras
  auto cameras = cameraManager_->detectCameras();
  std::cout << "Found " << cameras.size() << " camera(s):" << std::endl;
  for (const auto &cam : cameras) {
    std::cout << "  - " << cam << std::endl;
  }

  // Select first available camera
  if (!cameras.empty()) {
    cameraManager_->selectCamera(cameras[0]);
    std::cout << "Selected camera: " << cameras[0] << std::endl;
  }

  std::cout << "Initializing Burst Capture Manager..." << std::endl;
  if (cameraManager_ && cameraManager_->getCurrentCamera()) {
    burstCaptureManager_ = std::make_unique<BurstCaptureManager>(
        cameraManager_->getCurrentCamera());
  } else {
    std::cout << "Warning: No camera selected for Burst Capture Manager"
              << std::endl;
    // We can initialize it later when camera is selected if needed,
    // but for now let's just log it.
    // Ideally BurstCaptureManager should take CameraManager and get current
    // camera dynamically. But given the constructor takes ICamera*, we'll
    // initialize it if camera exists.
  }

  std::cout << "Initializing Database Manager..." << std::endl;
  dbManager_ = std::make_unique<DatabaseManager>();
  if (!dbManager_->initialize()) {
    std::cerr << "Failed to initialize Database Manager!" << std::endl;
    return false;
  }

  std::cout << "Initializing File Manager..." << std::endl;
  fileManager_ = std::make_unique<FileManager>();
  if (!fileManager_->initialize()) {
    std::cerr << "Failed to initialize File Manager!" << std::endl;
    return false;
  }

  std::cout << "Starting HTTP Server..." << std::endl;
  httpServer_ = std::make_unique<HTTPServer>(this, 8080);
  if (!httpServer_->start()) {
    std::cerr << "Failed to start HTTP Server!" << std::endl;
    return false;
  }

  std::cout << "Starting WebSocket Server..." << std::endl;
  wsServer_ = std::make_unique<WebSocketServer>(
      this, 8082); // Changed to 8082 to avoid conflict
  if (!wsServer_->start()) {
    std::cerr << "Failed to start WebSocket Server!" << std::endl;
    return false;
  }

  std::cout << "Starting LiveView Server (uWebSockets)..." << std::endl;
  LiveViewServer::getInstance().start(
      8081); // High-performance binary stream on 8081

  running_ = true;
  return true;
}

void Application::run() {
  while (running_) {
    // Main event loop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Process EDSDK events if Canon camera is active
    // EdsGetEvent(); // Uncomment when EDSDK is fully integrated
  }
}

void Application::shutdown() {
  if (!running_)
    return;

  std::cout << "Shutting down application..." << std::endl;
  running_ = false;

  if (wsServer_) {
    wsServer_->stop();
  }

  if (httpServer_) {
    httpServer_->stop();
  }

  std::cout << "Stopping LiveView Server..." << std::endl;
  LiveViewServer::getInstance().stop();

  if (cameraManager_) {
    cameraManager_->shutdown();
  }

  if (dbManager_) {
    dbManager_->close();
  }
}

} // namespace photobooth
