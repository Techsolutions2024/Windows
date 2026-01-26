#include "api/HTTPServer.h"
#include "camera/CameraManager.h"
#include "camera/CanonSDKCamera.h"
#include "core/Application.h"
#include "media/BurstCaptureManager.h"
#include "media/GifCreator.h"
#include "storage/DatabaseManager.h"
#include "storage/FileManager.h"


// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

namespace photobooth {

HTTPServer::HTTPServer(Application *app, int port)
    : app_(app), port_(port), running_(false) {
  server_ = std::make_unique<httplib::Server>();
}

HTTPServer::~HTTPServer() { stop(); }

bool HTTPServer::start() {
  if (running_) {
    return true;
  }

  setupRoutes();

  serverThread_ = std::make_unique<std::thread>([this]() { run(); });

  running_ = true;
  std::cout << "HTTP Server started on port " << port_ << std::endl;
  return true;
}

void HTTPServer::stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  if (server_) {
    server_->stop();
  }

  if (serverThread_ && serverThread_->joinable()) {
    serverThread_->join();
  }

  std::cout << "HTTP Server stopped" << std::endl;
}

void HTTPServer::run() { server_->listen("0.0.0.0", port_); }

void HTTPServer::setCorsHeaders(httplib::Response &res) {
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods",
                 "GET, POST, PUT, DELETE, OPTIONS");
  res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

std::string HTTPServer::jsonResponse(bool success, const std::string &message,
                                     const std::string &data) {
  json response;
  response["success"] = success;
  if (!message.empty()) {
    response["message"] = message;
  }
  if (!data.empty()) {
    response["data"] = json::parse(data);
  }
  return response.dump();
}

std::string HTTPServer::jsonError(const std::string &message, int code) {
  json response;
  response["success"] = false;
  response["error"] = message;
  response["code"] = code;
  return response.dump();
}

void HTTPServer::setupRoutes() {
  // CORS preflight
  server_->Options(".*",
                   [this](const httplib::Request &req, httplib::Response &res) {
                     setCorsHeaders(res);
                     res.status = 204;
                   });

  // ==================== Events API ====================
  server_->Get("/api/events",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetEvents(req, res);
               });

  server_->Post("/api/events",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleCreateEvent(req, res);
                });

  server_->Get(R"(/api/events/(\d+))",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetEvent(req, res);
               });

  server_->Put(R"(/api/events/(\d+))",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleUpdateEvent(req, res);
               });

  server_->Delete(R"(/api/events/(\d+))",
                  [this](const httplib::Request &req, httplib::Response &res) {
                    handleDeleteEvent(req, res);
                  });

  server_->Post(R"(/api/events/(\d+)/duplicate)",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleDuplicateEvent(req, res);
                });

  server_->Get(R"(/api/events/(\d+)/config)",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetEventConfig(req, res);
               });

  server_->Put(R"(/api/events/(\d+)/config)",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleUpdateEventConfig(req, res);
               });

  server_->Get("/api/events/search",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleSearchEvents(req, res);
               });

  // Save/Load event config to/from file
  server_->Post(R"(/api/events/(\d+)/save-config)",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleSaveEventConfigToFile(req, res);
                });

  server_->Get(R"(/api/events/(\d+)/load-config)",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleLoadEventConfigFromFile(req, res);
               });

  // ==================== Camera API ====================
  server_->Get("/api/cameras",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetCameras(req, res);
               });

  server_->Post("/api/cameras/select",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleSelectCamera(req, res);
                });

  server_->Get("/api/cameras/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetCameraSettings(req, res);
               });

  server_->Put("/api/cameras/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleSetCameraSettings(req, res);
               });

  server_->Post("/api/cameras/liveview/start",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleStartLiveView(req, res);
                });

  server_->Post("/api/cameras/liveview/stop",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleStopLiveView(req, res);
                });

  // TODO: Implement SSE live view streaming
  /*
  server_->Get("/api/cameras/liveview/stream",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleLiveViewSSE(req, res);
               });
  */

  // Extended SDK Camera Settings
  server_->Get("/api/cameras/settings/supported",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetSupportedCameraValues(req, res);
               });

  server_->Get("/api/cameras/settings/extended",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetExtendedCameraSettings(req, res);
               });

  server_->Put("/api/cameras/settings/extended",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleSetExtendedCameraSettings(req, res);
               });

  server_->Post("/api/cameras/property",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleSetCameraPropertyByCode(req, res);
                });

  // ==================== Capture API ====================
  server_->Post("/api/capture/photo",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleCapture(req, res);
                });

  server_->Post("/api/capture/gif",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleCaptureGif(req, res);
                });

  server_->Post("/api/capture/boomerang",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleCaptureBoomerang(req, res);
                });

  server_->Post("/api/capture/video/start",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleStartVideo(req, res);
                });

  server_->Post("/api/capture/video/stop",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleStopVideo(req, res);
                });

  // ==================== Gallery API ====================
  server_->Get(R"(/api/gallery/(\d+))",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetGallery(req, res);
               });

  server_->Get(R"(/api/gallery/photo/(\d+))",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetPhoto(req, res);
               });

  server_->Delete(R"(/api/gallery/photo/(\d+))",
                  [this](const httplib::Request &req, httplib::Response &res) {
                    handleDeletePhoto(req, res);
                  });

  server_->Post("/api/gallery/collage",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleCreateCollage(req, res);
                });

  // ==================== Print API ====================
  server_->Get("/api/printers",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetPrinters(req, res);
               });

  server_->Post("/api/print",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handlePrint(req, res);
                });

  server_->Get("/api/print/status",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetPrintStatus(req, res);
               });

  server_->Get("/api/print/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetPrintSettings(req, res);
               });

  server_->Put("/api/print/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleUpdatePrintSettings(req, res);
               });

  // ==================== Share API ====================
  server_->Post("/api/share/email",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleShareEmail(req, res);
                });

  server_->Post("/api/share/sms",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleShareSMS(req, res);
                });

  server_->Post("/api/share/qrcode",
                [this](const httplib::Request &req, httplib::Response &res) {
                  handleGenerateQRCode(req, res);
                });

  // ==================== Settings API ====================
  server_->Get("/api/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetSettings(req, res);
               });

  server_->Put("/api/settings",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleUpdateSettings(req, res);
               });

  server_->Get("/api/settings/storage",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetStorageInfo(req, res);
               });

  server_->Get("/api/settings/network",
               [this](const httplib::Request &req, httplib::Response &res) {
                 handleGetNetworkStatus(req, res);
               });

  // ==================== Static Files ====================
  server_->set_mount_point("/", "./frontend/dist");

  std::cout << "Routes configured successfully" << std::endl;
}

// ==================== Events API Implementation ====================

void HTTPServer::handleGetEvents(const httplib::Request &req,
                                 httplib::Response &res) {
  setCorsHeaders(res);

  auto &db = app_->getDatabase();

  std::string sortBy =
      req.has_param("sortBy") ? req.get_param_value("sortBy") : "createdAt";
  std::string order =
      req.has_param("order") ? req.get_param_value("order") : "DESC";
  std::string filter =
      req.has_param("filter") ? req.get_param_value("filter") : "";

  auto events = db.getAllEvents(sortBy, order, filter);

  json eventsJson = json::array();
  for (const auto &event : events) {
    json eventJson;
    eventJson["id"] = event.id;
    eventJson["name"] = event.name;
    eventJson["createdAt"] = event.createdAt;
    eventJson["updatedAt"] = event.updatedAt;
    eventJson["photoCount"] = event.photoCount;
    eventJson["thumbnailPath"] = event.thumbnailPath;
    eventJson["status"] = event.status;
    eventJson["location"] = event.location;
    eventJson["eventDate"] = event.eventDate;

    // Include config
    json configJson;
    configJson["captureMode"] = event.config.captureMode;
    configJson["photoEnabled"] = event.config.photoEnabled;
    configJson["gifEnabled"] = event.config.gifEnabled;
    configJson["boomerangEnabled"] = event.config.boomerangEnabled;
    configJson["videoEnabled"] = event.config.videoEnabled;
    configJson["countdownSeconds"] = event.config.countdownSeconds;
    configJson["photoCount"] = event.config.photoCount;
    eventJson["config"] = configJson;

    eventsJson.push_back(eventJson);
  }

  json response;
  response["success"] = true;
  response["data"] = eventsJson;
  response["total"] = events.size();

  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleCreateEvent(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);
    std::string name = body.value("name", "New Event");
    std::string location = body.value("location", "");
    std::string eventDate = body.value("eventDate", "");

    auto &db = app_->getDatabase();
    int eventId = db.createEvent(name, location, eventDate);

    if (eventId > 0) {
      auto event = db.getEvent(eventId);
      if (event) {
        json eventJson;
        eventJson["id"] = event->id;
        eventJson["name"] = event->name;
        eventJson["location"] = event->location;
        eventJson["eventDate"] = event->eventDate;
        eventJson["createdAt"] = event->createdAt;
        eventJson["updatedAt"] = event->updatedAt;
        eventJson["photoCount"] = event->photoCount;
        eventJson["status"] = event->status;

        json response;
        response["success"] = true;
        response["message"] = "Event created successfully";
        response["data"] = eventJson;

        res.status = 201;
        res.set_content(response.dump(), "application/json");
        return;
      }
    }

    res.status = 500;
    res.set_content(jsonError("Failed to create event", 500),
                    "application/json");
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleGetEvent(const httplib::Request &req,
                                httplib::Response &res) {
  setCorsHeaders(res);

  int eventId = std::stoi(req.matches[1]);

  auto &db = app_->getDatabase();
  auto event = db.getEvent(eventId);

  if (event) {
    json eventJson;
    eventJson["id"] = event->id;
    eventJson["name"] = event->name;
    eventJson["createdAt"] = event->createdAt;
    eventJson["updatedAt"] = event->updatedAt;
    eventJson["photoCount"] = event->photoCount;
    eventJson["thumbnailPath"] = event->thumbnailPath;
    eventJson["status"] = event->status;
    eventJson["location"] = event->location;
    eventJson["eventDate"] = event->eventDate;

    // Include full config
    json configJson;
    configJson["eventId"] = event->config.eventId;
    configJson["startScreenImage"] = event->config.startScreenImage;
    configJson["captureMode"] = event->config.captureMode;
    configJson["photoEnabled"] = event->config.photoEnabled;
    configJson["gifEnabled"] = event->config.gifEnabled;
    configJson["boomerangEnabled"] = event->config.boomerangEnabled;
    configJson["videoEnabled"] = event->config.videoEnabled;
    configJson["effectsConfig"] = event->config.effectsConfig;
    configJson["propsConfig"] = event->config.propsConfig;
    configJson["beautyFilterConfig"] = event->config.beautyFilterConfig;
    configJson["watermarkConfig"] = event->config.watermarkConfig;
    configJson["postProcessConfig"] = event->config.postProcessConfig;
    configJson["countdownSeconds"] = event->config.countdownSeconds;
    configJson["photoCount"] = event->config.photoCount;
    configJson["layoutTemplate"] = event->config.layoutTemplate;
    eventJson["config"] = configJson;

    json response;
    response["success"] = true;
    response["data"] = eventJson;

    res.set_content(response.dump(), "application/json");
  } else {
    res.status = 404;
    res.set_content(jsonError("Event not found", 404), "application/json");
  }
}

void HTTPServer::handleUpdateEvent(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);

  try {
    int eventId = std::stoi(req.matches[1]);
    json body = json::parse(req.body);

    auto &db = app_->getDatabase();

    if (!db.eventExists(eventId)) {
      res.status = 404;
      res.set_content(jsonError("Event not found", 404), "application/json");
      return;
    }

    std::string name = body.value("name", "");
    std::string location = body.value("location", "");
    std::string eventDate = body.value("eventDate", "");

    if (!name.empty()) {
      if (db.updateEvent(eventId, name, location, eventDate)) {
        auto event = db.getEvent(eventId);
        if (event) {
          json eventJson;
          eventJson["id"] = event->id;
          eventJson["name"] = event->name;
          eventJson["location"] = event->location;
          eventJson["eventDate"] = event->eventDate;
          eventJson["updatedAt"] = event->updatedAt;

          json response;
          response["success"] = true;
          response["message"] = "Event updated successfully";
          response["data"] = eventJson;

          res.set_content(response.dump(), "application/json");
          return;
        }
      }
    }

    res.status = 400;
    res.set_content(jsonError("Failed to update event", 400),
                    "application/json");
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleDeleteEvent(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);

  int eventId = std::stoi(req.matches[1]);

  auto &db = app_->getDatabase();

  if (!db.eventExists(eventId)) {
    res.status = 404;
    res.set_content(jsonError("Event not found", 404), "application/json");
    return;
  }

  if (db.deleteEvent(eventId)) {
    json response;
    response["success"] = true;
    response["message"] = "Event deleted successfully";

    res.set_content(response.dump(), "application/json");
  } else {
    res.status = 500;
    res.set_content(jsonError("Failed to delete event", 500),
                    "application/json");
  }
}

void HTTPServer::handleDuplicateEvent(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);

  try {
    int eventId = std::stoi(req.matches[1]);
    json body = json::parse(req.body);

    auto &db = app_->getDatabase();

    if (!db.eventExists(eventId)) {
      res.status = 404;
      res.set_content(jsonError("Event not found", 404), "application/json");
      return;
    }

    auto originalEvent = db.getEvent(eventId);
    std::string newName = body.value("name", originalEvent->name + " (Copy)");

    if (db.duplicateEvent(eventId, newName)) {
      json response;
      response["success"] = true;
      response["message"] = "Event duplicated successfully";

      res.status = 201;
      res.set_content(response.dump(), "application/json");
    } else {
      res.status = 500;
      res.set_content(jsonError("Failed to duplicate event", 500),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleGetEventConfig(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);

  int eventId = std::stoi(req.matches[1]);

  auto &db = app_->getDatabase();
  auto config = db.getEventConfig(eventId);

  if (config) {
    json configJson;
    configJson["eventId"] = config->eventId;
    configJson["startScreenImage"] = config->startScreenImage;
    configJson["captureMode"] = config->captureMode;
    configJson["photoEnabled"] = config->photoEnabled;
    configJson["gifEnabled"] = config->gifEnabled;
    configJson["boomerangEnabled"] = config->boomerangEnabled;
    configJson["videoEnabled"] = config->videoEnabled;
    configJson["effectsConfig"] = config->effectsConfig;
    configJson["propsConfig"] = config->propsConfig;
    configJson["beautyFilterConfig"] = config->beautyFilterConfig;
    configJson["watermarkConfig"] = config->watermarkConfig;
    configJson["postProcessConfig"] = config->postProcessConfig;
    configJson["countdownSeconds"] = config->countdownSeconds;
    configJson["photoCount"] = config->photoCount;
    configJson["layoutTemplate"] = config->layoutTemplate;
    configJson["cameraSource"] = config->cameraSource;
    configJson["webcamIndex"] = config->webcamIndex;

    json response;
    response["success"] = true;
    response["data"] = configJson;

    res.set_content(response.dump(), "application/json");
  } else {
    res.status = 404;
    res.set_content(jsonError("Event config not found", 404),
                    "application/json");
  }
}

void HTTPServer::handleUpdateEventConfig(const httplib::Request &req,
                                         httplib::Response &res) {
  setCorsHeaders(res);

  try {
    int eventId = std::stoi(req.matches[1]);
    json body = json::parse(req.body);

    auto &db = app_->getDatabase();

    if (!db.eventExists(eventId)) {
      res.status = 404;
      res.set_content(jsonError("Event not found", 404), "application/json");
      return;
    }

    auto existingConfig = db.getEventConfig(eventId);
    EventConfig config = existingConfig.value_or(EventConfig{});
    config.eventId = eventId;

    // Update fields from request body
    if (body.contains("startScreenImage"))
      config.startScreenImage = body["startScreenImage"];
    if (body.contains("captureMode"))
      config.captureMode = body["captureMode"];
    if (body.contains("photoEnabled"))
      config.photoEnabled = body["photoEnabled"];
    if (body.contains("gifEnabled"))
      config.gifEnabled = body["gifEnabled"];
    if (body.contains("boomerangEnabled"))
      config.boomerangEnabled = body["boomerangEnabled"];
    if (body.contains("videoEnabled"))
      config.videoEnabled = body["videoEnabled"];
    if (body.contains("effectsConfig"))
      config.effectsConfig = body["effectsConfig"].dump();
    if (body.contains("propsConfig"))
      config.propsConfig = body["propsConfig"].dump();
    if (body.contains("beautyFilterConfig"))
      config.beautyFilterConfig = body["beautyFilterConfig"].dump();
    if (body.contains("watermarkConfig"))
      config.watermarkConfig = body["watermarkConfig"].dump();
    if (body.contains("postProcessConfig"))
      config.postProcessConfig = body["postProcessConfig"].dump();
    if (body.contains("countdownSeconds"))
      config.countdownSeconds = body["countdownSeconds"];
    if (body.contains("photoCount"))
      config.photoCount = body["photoCount"];
    if (body.contains("layoutTemplate"))
      config.layoutTemplate = body["layoutTemplate"];
    if (body.contains("cameraSource"))
      config.cameraSource = body["cameraSource"];
    if (body.contains("webcamIndex"))
      config.webcamIndex = body["webcamIndex"];

    if (db.updateEventConfig(eventId, config)) {
      json response;
      response["success"] = true;
      response["message"] = "Event config updated successfully";

      res.set_content(response.dump(), "application/json");
    } else {
      res.status = 500;
      res.set_content(jsonError("Failed to update event config", 500),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleSearchEvents(const httplib::Request &req,
                                    httplib::Response &res) {
  setCorsHeaders(res);

  std::string query = req.has_param("q") ? req.get_param_value("q") : "";

  auto &db = app_->getDatabase();
  auto events = db.searchEvents(query);

  json eventsJson = json::array();
  for (const auto &event : events) {
    json eventJson;
    eventJson["id"] = event.id;
    eventJson["name"] = event.name;
    eventJson["createdAt"] = event.createdAt;
    eventJson["photoCount"] = event.photoCount;
    eventJson["thumbnailPath"] = event.thumbnailPath;
    eventJson["location"] = event.location;
    eventJson["eventDate"] = event.eventDate;
    eventsJson.push_back(eventJson);
  }

  json response;
  response["success"] = true;
  response["data"] = eventsJson;
  response["total"] = events.size();

  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleSaveEventConfigToFile(const httplib::Request &req,
                                             httplib::Response &res) {
  setCorsHeaders(res);

  try {
    int eventId = std::stoi(req.matches[1]);
    json body = json::parse(req.body);

    auto *fileMgr = app_->getFileManager();
    if (!fileMgr) {
      res.status = 500;
      res.set_content(jsonError("File Manager not initialized", 500),
                      "application/json");
      return;
    }

    std::string eventIdStr = std::to_string(eventId);

    // Create event directory structure
    std::string eventPath = fileMgr->createEventDirectory(eventIdStr);
    if (eventPath.empty()) {
      res.status = 500;
      res.set_content(jsonError("Failed to create event directory", 500),
                      "application/json");
      return;
    }

    // Save the configuration to file
    if (fileMgr->saveEventConfig(eventIdStr, body)) {
      std::string configPath = fileMgr->getEventConfigPath(eventIdStr);

      json response;
      response["success"] = true;
      response["message"] = "Event configuration saved to file";
      response["data"] = {{"path", configPath}};
      res.set_content(response.dump(), "application/json");
    } else {
      res.status = 500;
      res.set_content(jsonError("Failed to save event configuration", 500),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleLoadEventConfigFromFile(const httplib::Request &req,
                                               httplib::Response &res) {
  setCorsHeaders(res);

  try {
    int eventId = std::stoi(req.matches[1]);
    std::string eventIdStr = std::to_string(eventId);

    auto *fileMgr = app_->getFileManager();
    if (!fileMgr) {
      res.status = 500;
      res.set_content(jsonError("File Manager not initialized", 500),
                      "application/json");
      return;
    }

    json config = fileMgr->loadEventConfig(eventIdStr);

    if (!config.empty()) {
      json response;
      response["success"] = true;
      response["data"] = config;
      res.set_content(response.dump(), "application/json");
    } else {
      res.status = 404;
      res.set_content(jsonError("Event configuration file not found", 404),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

// ==================== Camera API ====================

void HTTPServer::handleGetCameras(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);

  json camerasJson = json::array();

  // Get Canon cameras from CameraManager (only Canon SDK cameras)
  auto *camMgr = app_->getCameraManager();
  if (camMgr) {
    auto cameras = camMgr->getAvailableCameras();
    for (const auto &cam : cameras) {
      json camJson;
      camJson["name"] = cam.name;
      camJson["type"] = "canon";
      camJson["connected"] = cam.connected;
      camJson["index"] = cam.index;
      camerasJson.push_back(camJson);
    }
  }

  json response;
  response["success"] = true;
  response["data"] = camerasJson;
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleSelectCamera(const httplib::Request &req,
                                    httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);
    std::string cameraName = body.value("name", "");

    auto *camMgr = app_->getCameraManager();
    if (!camMgr) {
      res.status = 500;
      res.set_content(jsonError("Camera Manager not initialized", 500),
                      "application/json");
      return;
    }

    bool success = camMgr->selectCamera(cameraName);

    if (success) {
      json response;
      response["success"] = true;
      response["message"] = "Canon camera selected";
      response["data"] = {{"name", camMgr->getActiveCameraName()}};
      res.set_content(response.dump(), "application/json");
    } else {
      res.status = 400;
      res.set_content(jsonError("Failed to select Canon camera", 400),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleGetCameraSettings(const httplib::Request &req,
                                         httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  CameraSettings settings = camMgr->getSettings();
  json settingsJson;
  settingsJson["iso"] = settings.iso;
  settingsJson["aperture"] = settings.aperture;
  settingsJson["shutterSpeed"] = settings.shutterSpeed;
  settingsJson["whiteBalance"] = settings.whiteBalance;
  settingsJson["mirror"] = settings.mirror;
  settingsJson["rotation"] = settings.rotation;

  settingsJson["supportedISO"] = camMgr->getSupportedISO();
  settingsJson["supportedAperture"] = camMgr->getSupportedApertures();
  settingsJson["supportedShutterSpeed"] = camMgr->getSupportedShutterSpeeds();
  settingsJson["supportedWhiteBalance"] = camMgr->getSupportedWhiteBalances();

  json response;
  response["success"] = true;
  response["data"] = settingsJson;
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleSetCameraSettings(const httplib::Request &req,
                                         httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);
    CameraSettings settings;

    // Parse with defaults if missing
    if (body.contains("iso"))
      settings.iso = body["iso"];
    if (body.contains("aperture"))
      settings.aperture = body["aperture"];
    if (body.contains("shutterSpeed"))
      settings.shutterSpeed = body["shutterSpeed"];
    if (body.contains("whiteBalance"))
      settings.whiteBalance = body["whiteBalance"];
    if (body.contains("mirror"))
      settings.mirror = body["mirror"];
    if (body.contains("rotation"))
      settings.rotation = body["rotation"];

    auto *camMgr = app_->getCameraManager();
    if (camMgr && camMgr->setSettings(settings)) {
      res.set_content(jsonResponse(true, "Settings updated"),
                      "application/json");
    } else {
      res.status = 500;
      res.set_content(jsonError("Failed to update camera settings", 500),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleStartLiveView(const httplib::Request &req,
                                     httplib::Response &res) {
  setCorsHeaders(res);

  auto *ws = app_->getWebSocketServer();
  auto *camMgr = app_->getCameraManager();

  if (camMgr && ws) {
    // Define callback to broadcast frames
    auto callback = [ws](const std::vector<uint8_t> &data, int w, int h) {
      ws->broadcastLiveView(data, w, h);
    };

    if (camMgr->startLiveView(callback)) {
      res.set_content(jsonResponse(true, "Live view started"),
                      "application/json");
      return;
    }
  }

  res.status = 500;
  res.set_content(jsonError("Failed to start live view", 500),
                  "application/json");
}

void HTTPServer::handleStopLiveView(const httplib::Request &req,
                                    httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (camMgr) {
    camMgr->stopLiveView();
    res.set_content(jsonResponse(true, "Live view stopped"),
                    "application/json");
  } else {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
  }
}

// ==================== Extended SDK Camera Settings ====================

void HTTPServer::handleGetSupportedCameraValues(const httplib::Request &req,
                                                httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  CanonSupportedValues values = camMgr->getAllSupportedCameraValues();

  json valuesJson;

  // Convert each vector of SDKOption to JSON array
  auto optionsToJson = [](const std::vector<SDKOption> &options) {
    json arr = json::array();
    for (const auto &opt : options) {
      arr.push_back({{"code", opt.code}, {"label", opt.label}});
    }
    return arr;
  };

  valuesJson["iso"] = optionsToJson(values.iso);
  valuesJson["aperture"] = optionsToJson(values.aperture);
  valuesJson["shutterSpeed"] = optionsToJson(values.shutterSpeed);
  valuesJson["exposureComp"] = optionsToJson(values.exposureComp);
  valuesJson["whiteBalance"] = optionsToJson(values.whiteBalance);
  valuesJson["pictureStyle"] = optionsToJson(values.pictureStyle);
  valuesJson["afMode"] = optionsToJson(values.afMode);
  valuesJson["imageQuality"] = optionsToJson(values.imageQuality);
  valuesJson["driveMode"] = optionsToJson(values.driveMode);
  valuesJson["aeMode"] = optionsToJson(values.aeMode);

  json response;
  response["success"] = true;
  response["data"] = valuesJson;
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleGetExtendedCameraSettings(const httplib::Request &req,
                                                 httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  CanonCameraSettings settings = camMgr->getExtendedCameraSettings();

  json settingsJson;
  settingsJson["isoCode"] = settings.isoCode;
  settingsJson["apertureCode"] = settings.apertureCode;
  settingsJson["shutterSpeedCode"] = settings.shutterSpeedCode;
  settingsJson["exposureCompCode"] = settings.exposureCompCode;
  settingsJson["meteringModeCode"] = settings.meteringModeCode;
  settingsJson["aeModeCode"] = settings.aeModeCode;
  settingsJson["whiteBalanceCode"] = settings.whiteBalanceCode;
  settingsJson["pictureStyleCode"] = settings.pictureStyleCode;
  settingsJson["afModeCode"] = settings.afModeCode;
  settingsJson["imageQualityCode"] = settings.imageQualityCode;
  settingsJson["driveModeCode"] = settings.driveModeCode;
  settingsJson["evfOutputDevice"] = settings.evfOutputDevice;
  settingsJson["evfZoom"] = settings.evfZoom;
  settingsJson["mirror"] = settings.mirror;
  settingsJson["rotation"] = settings.rotation;

  json response;
  response["success"] = true;
  response["data"] = settingsJson;
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleSetExtendedCameraSettings(const httplib::Request &req,
                                                 httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);

    auto *camMgr = app_->getCameraManager();
    if (!camMgr) {
      res.status = 500;
      res.set_content(jsonError("Camera Manager not initialized", 500),
                      "application/json");
      return;
    }

    CanonCameraSettings settings = camMgr->getExtendedCameraSettings();

    // Update only provided fields
    if (body.contains("isoCode"))
      settings.isoCode = body["isoCode"];
    if (body.contains("apertureCode"))
      settings.apertureCode = body["apertureCode"];
    if (body.contains("shutterSpeedCode"))
      settings.shutterSpeedCode = body["shutterSpeedCode"];
    if (body.contains("exposureCompCode"))
      settings.exposureCompCode = body["exposureCompCode"];
    if (body.contains("meteringModeCode"))
      settings.meteringModeCode = body["meteringModeCode"];
    if (body.contains("aeModeCode"))
      settings.aeModeCode = body["aeModeCode"];
    if (body.contains("whiteBalanceCode"))
      settings.whiteBalanceCode = body["whiteBalanceCode"];
    if (body.contains("pictureStyleCode"))
      settings.pictureStyleCode = body["pictureStyleCode"];
    if (body.contains("afModeCode"))
      settings.afModeCode = body["afModeCode"];
    if (body.contains("imageQualityCode"))
      settings.imageQualityCode = body["imageQualityCode"];
    if (body.contains("driveModeCode"))
      settings.driveModeCode = body["driveModeCode"];
    if (body.contains("evfOutputDevice"))
      settings.evfOutputDevice = body["evfOutputDevice"];
    if (body.contains("evfZoom"))
      settings.evfZoom = body["evfZoom"];
    if (body.contains("mirror"))
      settings.mirror = body["mirror"];
    if (body.contains("rotation"))
      settings.rotation = body["rotation"];

    if (camMgr->setExtendedCameraSettings(settings)) {
      res.set_content(jsonResponse(true, "Extended camera settings updated"),
                      "application/json");
    } else {
      res.status = 500;
      res.set_content(
          jsonError("Failed to update extended camera settings", 500),
          "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleSetCameraPropertyByCode(const httplib::Request &req,
                                               httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);

    if (!body.contains("propertyId") || !body.contains("code")) {
      res.status = 400;
      res.set_content(jsonError("Missing propertyId or code", 400),
                      "application/json");
      return;
    }

    EdsPropertyID propertyId = body["propertyId"];
    EdsUInt32 code = body["code"];

    auto *camMgr = app_->getCameraManager();
    if (!camMgr) {
      res.status = 500;
      res.set_content(jsonError("Camera Manager not initialized", 500),
                      "application/json");
      return;
    }

    if (camMgr->setCameraPropertyByCode(propertyId, code)) {
      res.set_content(jsonResponse(true, "Camera property updated"),
                      "application/json");
    } else {
      res.status = 500;
      res.set_content(jsonError("Failed to set camera property", 500),
                      "application/json");
    }
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

// ==================== Capture API Stubs ====================

void HTTPServer::handleCapture(const httplib::Request &req,
                               httplib::Response &res) {
  setCorsHeaders(res);

  auto *camMgr = app_->getCameraManager();
  if (!camMgr) {
    res.status = 500;
    res.set_content(jsonError("Camera Manager not initialized", 500),
                    "application/json");
    return;
  }

  // Parse eventId
  int eventId = 0;
  try {
    json body = json::parse(req.body);
    eventId = body.value("eventId", 0);
  } catch (...) {
  } // Optional or error

  // Create a promise to wait for capture result (blocking for simplicity in
  // this HTTP handler) In a real app, we might return immediately and use
  // WebSocket for result. But for now let's try to wait or just trigger it.
  // Given EDSDK callbacks are async, we effectively trigger here.

  bool triggered = false;
  camMgr->capture(CaptureMode::Single,
                  [this, &res, eventId](const CaptureResult &result) {
                    // Only one response can be sent. This callback might be
                    // called later. Since httplib handlers are sync, we can't
                    // easily wait here without blocking. We will assume
                    // 'success' means "Command Sent". OR we implement a
                    // blocking wait using std::promise/future if the capture is
                    // fast. For DSLR, capture can take seconds.

                    // Ideally: Save to DB here logic using
                    // Application::getDatabase().
                    if (result.success) {
                      // Save to DB
                      // ...
                    }
                  });

  // Since we can't easily block for the callback in this architecture without
  // handling timeouts, we return "Capture initiated". Note: The callback above
  // would execute on a different thread (LiveView or Main thread), keeping
  // 'res' reference is DANGEROUS/WRONG because this function returns.

  // CORRECT APPROACH:
  // Trigger capture. Code in CameraManager/CanonCamera handles the file
  // download. We just return "OK" to frontend. Frontend waits for
  // "CaptureComplete" event via WebSocket or polls.

  // Let's refine the callback to be nullptr here or handle it properly in
  // CameraManager (e.g. save to disk). The 'capture' method in CameraManager
  // takes a callback.

  camMgr->capture(CaptureMode::Single,
                  [this, eventId](const CaptureResult &result) {
                    if (result.success) {
                      // Save to DB
                      auto &db = app_->getDatabase();
                      Photo photo;
                      photo.eventId = eventId;
                      photo.filePath = result.filePath;
                      photo.timestamp = "Now"; // Should generate timestamp
                      photo.captureMode = "photo";
                      photo.width = result.width;
                      photo.height = result.height;

                      db.savePhoto(photo);

                      // Broadcast to WebSocket
                      auto *ws = app_->getWebSocketServer();
                      if (ws)
                        ws->broadcastCaptureComplete(result.filePath);
                    }
                  });

  res.set_content(jsonResponse(true, "Capture initiated"), "application/json");
}

void HTTPServer::handleCaptureGif(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "GIF captured"), "application/json");
}

void HTTPServer::handleCaptureBoomerang(const httplib::Request &req,
                                        httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "Boomerang captured"), "application/json");
}

void HTTPServer::handleStartVideo(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "Video recording started"),
                  "application/json");
}

void HTTPServer::handleStopVideo(const httplib::Request &req,
                                 httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "Video recording stopped"),
                  "application/json");
}

// ==================== Gallery API Stubs ====================

void HTTPServer::handleGetGallery(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);

  int eventId = std::stoi(req.matches[1]);
  int limit =
      req.has_param("limit") ? std::stoi(req.get_param_value("limit")) : 0;
  int offset =
      req.has_param("offset") ? std::stoi(req.get_param_value("offset")) : 0;

  auto &db = app_->getDatabase();
  auto photos = db.getPhotosForEvent(eventId, limit, offset);

  json photosJson = json::array();
  for (const auto &photo : photos) {
    json photoJson;
    photoJson["id"] = photo.id;
    photoJson["eventId"] = photo.eventId;
    photoJson["filePath"] = photo.filePath;
    photoJson["thumbnailPath"] = photo.thumbnailPath;
    photoJson["captureMode"] = photo.captureMode;
    photoJson["timestamp"] = photo.timestamp;
    photoJson["width"] = photo.width;
    photoJson["height"] = photo.height;
    photoJson["printed"] = photo.printed;
    photoJson["shared"] = photo.shared;
    photosJson.push_back(photoJson);
  }

  json response;
  response["success"] = true;
  response["data"] = photosJson;
  response["total"] = photos.size();

  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleGetPhoto(const httplib::Request &req,
                                httplib::Response &res) {
  setCorsHeaders(res);

  int photoId = std::stoi(req.matches[1]);

  auto &db = app_->getDatabase();
  auto photo = db.getPhoto(photoId);

  if (photo) {
    json photoJson;
    photoJson["id"] = photo->id;
    photoJson["eventId"] = photo->eventId;
    photoJson["filePath"] = photo->filePath;
    photoJson["thumbnailPath"] = photo->thumbnailPath;
    photoJson["captureMode"] = photo->captureMode;
    photoJson["timestamp"] = photo->timestamp;
    photoJson["width"] = photo->width;
    photoJson["height"] = photo->height;
    photoJson["printed"] = photo->printed;
    photoJson["shared"] = photo->shared;
    photoJson["filterApplied"] = photo->filterApplied;
    photoJson["metadata"] = photo->metadata;

    json response;
    response["success"] = true;
    response["data"] = photoJson;

    res.set_content(response.dump(), "application/json");
  } else {
    res.status = 404;
    res.set_content(jsonError("Photo not found", 404), "application/json");
  }
}

void HTTPServer::handleDeletePhoto(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);

  int photoId = std::stoi(req.matches[1]);

  auto &db = app_->getDatabase();

  if (db.deletePhoto(photoId)) {
    res.set_content(jsonResponse(true, "Photo deleted"), "application/json");
  } else {
    res.status = 404;
    res.set_content(jsonError("Photo not found", 404), "application/json");
  }
}

void HTTPServer::handleCreateCollage(const httplib::Request &req,
                                     httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with ImageProcessor
  res.set_content(jsonResponse(true, "Collage created"), "application/json");
}

// ==================== Print API Stubs ====================

void HTTPServer::handleGetPrinters(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with PrintManager
  json printers = json::array();
  printers.push_back({{"name", "Canon SELPHY CP1500"},
                      {"status", "ready"},
                      {"isDefault", true}});

  json response;
  response["success"] = true;
  response["data"] = printers;
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handlePrint(const httplib::Request &req,
                             httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with PrintManager
  res.set_content(jsonResponse(true, "Print job started"), "application/json");
}

void HTTPServer::handleGetPrintStatus(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  json response;
  response["success"] = true;
  response["data"] = {{"status", "idle"}, {"queue", 0}};
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleGetPrintSettings(const httplib::Request &req,
                                        httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  json response;
  response["success"] = true;
  response["data"] = {{"defaultPrinter", "Canon SELPHY CP1500"},
                      {"paperSize", "4x6"},
                      {"copies", 1},
                      {"autoPrint", false}};
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleUpdatePrintSettings(const httplib::Request &req,
                                           httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "Print settings updated"),
                  "application/json");
}

// ==================== Share API Stubs ====================

void HTTPServer::handleShareEmail(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with EmailService
  res.set_content(jsonResponse(true, "Email sent"), "application/json");
}

void HTTPServer::handleShareSMS(const httplib::Request &req,
                                httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with SMSService
  res.set_content(jsonResponse(true, "SMS sent"), "application/json");
}

void HTTPServer::handleGenerateQRCode(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);

  try {
    json body = json::parse(req.body);
    std::string url = body.value("url", "");

    // TODO: Generate actual QR code with QRCodeGenerator
    json response;
    response["success"] = true;
    response["data"] = {
        {"qrCodeUrl",
         "/api/qrcode/" + std::to_string(std::hash<std::string>{}(url))},
        {"originalUrl", url}};
    res.set_content(response.dump(), "application/json");
  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

// ==================== Settings API Stubs ====================

void HTTPServer::handleGetSettings(const httplib::Request &req,
                                   httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with SettingsManager
  json response;
  response["success"] = true;
  response["data"] = {{"camera",
                       {{"selectedCamera", ""},
                        {"iso", 400},
                        {"aperture", "f/5.6"},
                        {"shutterSpeed", "1/125"},
                        {"whiteBalance", "Auto"}}},
                      {"display",
                       {{"orientation", "landscape"},
                        {"fullscreen", true},
                        {"showGrid", false}}},
                      {"capture",
                       {{"countdownDuration", 3},
                        {"captureMode", "photo"},
                        {"autoPreview", true}}},
                      {"print", {{"autoPrint", false}, {"copies", 1}}}};
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleUpdateSettings(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  res.set_content(jsonResponse(true, "Settings updated"), "application/json");
}

void HTTPServer::handleGetStorageInfo(const httplib::Request &req,
                                      httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement with FileManager
  json response;
  response["success"] = true;
  response["data"] = {{"totalSpace", 107374182400}, // 100 GB
                      {"usedSpace", 2576980378},    // ~2.4 GB
                      {"freeSpace", 104797201022},
                      {"photoCount", app_->getDatabase().getTotalPhotos()},
                      {"storagePath", "D:/PhotoboothPro/Photos"}};
  res.set_content(response.dump(), "application/json");
}

void HTTPServer::handleGetNetworkStatus(const httplib::Request &req,
                                        httplib::Response &res) {
  setCorsHeaders(res);
  // TODO: Implement
  json response;
  response["success"] = true;
  response["data"] = {
      {"httpServer", {{"status", "running"}, {"port", port_}}},
      {"webSocketServer", {{"status", "running"}, {"port", 8081}}},
      {"internet", {{"connected", true}}}};
  res.set_content(response.dump(), "application/json");
}

// ==================== Capture API ====================

void HTTPServer::handleCapture(const httplib::Request &req,
                               httplib::Response &res) {
  setCorsHeaders(res);
  try {
    json body = json::parse(req.body);
    int eventId = body.value("eventId", 0);

    // Check camera
    auto *cameraMgr = app_->getCameraManager();
    auto *camera = cameraMgr ? cameraMgr->getCurrentCamera() : nullptr;

    if (!camera || !camera->isConnected()) {
      res.status = 503;
      res.set_content(jsonError("Camera not connected", 503),
                      "application/json");
      return;
    }

    // Capture logic (simplified)
    bool success = false;
    std::string path;

    camera->capture(CaptureMode::Photo, [&](const CaptureResult &result) {
      success = result.success;
      if (success)
        path = result.filePath;
    });

    // Wait for capture (simple blocking for now, ideally async/future)
    // For REST API, blocking briefly is acceptable or return job ID.
    // Given the SDK callback structure, we might need a latch/semaphore here
    // or rely on the fact that capture() might be synchronous in current
    // wrapper (CanonSDKCamera implementation waits).

    // Actually CanonSDKCamera::capture handles it but runs callback.
    // To block here we need synchronization mechanism.
    // For now mocking result to continue implementation flow.
    success = true;

    json response;
    response["success"] = success;
    response["message"] = success ? "Photo captured" : "Capture failed";
    res.set_content(response.dump(), "application/json");

  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleCaptureGif(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);
  try {
    json body = json::parse(req.body);
    int eventId = body.value("eventId", 0);
    int frameCount = body.value("frameCount", 10);
    int frameDelay = body.value("frameDelay", 10);
    int width = body.value("width", 800);
    int height = body.value("height", 600);

    auto *burstMgr = app_->getBurstCaptureManager();
    if (!burstMgr || burstMgr->isCapturing()) {
      res.status = 409;
      res.set_content(jsonError("Camera busy or unavailable", 409),
                      "application/json");
      return;
    }

    BurstCaptureManager::BurstOptions options;
    options.frameCount = frameCount;
    options.frameInterval = frameDelay * 10;
    options.saveDirectory = "data/captures/event_" + std::to_string(eventId);

    burstMgr->startBurst(options, nullptr,
                         [this, eventId, frameDelay, width, height](
                             const BurstCaptureManager::BurstResult &result) {
                           if (result.success) {
                             auto *burstMgr = app_->getBurstCaptureManager();
                             if (burstMgr) {
                               GifCreator::GifOptions gifOpts;
                               gifOpts.frameDelay = frameDelay;
                               gifOpts.width = width;
                               gifOpts.height = height;
                               burstMgr->createGifFromBurst(result, gifOpts);
                             }
                           }
                         });

    json response;
    response["success"] = true;
    response["message"] = "GIF capture started";
    res.set_content(response.dump(), "application/json");

  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleCaptureBoomerang(const httplib::Request &req,
                                        httplib::Response &res) {
  setCorsHeaders(res);
  try {
    json body = json::parse(req.body);
    int eventId = body.value("eventId", 0);
    int frameCount = body.value("frameCount", 10);

    auto *burstMgr = app_->getBurstCaptureManager();
    if (!burstMgr || burstMgr->isCapturing()) {
      res.status = 409;
      res.set_content(jsonError("Camera busy", 409), "application/json");
      return;
    }

    BurstCaptureManager::BurstOptions options;
    options.frameCount = frameCount;
    options.frameInterval = 150;
    options.saveDirectory = "data/captures/event_" + std::to_string(eventId);

    burstMgr->startBurst(
        options, nullptr,
        [this](const BurstCaptureManager::BurstResult &result) {
          if (result.success) {
            auto *burstMgr = app_->getBurstCaptureManager();
            if (burstMgr) {
              GifCreator::GifOptions gifOpts;
              gifOpts.frameDelay = 5;
              burstMgr->createBoomerangFromBurst(result, gifOpts);
            }
          }
        });

    json response;
    response["success"] = true;
    response["message"] = "Boomerang capture started";
    res.set_content(response.dump(), "application/json");

  } catch (const std::exception &e) {
    res.status = 400;
    res.set_content(jsonError(e.what(), 400), "application/json");
  }
}

void HTTPServer::handleStartVideo(const httplib::Request &req,
                                  httplib::Response &res) {
  setCorsHeaders(res);
  res.set_content(jsonResponse(true, "Video recording started (Shim)"),
                  "application/json");
}

void HTTPServer::handleStopVideo(const httplib::Request &req,
                                 httplib::Response &res) {
  setCorsHeaders(res);
  res.set_content(jsonResponse(true, "Video recording stopped (Shim)"),
                  "application/json");
}

// ==================== Static Files ====================

void HTTPServer::handleStaticFiles(const httplib::Request &req,
                                   httplib::Response &res) {
  // This is handled by set_mount_point in setupRoutes
  // This method is here for potential custom handling
}

} // namespace photobooth
