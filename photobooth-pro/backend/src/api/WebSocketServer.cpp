#include "api/WebSocketServer.h"
#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace photobooth {

// Base64 encoding table
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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
  server_.set_open_handler([this](ConnectionHandle hdl) { this->onOpen(hdl); });
  server_.set_close_handler(
      [this](ConnectionHandle hdl) { this->onClose(hdl); });
  server_.set_message_handler(
      [this](ConnectionHandle hdl, WsServer::message_ptr msg) {
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
  std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(connectionsMutex_));
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
  std::lock_guard<std::mutex> lock(connectionsMutex_);
  connections_.erase(hdl);
  std::cout << "WebSocket client disconnected. Total: " << connections_.size()
            << std::endl;
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
      response["timestamp"] =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();
      server_.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } else if (type == "subscribe") {
      // Client subscribes to specific events
      json response;
      response["type"] = "subscribed";
      response["channel"] = request.value("channel", "all");
      server_.send(hdl, response.dump(), websocketpp::frame::opcode::text);
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

// broadcastBinary is already defined in the class as private, but I just made
// it public in header? Wait, I made it public in header, so I should implement
// it or use the existing implementation. Existing implementation: void
// WebSocketServer::broadcastBinary(const std::vector<uint8_t> &data) { ... } It
// is at line 162.

void WebSocketServer::broadcastLiveView(const std::vector<uint8_t> &imageData,
                                        int width, int height) {
  if (connections_.empty()) {
    return;
  }

  // Send raw binary JPEG data directly
  // The frontend handles this as a Blob -> URL
  broadcastBinary(imageData);
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

std::string WebSocketServer::encodeBase64(const std::vector<uint8_t> &data) {
  std::string encoded;
  encoded.reserve(((data.size() + 2) / 3) * 4);

  size_t i = 0;
  while (i < data.size()) {
    uint32_t octet_a = i < data.size() ? data[i++] : 0;
    uint32_t octet_b = i < data.size() ? data[i++] : 0;
    uint32_t octet_c = i < data.size() ? data[i++] : 0;

    uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

    encoded.push_back(base64_chars[(triple >> 18) & 0x3F]);
    encoded.push_back(base64_chars[(triple >> 12) & 0x3F]);
    encoded.push_back(base64_chars[(triple >> 6) & 0x3F]);
    encoded.push_back(base64_chars[triple & 0x3F]);
  }

  // Add padding
  size_t padding = data.size() % 3;
  if (padding > 0) {
    for (size_t p = padding; p < 3; ++p) {
      encoded[encoded.size() - (3 - p)] = '=';
    }
  }

  return encoded;
}

} // namespace photobooth
