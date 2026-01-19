#pragma once

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>


namespace photobooth {

class Application;

class WebSocketServer {
public:
  WebSocketServer(Application *app, int port = 8081);
  ~WebSocketServer();

  bool start();
  void stop();
  bool isRunning() const;

  // Broadcast methods
  void broadcastLiveView(const std::vector<uint8_t> &imageData, int width,
                         int height);
  void broadcastEvent(const std::string &eventType, const std::string &data);
  void broadcastCountdown(int seconds);
  void broadcastCaptureComplete(const std::string &imagePath);

  int getPort() const { return port_; }

private:
  Application *app_;
  int port_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> serverThread_;

  void run();
  void handleMessage(const std::string &message);
  std::string encodeBase64(const std::vector<uint8_t> &data);
};

} // namespace photobooth
