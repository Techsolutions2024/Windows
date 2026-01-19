#pragma once

#include <memory>
#include <string>
#include "camera/CameraManager.h"
#include "api/HTTPServer.h"
#include "api/WebSocketServer.h"
#include "storage/DatabaseManager.h"

namespace photobooth {

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

    // Getters
    CameraManager* getCameraManager() { return cameraManager_.get(); }
    HTTPServer* getHTTPServer() { return httpServer_.get(); }
    WebSocketServer* getWebSocketServer() { return wsServer_.get(); }
    DatabaseManager* getDatabaseManager() { return dbManager_.get(); }
    DatabaseManager& getDatabase() { return *dbManager_; }

private:
    std::unique_ptr<CameraManager> cameraManager_;
    std::unique_ptr<HTTPServer> httpServer_;
    std::unique_ptr<WebSocketServer> wsServer_;
    std::unique_ptr<DatabaseManager> dbManager_;
    
    bool running_;
    std::string configPath_;
};

} // namespace photobooth
