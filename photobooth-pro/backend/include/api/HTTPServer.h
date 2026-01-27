#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>

namespace httplib {
    class Server;
    struct Request;
    struct Response;
}

namespace photobooth {

class Application;

class HTTPServer {
public:
    HTTPServer(Application* app, int port = 8080);
    ~HTTPServer();

    bool start();
    void stop();
    bool isRunning() const { return running_; }

    int getPort() const { return port_; }

private:
    Application* app_;
    int port_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> serverThread_;
    std::unique_ptr<httplib::Server> server_;

    void setupRoutes();
    void run();

    // Helper methods
    std::string jsonResponse(bool success, const std::string& message = "", const std::string& data = "");
    std::string jsonError(const std::string& message, int code = 400);
    void setCorsHeaders(httplib::Response& res);

    // ==================== Events API ====================
    void handleGetEvents(const httplib::Request& req, httplib::Response& res);
    void handleCreateEvent(const httplib::Request& req, httplib::Response& res);
    void handleGetEvent(const httplib::Request& req, httplib::Response& res);
    void handleUpdateEvent(const httplib::Request& req, httplib::Response& res);
    void handleDeleteEvent(const httplib::Request& req, httplib::Response& res);
    void handleDuplicateEvent(const httplib::Request& req, httplib::Response& res);
    void handleGetEventConfig(const httplib::Request& req, httplib::Response& res);
    void handleUpdateEventConfig(const httplib::Request& req, httplib::Response& res);
    void handleSearchEvents(const httplib::Request& req, httplib::Response& res);

    // ==================== Camera API ====================
    void handleGetCameras(const httplib::Request& req, httplib::Response& res);
    void handleSelectCamera(const httplib::Request& req, httplib::Response& res);
    void handleGetCameraSettings(const httplib::Request& req, httplib::Response& res);
    void handleSetCameraSettings(const httplib::Request& req, httplib::Response& res);
    void handleStartLiveView(const httplib::Request& req, httplib::Response& res);
    void handleStopLiveView(const httplib::Request& req, httplib::Response& res);

    // ==================== Capture API ====================
    void handleCapture(const httplib::Request& req, httplib::Response& res);
    void handleCaptureGif(const httplib::Request& req, httplib::Response& res);
    void handleCaptureBoomerang(const httplib::Request& req, httplib::Response& res);
    void handleStartVideo(const httplib::Request& req, httplib::Response& res);
    void handleStopVideo(const httplib::Request& req, httplib::Response& res);

    // ==================== Gallery API ====================
    void handleGetGallery(const httplib::Request& req, httplib::Response& res);
    void handleGetPhoto(const httplib::Request& req, httplib::Response& res);
    void handleDeletePhoto(const httplib::Request& req, httplib::Response& res);
    void handleCreateCollage(const httplib::Request& req, httplib::Response& res);

    // ==================== Print API ====================
    void handleGetPrinters(const httplib::Request& req, httplib::Response& res);
    void handlePrint(const httplib::Request& req, httplib::Response& res);
    void handleGetPrintStatus(const httplib::Request& req, httplib::Response& res);
    void handleGetPrintSettings(const httplib::Request& req, httplib::Response& res);
    void handleUpdatePrintSettings(const httplib::Request& req, httplib::Response& res);

    // ==================== Share API ====================
    void handleShareEmail(const httplib::Request& req, httplib::Response& res);
    void handleShareSMS(const httplib::Request& req, httplib::Response& res);
    void handleGenerateQRCode(const httplib::Request& req, httplib::Response& res);

    // ==================== Settings API ====================
    void handleGetSettings(const httplib::Request& req, httplib::Response& res);
    void handleUpdateSettings(const httplib::Request& req, httplib::Response& res);
    void handleGetStorageInfo(const httplib::Request& req, httplib::Response& res);
    void handleGetNetworkStatus(const httplib::Request& req, httplib::Response& res);

    // ==================== Layout API ====================
    void handleAnalyzeLayout(const httplib::Request& req, httplib::Response& res);
    void handleComposePhotos(const httplib::Request& req, httplib::Response& res);
    void handleGetLayoutSlots(const httplib::Request& req, httplib::Response& res);
    void handleSaveLayoutConfig(const httplib::Request& req, httplib::Response& res);

    // ==================== Static Files ====================
    void handleStaticFiles(const httplib::Request& req, httplib::Response& res);
};

} // namespace photobooth
