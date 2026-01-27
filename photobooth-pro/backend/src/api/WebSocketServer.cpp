#include "api/WebSocketServer.h"
#include "core/Application.h"
#include "camera/CameraManager.h"
#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace photobooth {

WebSocketServer::WebSocketServer(Application *app, int port)
    : app_(app), port_(port), running_(false) {
  // Configure server
  server_.set_access_channels(websocketpp::log::alevel::none);
  server_.clear_access_channels(websocketpp::log::alevel::all);
  server_.set_error_channels(websocketpp::log::elevel::warn |
                             websocketpp::log::elevel::rerror |
                             websocketpp::log::elevel::fatal);

  server_.init_asio();
  server_.set_reuse_addr(true);

  // Set handlers
  server_.set_open_handler(
      [this](ConnectionHandle hdl) { this->onOpen(hdl); });
  server_.set_close_handler(
      [this](ConnectionHandle hdl) { this->onClose(hdl); });
  server_.set_message_handler([this](ConnectionHandle hdl,
                                     WsServer::message_ptr msg) {
    this->onMessage(hdl, msg);
  });
}

WebSocketServer::~WebSocketServer() { stop(); }

bool WebSocketServer::start() {
  if (running_) {
    return true;
  }

  try {
    server_.listen(port_);
    server_.start_accept();

    serverThread_ = std::make_unique<std::thread>([this]() { run(); });

    running_ = true;
    std::cout << "WebSocket Server started on port " << port_ << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "WebSocket Server failed to start: " << e.what() << std::endl;
    return false;
  }
}

void WebSocketServer::stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  stopLiveViewBroadcast();

  try {
    server_.stop_listening();

    // Close all connections
    {
      std::lock_guard<std::mutex> lock(connectionsMutex_);
      for (auto &hdl : connections_) {
        try {
          server_.close(hdl, websocketpp::close::status::going_away,
                        "Server shutting down");
        } catch (...) {
        }
      }
      connections_.clear();
    }

    server_.stop();

    if (serverThread_ && serverThread_->joinable()) {
      serverThread_->join();
    }

    std::cout << "WebSocket Server stopped" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error stopping WebSocket Server: " << e.what() << std::endl;
  }
}

bool WebSocketServer::isRunning() const { return running_; }

size_t WebSocketServer::getConnectionCount() const {
  std::lock_guard<std::mutex> lock(
      const_cast<std::mutex &>(connectionsMutex_));
  return connections_.size();
}

void WebSocketServer::run() {
  try {
    server_.run();
  } catch (const std::exception &e) {
    std::cerr << "WebSocket Server error: " << e.what() << std::endl;
  }
}

void WebSocketServer::onOpen(ConnectionHandle hdl) {
  std::lock_guard<std::mutex> lock(connectionsMutex_);
  connections_.insert(hdl);
  std::cout << "WebSocket client connected. Total: " << connections_.size()
            << std::endl;
}

void WebSocketServer::onClose(ConnectionHandle hdl) {
  bool shouldStopLiveView = false;
  {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.erase(hdl);
    liveViewClients_.erase(hdl);
    readyClients_.erase(hdl);
    shouldStopLiveView = liveViewClients_.empty() && liveViewBroadcasting_;
    std::cout << "WebSocket client disconnected. Total: " << connections_.size()
              << std::endl;
  }

  if (shouldStopLiveView) {
    stopLiveViewBroadcast();
  }
}

void WebSocketServer::onMessage(ConnectionHandle hdl,
                                WsServer::message_ptr msg) {
  try {
    std::string payload = msg->get_payload();
    json request = json::parse(payload);

    std::string type = request.value("type", "");

    // Handle client messages
    if (type == "ping") {
      json response;
      response["type"] = "pong";
      response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
      server_.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } else if (type == "liveview:start") {
      {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        liveViewClients_.insert(hdl);
        // Prime the pump: "Start" implies ready for the first frame
        readyClients_.insert(hdl);
      }
      startLiveViewBroadcast();
      json response;
      response["type"] = "liveview:started";
      server_.send(hdl, response.dump(), websocketpp::frame::opcode::text);
      std::cout << "LiveView started for client. Ready set: " << 1 << std::endl;
    } else if (type == "liveview:stop") {
      bool shouldStop = false;
      {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        liveViewClients_.erase(hdl);
        readyClients_.erase(hdl);
        shouldStop = liveViewClients_.empty();
      }
      if (shouldStop) {
        stopLiveViewBroadcast();
      }
      json response;
      response["type"] = "liveview:stopped";
      server_.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } else if (type == "liveview:ready") {
      size_t readyCount = 0;
      {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        readyClients_.insert(hdl);
        readyCount = readyClients_.size();
      }
      // Debug logging (optional, can be removed in production)
      // std::cout << "Client ready. Total ready: " << readyCount << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error handling WebSocket message: " << e.what() << std::endl;
  }
}

void WebSocketServer::broadcast(const std::string &message) {
  std::lock_guard<std::mutex> lock(connectionsMutex_);
  for (auto &hdl : connections_) {
    try {
      server_.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (const std::exception &e) {
      std::cerr << "Error broadcasting message: " << e.what() << std::endl;
    }
  }
}

void WebSocketServer::broadcastBinary(const std::vector<uint8_t> &data) {
  std::lock_guard<std::mutex> lock(connectionsMutex_);
  for (auto &hdl : connections_) {
    try {
      server_.send(hdl, data.data(), data.size(),
                   websocketpp::frame::opcode::binary);
    } catch (const std::exception &e) {
      std::cerr << "Error broadcasting binary: " << e.what() << std::endl;
    }
  }
}

void WebSocketServer::startLiveViewBroadcast() {
  if (liveViewBroadcasting_) return;

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) return;

  if (!camMgr->startMjpegStream()) return;

  liveViewBroadcasting_ = true;
  liveViewThread_ = std::make_unique<std::thread>(&WebSocketServer::liveViewBroadcastLoop, this);
}

void WebSocketServer::stopLiveViewBroadcast() {
  if (!liveViewBroadcasting_) return;

  liveViewBroadcasting_ = false;

  auto *camMgr = app_->getCameraManager();
  if (camMgr) camMgr->stopMjpegStream();

  if (liveViewThread_ && liveViewThread_->joinable()) {
    liveViewThread_->join();
  }
  liveViewThread_.reset();
}

void WebSocketServer::liveViewBroadcastLoop() {
  auto *camMgr = app_->getCameraManager();

  while (liveViewBroadcasting_ && running_) {
    std::vector<uint8_t> frame;
    // Wait nicely for frame
    if (!camMgr->waitForFrame(frame, 100)) {
      continue; // timeout or stopped
    }

    // Copy subscribed client handles (release lock before I/O)
    std::vector<ConnectionHandle> clients;
    {
      std::lock_guard<std::mutex> lock(connectionsMutex_);
      clients.assign(liveViewClients_.begin(), liveViewClients_.end());
    }

    if (clients.empty()) continue;

    // Send binary JPEG ONLY to subscribed AND READY clients
    for (auto &hdl : clients) {
      bool isReady = false;
      {
          std::lock_guard<std::mutex> lock(connectionsMutex_);
          auto it = readyClients_.find(hdl);
          if (it != readyClients_.end()) {
              isReady = true;
              readyClients_.erase(it); // Consume the ready token
          }
      }

      if (isReady) {
          try {
            // Check socket state before sending to avoid 10053 hard crashes
            auto con = server_.get_con_from_hdl(hdl);
            if (con && con->get_state() == websocketpp::session::state::open) {
                 server_.send(hdl, frame.data(), frame.size(), websocketpp::frame::opcode::binary);
            }
          } catch (const websocketpp::exception &e) {
             // Specific websocket error - client likely disconnected
             std::cerr << "WS Send Error: " << e.what() << std::endl;
          } catch (...) {
             // General error
             std::cerr << "Unknown error sending frame" << std::endl;
          }
      }
    }
  }
}

void WebSocketServer::broadcastEvent(const std::string &eventType,
                                     const std::string &data) {
  json message;
  message["type"] = eventType;
  if (!data.empty()) {
    try {
      message["data"] = json::parse(data);
    } catch (...) {
      message["data"] = data;
    }
  }
  broadcast(message.dump());
}

void WebSocketServer::broadcastCountdown(int seconds) {
  json message;
  message["type"] = "capture:countdown";
  message["data"]["seconds"] = seconds;
  broadcast(message.dump());
}

void WebSocketServer::broadcastCaptureComplete(const std::string &imagePath) {
  json message;
  message["type"] = "capture:complete";
  message["data"]["imagePath"] = imagePath;
  message["data"]["timestamp"] =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  broadcast(message.dump());
}

} // namespace photobooth
