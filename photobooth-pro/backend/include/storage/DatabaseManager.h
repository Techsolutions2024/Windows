#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace photobooth {

struct EventConfig {
  int eventId;
  std::string startScreenImage;
  std::string captureMode; // "photo", "gif", "boomerang", "video"
  bool photoEnabled;
  bool gifEnabled;
  bool boomerangEnabled;
  bool videoEnabled;
  std::string effectsConfig;      // JSON string
  std::string propsConfig;        // JSON string
  std::string beautyFilterConfig; // JSON string
  std::string watermarkConfig;    // JSON string
  std::string postProcessConfig;  // JSON string
  std::string cameraSettings;     // JSON string
  std::string cameraSource;       // "canon", "webcam", or specific camera name
  int webcamIndex;                // Webcam device index (0, 1, 2, etc.)
  int countdownSeconds;
  int photoCount;             // Number of photos per session
  std::string layoutTemplate; // Layout template name
};

struct Event {
  int id;
  std::string name;
  std::string location;
  std::string eventDate;
  std::string createdAt;
  std::string updatedAt;
  int photoCount;
  std::string thumbnailPath;
  std::string status; // "active", "archived"
  EventConfig config;
};

struct Photo {
  int id;
  int eventId;
  std::string filePath;
  std::string thumbnailPath;
  std::string captureMode;
  std::string timestamp;
  int width;
  int height;
  bool printed;
  bool shared;
  std::string filterApplied;
  std::string metadata; // JSON string for extra data
};

class DatabaseManager {
public:
  DatabaseManager(const std::string &dbPath = "photobooth.db");
  ~DatabaseManager();

  bool initialize();
  void close();
  bool isInitialized() const { return initialized_; }

  // Event management
  int createEvent(const std::string &name, const std::string &location = "",
                  const std::string &eventDate = "");
  bool updateEvent(int eventId, const std::string &name,
                   const std::string &location = "",
                   const std::string &eventDate = "");
  bool deleteEvent(int eventId);
  bool duplicateEvent(int eventId, const std::string &newName);
  std::vector<Event> getAllEvents(const std::string &sortBy = "createdAt",
                                  const std::string &order = "DESC",
                                  const std::string &filter = "");
  std::optional<Event> getEvent(int eventId);
  bool eventExists(int eventId);

  // Event config management
  bool saveEventConfig(const EventConfig &config);
  std::optional<EventConfig> getEventConfig(int eventId);
  bool updateEventConfig(int eventId, const EventConfig &config);

  // Photo management
  int savePhoto(const Photo &photo);
  bool deletePhoto(int photoId);
  bool deletePhotosForEvent(int eventId);
  std::vector<Photo> getPhotosForEvent(int eventId, int limit = 0,
                                       int offset = 0);
  std::optional<Photo> getPhoto(int photoId);
  bool markAsPrinted(int photoId);
  bool markAsShared(int photoId);
  bool updatePhotoThumbnail(int photoId, const std::string &thumbnailPath);

  // Statistics
  int getTotalPhotos();
  int getTotalPhotosForEvent(int eventId);
  int getTotalPrints();
  int getTotalShares();

  // Search
  std::vector<Event> searchEvents(const std::string &query);

private:
  std::string dbPath_;
  void *db_; // sqlite3*
  bool initialized_;

  bool executeSQL(const std::string &sql);
  bool createTables();
  std::string escapeString(const std::string &str);
  std::string getCurrentTimestamp();
};

} // namespace photobooth
