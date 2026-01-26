#include "server/LiveViewServer.h"
#include "App.h"
#include <iostream>

namespace photobooth {

LiveViewServer &LiveViewServer::getInstance() {
  static LiveViewServer instance;
  return instance;
}

LiveViewServer::LiveViewServer() {}

LiveViewServer::~LiveViewServer() { stop(); }

void LiveViewServer::start(int port) {
  if (running_)
    return;

  running_ = true;
  serverThread_ = std::thread(&LiveViewServer::serverLoop, this, port);
  serverThread_.detach();
}

void LiveViewServer::stop() {
  if (!running_)
    return;

  // We can't cleanly stop uWS loop from outside easily without more infra.
  // Assuming simple lifecycle for now.
  running_ = false;
}

struct PerSocketData {
  // Nothing for now
};

void LiveViewServer::updateFrame(std::vector<unsigned char> frameData) {
  if (!running_ || !loop_)
    return;

  // 1. Update the shared frame buffer (Producer)
  {
    std::lock_guard<std::mutex> lock(frameMutex_);
    currentFrame_ = std::move(frameData); // Zero-copy move into storage
    hasNewFrame_ = true;
  }

  // 2. Schedule a send on the server thread (Consumer)
  struct uWS::Loop *loop = (struct uWS::Loop *)loop_;
  loop->defer([this]() {
    // This lambda runs on the server thread (Event Loop)

    // Flow Control: Only send if client is ready
    if (!clientReady_) {
      // Drop frame to prevent lag/buildup
      return;
    }

    std::vector<unsigned char> dataToSend;
    {
      std::lock_guard<std::mutex> lock(frameMutex_);
      if (!hasNewFrame_)
        return; // Already sent or no new data

      // Copy data to send (needed because publish takes string_view and we want
      // to keep currentFrame for re-sending if needed? Actually, we can move or
      // copy. Copy is safer if we want to support multiple clients
      // theoretically later.
      dataToSend = currentFrame_;
      hasNewFrame_ = false;
    }

    if (this->app_ && !dataToSend.empty()) {
      // Publish binary data to all subscribers of "liveview"
      this->app_->publish(
          "liveview",
          std::string_view((char *)dataToSend.data(), dataToSend.size()),
          uWS::OpCode::BINARY, false);

      // Reset ready flag (Flow Control)
      // Client must send another READY to get the next frame
      clientReady_ = false;
    }
  });
}

void LiveViewServer::serverLoop(int port) {
  // Stack allocated App
  uWS::App app;
  this->app_ = &app;
  this->loop_ = uWS::Loop::get();

  app.ws<PerSocketData>(
         "/*",
         {.compression = uWS::SHARED_COMPRESSOR,
          .maxPayloadLength = 16 * 1024 * 1024,
          .idleTimeout = 16,
          .maxBackpressure = 1 * 1024 * 1024,
          .open =
              [](auto *ws) {
                // Client connected
                ws->subscribe("liveview");
                std::cout << "LiveView Client Connected" << std::endl;
              },
          .message =
              [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                // Simple flow control protocol
                if (message == "READY") {
                  this->clientReady_ = true;

                  // Optional: Trigger immediate send if we have a frame?
                  // For now, let the next updateFrame handle it to ensure
                  // freshness.
                }
              },
          .drain =
              [](auto *ws) {
                // Backpressure handling if needed
              },
          .close =
              [](auto *ws, int code, std::string_view message) {
                std::cout << "LiveView Client Disconnected" << std::endl;
              }

         })
      .listen(port,
              [port](auto *token) {
                if (token) {
                  std::cout << "LiveView Server listening on port " << port
                            << std::endl;
                } else {
                  std::cerr << "LiveView Server failed to verify port " << port
                            << std::endl;
                }
              })
      .run();

  // Loop finished
  this->app_ = nullptr;
  this->loop_ = nullptr;
  std::cout << "LiveView Server stopped" << std::endl;
}

} // namespace photobooth
