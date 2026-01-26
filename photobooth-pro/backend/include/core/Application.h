#pragma once

#include "api/HTTPServer.h"
#include "api/WebSocketServer.h"
#include "camera/CameraManager.h"
#include "media/BurstCaptureManager.h"
#include "storage/DatabaseManager.h"
#include "storage/FileManager.h"
#include <memory>
#include <string>

namespace photobooth {

class Application {
public:
  Application();
  ~Application();

  bool initialize();
  void run();
  void shutdown();

  // Getters
  CameraManager *getCameraManager() { return cameraManager_.get(); }
  BurstCaptureManager *getBurstCaptureManager() {
    return burstCaptureManager_.get();
  }
  HTTPServer *getHTTPServer() { return httpServer_.get(); }
  WebSocketServer *getWebSocketServer() { return wsServer_.get(); }
  DatabaseManager *getDatabaseManager() { return dbManager_.get(); }
  DatabaseManager &getDatabase() { return *dbManager_; }
  FileManager *getFileManager() { return fileManager_.get(); }

private:
  std::unique_ptr<CameraManager> cameraManager_;
  std::unique_ptr<BurstCaptureManager> burstCaptureManager_;
  std::unique_ptr<HTTPServer> httpServer_;
  std::unique_ptr<WebSocketServer> wsServer_;
  std::unique_ptr<DatabaseManager> dbManager_;
  std::unique_ptr<FileManager> fileManager_;

  bool running_;
  std::string configPath_;
};

} // namespace photobooth
