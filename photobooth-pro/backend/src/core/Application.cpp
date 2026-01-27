#include "core/Application.h"
#include "EDSDK.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace photobooth {

Application::Application() 
    : running_(false)
    , configPath_("config.json") {
}

Application::~Application() {
    shutdown();
}

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
    for (const auto& cam : cameras) {
        std::cout << "  - " << cam << std::endl;
    }

    // Select first available camera
    if (!cameras.empty()) {
        cameraManager_->selectCamera(cameras[0]);
        std::cout << "Selected camera: " << cameras[0] << std::endl;
    }

    std::cout << "Initializing Database Manager..." << std::endl;
    dbManager_ = std::make_unique<DatabaseManager>();
    if (!dbManager_->initialize()) {
        std::cerr << "Failed to initialize Database Manager!" << std::endl;
        return false;
    }

    std::cout << "Starting HTTP Server..." << std::endl;
    httpServer_ = std::make_unique<HTTPServer>(this, 8080);
    if (!httpServer_->start()) {
        std::cerr << "Failed to start HTTP Server!" << std::endl;
        return false;
    }

    std::cout << "Starting WebSocket Server..." << std::endl;
    wsServer_ = std::make_unique<WebSocketServer>(this, 8081);
    if (!wsServer_->start()) {
        std::cerr << "Failed to start WebSocket Server!" << std::endl;
        return false;
    }

    running_ = true;
    return true;
}

void Application::run() {
    while (running_) {
        // Main event loop
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Process EDSDK events
        EdsGetEvent();
    }
}

void Application::shutdown() {
    if (!running_) return;

    std::cout << "Shutting down application..." << std::endl;
    running_ = false;

    if (wsServer_) {
        wsServer_->stop();
    }

    if (httpServer_) {
        httpServer_->stop();
    }

    if (cameraManager_) {
        cameraManager_->shutdown();
    }

    if (dbManager_) {
        dbManager_->close();
    }
}

} // namespace photobooth
