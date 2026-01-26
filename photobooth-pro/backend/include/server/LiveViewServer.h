#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>


// Forward declarations to avoid including uWS headers heavily in the header
namespace uWS {
template <bool SSL> struct TemplatedApp;
}

namespace photobooth {

class LiveViewServer {
public:
  static LiveViewServer &getInstance();

  void start(int port = 8081);
  void stop();

  // Send a new frame to the client (if ready)
  // The vector is moved to avoid copies if possible
  void updateFrame(std::vector<unsigned char> frameData);

private:
  LiveViewServer();
  ~LiveViewServer();

  LiveViewServer(const LiveViewServer &) = delete;
  LiveViewServer &operator=(const LiveViewServer &) = delete;

  void serverLoop(int port);

  uWS::TemplatedApp<false> *app_{nullptr};
  void *loop_{nullptr}; // uWS::Loop*

  std::thread serverThread_;
  std::atomic<bool> running_{false};

  // Frame data sync
  std::mutex frameMutex_;
  std::vector<unsigned char> currentFrame_;
  bool hasNewFrame_{false};

  // Flow control
  // We assume single active client for the photobooth live view
  // If multiple connect, this logic means "at least one is ready" or "global
  // ready" Ideally we'd track per socket, but publish is broadcast. Given the
  // requirement, we'll use a simple latch.
  bool clientReady_{false}; // Accessed only on server thread
};

} // namespace photobooth
