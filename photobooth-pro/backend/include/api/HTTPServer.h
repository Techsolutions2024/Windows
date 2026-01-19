#pragma once

#include <string>
#include <functional>
#include <memory>

namespace photobooth {

class Application;

class HTTPServer {
public:
    HTTPServer(Application* app, int port = 8080);
    ~HTTPServer();

    bool start();
    void stop();
    bool isRunning() const;

    int getPort() const { return port_; }

private:
    Application* app_;
    int port_;
    bool running_;
    std::unique_ptr<std::thread> serverThread_;

    void setupRoutes();
    void run();

    // Route handlers
    void handleGetCameras(const std::string& req, std::string& res);
    void handleSelectCamera(const std::string& req, std::string& res);
    void handleGetCameraSettings(const std::string& req, std::string& res);
    void handleSetCameraSettings(const std::string& req, std::string& res);
    void handleCapture(const std::string& req, std::string& res);
    void handleGetEvents(const std::string& req, std::string& res);
    void handleGetGallery(const std::string& req, std::string& res);
    void handleGetImage(const std::string& req, std::string& res);
    void handleDeleteImage(const std::string& req, std::string& res);
    void handlePrint(const std::string& req, std::string& res);
    void handleShare(const std::string& req, std::string& res);
};

} // namespace photobooth
