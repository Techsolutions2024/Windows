#include "api/WebSocketServer.h"
#include <iostream>

namespace photobooth {

WebSocketServer::WebSocketServer(Application *app, int port)
    : app_(app), port_(port), running_(false) {}

WebSocketServer::~WebSocketServer() { stop(); }

bool WebSocketServer::start() {
  running_ = true;
  std::cout << "WebSocket Server started (stub) on port " << port_ << std::endl;
  return true;
}

void WebSocketServer::stop() { running_ = false; }

bool WebSocketServer::isRunning() const { return running_; }

void WebSocketServer::broadcastLiveView(const std::vector<uint8_t> &imageData,
                                        int width, int height) {}
void WebSocketServer::broadcastEvent(const std::string &eventType,
                                     const std::string &data) {}
void WebSocketServer::broadcastCountdown(int seconds) {}
void WebSocketServer::broadcastCaptureComplete(const std::string &imagePath) {}

void WebSocketServer::run() {}
void WebSocketServer::handleMessage(const std::string &message) {}
std::string WebSocketServer::encodeBase64(const std::vector<uint8_t> &data) {
  return "";
}

} // namespace photobooth
