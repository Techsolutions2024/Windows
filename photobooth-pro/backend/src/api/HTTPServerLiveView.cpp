#include "api/HTTPServer.h"
#include "camera/CameraManager.h"
#include "core/Application.h"
#include "httplib.h"
#include "nlohmann/json.hpp"

#include <chrono>
#include <thread>

using json = nlohmann::json;

namespace photobooth {

// ==================== Live View Handlers ====================

void HTTPServer::handleStartLiveView(const httplib::Request &req,
                                     httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  // Start live view with empty callback for now
  bool success = camMgr->startLiveView(nullptr);

  if (success) {
    res.set_content(jsonResponse(true, "Live view started"),
                    "application/json");
  } else {
    res.status = 400;
    res.set_content(jsonError("Failed to start live view", 400),
                    "application/json");
  }
}

void HTTPServer::handleStopLiveView(const httplib::Request &req,
                                    httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  camMgr->stopLiveView();
  res.set_content(jsonResponse(true, "Live view stopped"), "application/json");
}

void HTTPServer::handleLiveViewSSE(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);

  // Set SSE headers
  res.set_header("Content-Type", "text/event-stream");
  res.set_header("Cache-Control", "no-cache");
  res.set_header("Connection", "keep-alive");
  res.set_header("X-Accel-Buffering", "no"); // Disable nginx buffering

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    return;
  }

  std::cout << "SSE client connected for live view" << std::endl;

  // TODO: Implement SSE streaming using ProcessManager
  // For now, send a placeholder message
  std::string event = "data: SSE streaming not yet implemented\n\n";
  res.set_content(event, "text/event-stream");

  std::cout << "SSE stream ended (placeholder)" << std::endl;
}

// ==================== Base64 Encoding ====================

std::string HTTPServer::encodeBase64(const std::vector<uint8_t> &data) {
  static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  size_t in_len = data.size();
  const unsigned char *bytes_to_encode = data.data();

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}

} // namespace photobooth
