#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

// WebSocket++ configuration - use C++11 STL instead of Boost
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#ifndef _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_THREAD_
#endif

#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif

#ifndef _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#endif

#ifndef _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#endif

#ifndef _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#endif

#ifndef _WEBSOCKETPP_CPP11_MEMORY_
#define _WEBSOCKETPP_CPP11_MEMORY_
#endif

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace photobooth {

class Application;

using WsServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionHandle = websocketpp::connection_hdl;

class WebSocketServer {
public:
  WebSocketServer(Application *app, int port = 8081);
  ~WebSocketServer();

  bool start();
  void stop();
  bool isRunning() const;

  // Event broadcast (text JSON)
  void broadcastEvent(const std::string &eventType, const std::string &data);
  void broadcastCountdown(int seconds);
  void broadcastCaptureComplete(const std::string &imagePath);

  // Live view streaming
  void startLiveViewBroadcast();
  void stopLiveViewBroadcast();

  int getPort() const { return port_; }
  size_t getConnectionCount() const;

private:
  Application *app_;
  int port_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> serverThread_;

  // Live view broadcast thread
  std::unique_ptr<std::thread> liveViewThread_;
  std::atomic<bool> liveViewBroadcasting_{false};

  // WebSocket++ server
  WsServer server_;
  std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> connections_;
  std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> liveViewClients_;
  // Clients who have explicitly signaled they are ready for the next frame
  std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> readyClients_;
  std::mutex connectionsMutex_;

  void run();
  void liveViewBroadcastLoop();
  void onOpen(ConnectionHandle hdl);
  void onClose(ConnectionHandle hdl);
  void onMessage(ConnectionHandle hdl, WsServer::message_ptr msg);

  void broadcast(const std::string &message);
  void broadcastBinary(const std::vector<uint8_t> &data);
};

} // namespace photobooth
