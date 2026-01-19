#include "storage/DatabaseManager.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sqlite3.h>
#include <sstream>

namespace photobooth {

DatabaseManager::DatabaseManager(const std::string &dbPath)
    : dbPath_(dbPath), db_(nullptr), initialized_(false) {}

DatabaseManager::~DatabaseManager() { close(); }

bool DatabaseManager::initialize() {
  if (initialized_) {
    return true;
  }

  sqlite3 *db = nullptr;
  int rc = sqlite3_open(dbPath_.c_str(), &db);
  if (rc != SQLITE_OK) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }

  db_ = db;

  if (!createTables()) {
    std::cerr << "Failed to create tables" << std::endl;
    close();
    return false;
  }

  initialized_ = true;
  std::cout << "Database initialized successfully: " << dbPath_ << std::endl;
  return true;
}

void DatabaseManager::close() {
  if (db_) {
    sqlite3_close(static_cast<sqlite3 *>(db_));
    db_ = nullptr;
  }
  initialized_ = false;
}

bool DatabaseManager::createTables() {
  const char *eventsSql = R"(
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            location TEXT,
            event_date TEXT,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            photo_count INTEGER DEFAULT 0,
            thumbnail_path TEXT,
            status TEXT DEFAULT 'active'
        );
    )";

  // Lazy migration for existing tables
  executeSQL("ALTER TABLE events ADD COLUMN location TEXT;");
  executeSQL("ALTER TABLE events ADD COLUMN event_date TEXT;");

  const char *eventConfigSql = R"(
        CREATE TABLE IF NOT EXISTS event_configs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id INTEGER NOT NULL UNIQUE,
            start_screen_image TEXT,
            capture_mode TEXT DEFAULT 'photo',
            photo_enabled INTEGER DEFAULT 1,
            gif_enabled INTEGER DEFAULT 1,
            boomerang_enabled INTEGER DEFAULT 0,
            video_enabled INTEGER DEFAULT 1,
            effects_config TEXT,
            props_config TEXT,
            beauty_filter_config TEXT,
            watermark_config TEXT,
            post_process_config TEXT,
            countdown_seconds INTEGER DEFAULT 3,
            photo_count INTEGER DEFAULT 4,
            layout_template TEXT,
            FOREIGN KEY (event_id) REFERENCES events(id) ON DELETE CASCADE
        );
    )";

  const char *photosSql = R"(
        CREATE TABLE IF NOT EXISTS photos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id INTEGER NOT NULL,
            file_path TEXT NOT NULL,
            thumbnail_path TEXT,
            capture_mode TEXT,
            timestamp TEXT NOT NULL,
            width INTEGER,
            height INTEGER,
            printed INTEGER DEFAULT 0,
            shared INTEGER DEFAULT 0,
            filter_applied TEXT,
            metadata TEXT,
            FOREIGN KEY (event_id) REFERENCES events(id) ON DELETE CASCADE
        );
    )";

  const char *indexSql = R"(
        CREATE INDEX IF NOT EXISTS idx_photos_event_id ON photos(event_id);
        CREATE INDEX IF NOT EXISTS idx_events_status ON events(status);
        CREATE INDEX IF NOT EXISTS idx_events_created_at ON events(created_at);
    )";

  if (!executeSQL(eventsSql))
    return false;
  if (!executeSQL(eventConfigSql))
    return false;
  if (!executeSQL(photosSql))
    return false;
  if (!executeSQL(indexSql))
    return false;

  return true;
}

bool DatabaseManager::executeSQL(const std::string &sql) {
  char *errMsg = nullptr;
  int rc = sqlite3_exec(static_cast<sqlite3 *>(db_), sql.c_str(), nullptr,
                        nullptr, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

std::string DatabaseManager::escapeString(const std::string &str) {
  std::string result;
  for (char c : str) {
    if (c == '\'') {
      result += "''";
    } else {
      result += c;
    }
  }
  return result;
}

std::string DatabaseManager::getCurrentTimestamp() {
  auto now = std::time(nullptr);
  auto tm = *std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

// ==================== Event Management ====================

int DatabaseManager::createEvent(const std::string &name,
                                 const std::string &location,
                                 const std::string &eventDate) {
  std::string timestamp = getCurrentTimestamp();
  std::ostringstream sql;
  sql << "INSERT INTO events (name, location, event_date, created_at, "
         "updated_at, status) VALUES ('"
      << escapeString(name) << "', '" << escapeString(location) << "', '"
      << escapeString(eventDate) << "', '" << timestamp << "', '" << timestamp
      << "', 'active');";

  if (!executeSQL(sql.str())) {
    return -1;
  }

  int eventId =
      static_cast<int>(sqlite3_last_insert_rowid(static_cast<sqlite3 *>(db_)));

  // Create default config for the event
  EventConfig defaultConfig;
  defaultConfig.eventId = eventId;
  defaultConfig.captureMode = "photo";
  defaultConfig.photoEnabled = true;
  defaultConfig.gifEnabled = true;
  defaultConfig.boomerangEnabled = false;
  defaultConfig.videoEnabled = true;
  defaultConfig.countdownSeconds = 3;
  defaultConfig.photoCount = 4;
  saveEventConfig(defaultConfig);

  return eventId;
}

bool DatabaseManager::updateEvent(int eventId, const std::string &name,
                                  const std::string &location,
                                  const std::string &eventDate) {
  std::string timestamp = getCurrentTimestamp();
  std::ostringstream sql;
  sql << "UPDATE events SET name = '" << escapeString(name) << "', location = '"
      << escapeString(location) << "', event_date = '"
      << escapeString(eventDate) << "', updated_at = '" << timestamp
      << "' WHERE id = " << eventId << ";";
  return executeSQL(sql.str());
}

bool DatabaseManager::deleteEvent(int eventId) {
  // Delete photos first (cascade should handle this, but being explicit)
  deletePhotosForEvent(eventId);

  std::ostringstream sql;
  sql << "DELETE FROM event_configs WHERE event_id = " << eventId << ";";
  executeSQL(sql.str());

  sql.str("");
  sql << "DELETE FROM events WHERE id = " << eventId << ";";
  return executeSQL(sql.str());
}

bool DatabaseManager::duplicateEvent(int eventId, const std::string &newName) {
  auto originalEvent = getEvent(eventId);
  if (!originalEvent) {
    return false;
  }

  int newEventId = createEvent(newName);
  if (newEventId < 0) {
    return false;
  }

  // Copy config
  auto config = getEventConfig(eventId);
  if (config) {
    config->eventId = newEventId;
    updateEventConfig(newEventId, *config);
  }

  return true;
}

std::vector<Event> DatabaseManager::getAllEvents(const std::string &sortBy,
                                                 const std::string &order,
                                                 const std::string &filter) {
  std::vector<Event> events;

  std::ostringstream sql;
  sql << "SELECT e.id, e.name, e.created_at, e.updated_at, e.photo_count, "
      << "e.thumbnail_path, e.status, e.location, e.event_date FROM events e";

  if (!filter.empty()) {
    sql << " WHERE e.status = '" << escapeString(filter) << "'";
  }

  std::string sortColumn = "created_at";
  if (sortBy == "name")
    sortColumn = "name";
  else if (sortBy == "photoCount")
    sortColumn = "photo_count";
  else if (sortBy == "updatedAt")
    sortColumn = "updated_at";

  sql << " ORDER BY e." << sortColumn << " "
      << (order == "ASC" ? "ASC" : "DESC") << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: "
              << sqlite3_errmsg(static_cast<sqlite3 *>(db_)) << std::endl;
    return events;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    Event event;
    event.id = sqlite3_column_int(stmt, 0);
    event.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    event.createdAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    event.updatedAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    event.photoCount = sqlite3_column_int(stmt, 4);

    const char *thumbnail =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    event.thumbnailPath = thumbnail ? thumbnail : "";

    const char *status =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
    event.status = status ? status : "active";

    const char *location =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
    event.location = location ? location : "";

    const char *eventDate =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
    event.eventDate = eventDate ? eventDate : "";

    // Load config
    auto config = getEventConfig(event.id);
    if (config) {
      event.config = *config;
    }

    events.push_back(event);
  }

  sqlite3_finalize(stmt);
  return events;
}

std::optional<Event> DatabaseManager::getEvent(int eventId) {
  std::ostringstream sql;
  sql << "SELECT id, name, created_at, updated_at, photo_count, "
         "thumbnail_path, status, location, event_date "
      << "FROM events WHERE id = " << eventId << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return std::nullopt;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    Event event;
    event.id = sqlite3_column_int(stmt, 0);
    event.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    event.createdAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    event.updatedAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    event.photoCount = sqlite3_column_int(stmt, 4);

    const char *thumbnail =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    event.thumbnailPath = thumbnail ? thumbnail : "";

    const char *status =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
    event.status = status ? status : "active";

    const char *location =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
    event.location = location ? location : "";

    const char *eventDate =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
    event.eventDate = eventDate ? eventDate : "";

    sqlite3_finalize(stmt);

    // Load config
    auto config = getEventConfig(event.id);
    if (config) {
      event.config = *config;
    }

    return event;
  }

  sqlite3_finalize(stmt);
  return std::nullopt;
}

bool DatabaseManager::eventExists(int eventId) {
  std::ostringstream sql;
  sql << "SELECT COUNT(*) FROM events WHERE id = " << eventId << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return false;
  }

  bool exists = false;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    exists = sqlite3_column_int(stmt, 0) > 0;
  }

  sqlite3_finalize(stmt);
  return exists;
}

std::vector<Event> DatabaseManager::searchEvents(const std::string &query) {
  std::vector<Event> events;

  std::ostringstream sql;
  sql << "SELECT id, name, created_at, updated_at, photo_count, "
         "thumbnail_path, status, location, event_date "
      << "FROM events WHERE name LIKE '%" << escapeString(query) << "%' "
      << "ORDER BY created_at DESC;";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return events;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    Event event;
    event.id = sqlite3_column_int(stmt, 0);
    event.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    event.createdAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    event.updatedAt =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    event.photoCount = sqlite3_column_int(stmt, 4);

    const char *thumbnail =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    event.thumbnailPath = thumbnail ? thumbnail : "";

    const char *status =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
    event.status = status ? status : "active";

    const char *location =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
    event.location = location ? location : "";

    const char *eventDate =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
    event.eventDate = eventDate ? eventDate : "";

    events.push_back(event);
  }

  sqlite3_finalize(stmt);
  return events;
}

// ==================== Event Config Management ====================

bool DatabaseManager::saveEventConfig(const EventConfig &config) {
  std::ostringstream sql;
  sql << "INSERT OR REPLACE INTO event_configs ("
      << "event_id, start_screen_image, capture_mode, photo_enabled, "
         "gif_enabled, "
      << "boomerang_enabled, video_enabled, effects_config, props_config, "
      << "beauty_filter_config, watermark_config, post_process_config, "
      << "countdown_seconds, photo_count, layout_template) VALUES ("
      << config.eventId << ", '" << escapeString(config.startScreenImage)
      << "', '" << escapeString(config.captureMode) << "', "
      << (config.photoEnabled ? 1 : 0) << ", " << (config.gifEnabled ? 1 : 0)
      << ", " << (config.boomerangEnabled ? 1 : 0) << ", "
      << (config.videoEnabled ? 1 : 0) << ", '"
      << escapeString(config.effectsConfig) << "', '"
      << escapeString(config.propsConfig) << "', '"
      << escapeString(config.beautyFilterConfig) << "', '"
      << escapeString(config.watermarkConfig) << "', '"
      << escapeString(config.postProcessConfig) << "', "
      << config.countdownSeconds << ", " << config.photoCount << ", '"
      << escapeString(config.layoutTemplate) << "');";

  return executeSQL(sql.str());
}

std::optional<EventConfig> DatabaseManager::getEventConfig(int eventId) {
  std::ostringstream sql;
  sql << "SELECT event_id, start_screen_image, capture_mode, photo_enabled, "
         "gif_enabled, "
      << "boomerang_enabled, video_enabled, effects_config, props_config, "
      << "beauty_filter_config, watermark_config, post_process_config, "
      << "countdown_seconds, photo_count, layout_template "
      << "FROM event_configs WHERE event_id = " << eventId << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return std::nullopt;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    EventConfig config;
    config.eventId = sqlite3_column_int(stmt, 0);

    const char *startScreen =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    config.startScreenImage = startScreen ? startScreen : "";

    const char *captureMode =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    config.captureMode = captureMode ? captureMode : "photo";

    config.photoEnabled = sqlite3_column_int(stmt, 3) != 0;
    config.gifEnabled = sqlite3_column_int(stmt, 4) != 0;
    config.boomerangEnabled = sqlite3_column_int(stmt, 5) != 0;
    config.videoEnabled = sqlite3_column_int(stmt, 6) != 0;

    const char *effects =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
    config.effectsConfig = effects ? effects : "";

    const char *props =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
    config.propsConfig = props ? props : "";

    const char *beauty =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9));
    config.beautyFilterConfig = beauty ? beauty : "";

    const char *watermark =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
    config.watermarkConfig = watermark ? watermark : "";

    const char *postProcess =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
    config.postProcessConfig = postProcess ? postProcess : "";

    config.countdownSeconds = sqlite3_column_int(stmt, 12);
    config.photoCount = sqlite3_column_int(stmt, 13);

    const char *layout =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 14));
    config.layoutTemplate = layout ? layout : "";

    sqlite3_finalize(stmt);
    return config;
  }

  sqlite3_finalize(stmt);
  return std::nullopt;
}

bool DatabaseManager::updateEventConfig(int eventId,
                                        const EventConfig &config) {
  EventConfig updatedConfig = config;
  updatedConfig.eventId = eventId;
  return saveEventConfig(updatedConfig);
}

// ==================== Photo Management ====================

int DatabaseManager::savePhoto(const Photo &photo) {
  std::ostringstream sql;
  sql << "INSERT INTO photos (event_id, file_path, thumbnail_path, "
         "capture_mode, "
      << "timestamp, width, height, printed, shared, filter_applied, metadata) "
         "VALUES ("
      << photo.eventId << ", '" << escapeString(photo.filePath) << "', '"
      << escapeString(photo.thumbnailPath) << "', '"
      << escapeString(photo.captureMode) << "', '"
      << escapeString(photo.timestamp) << "', " << photo.width << ", "
      << photo.height << ", " << (photo.printed ? 1 : 0) << ", "
      << (photo.shared ? 1 : 0) << ", '" << escapeString(photo.filterApplied)
      << "', '" << escapeString(photo.metadata) << "');";

  if (!executeSQL(sql.str())) {
    return -1;
  }

  int photoId =
      static_cast<int>(sqlite3_last_insert_rowid(static_cast<sqlite3 *>(db_)));

  // Update photo count for event
  std::ostringstream updateSql;
  updateSql << "UPDATE events SET photo_count = photo_count + 1, updated_at = '"
            << getCurrentTimestamp() << "' WHERE id = " << photo.eventId << ";";
  executeSQL(updateSql.str());

  return photoId;
}

bool DatabaseManager::deletePhoto(int photoId) {
  // Get event_id first
  auto photo = getPhoto(photoId);
  if (!photo) {
    return false;
  }

  std::ostringstream sql;
  sql << "DELETE FROM photos WHERE id = " << photoId << ";";

  if (!executeSQL(sql.str())) {
    return false;
  }

  // Update photo count for event
  std::ostringstream updateSql;
  updateSql << "UPDATE events SET photo_count = photo_count - 1, updated_at = '"
            << getCurrentTimestamp() << "' WHERE id = " << photo->eventId
            << ";";
  executeSQL(updateSql.str());

  return true;
}

bool DatabaseManager::deletePhotosForEvent(int eventId) {
  std::ostringstream sql;
  sql << "DELETE FROM photos WHERE event_id = " << eventId << ";";
  return executeSQL(sql.str());
}

std::vector<Photo> DatabaseManager::getPhotosForEvent(int eventId, int limit,
                                                      int offset) {
  std::vector<Photo> photos;

  std::ostringstream sql;
  sql << "SELECT id, event_id, file_path, thumbnail_path, capture_mode, "
         "timestamp, "
      << "width, height, printed, shared, filter_applied, metadata "
      << "FROM photos WHERE event_id = " << eventId
      << " ORDER BY timestamp DESC";

  if (limit > 0) {
    sql << " LIMIT " << limit;
    if (offset > 0) {
      sql << " OFFSET " << offset;
    }
  }
  sql << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return photos;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    Photo photo;
    photo.id = sqlite3_column_int(stmt, 0);
    photo.eventId = sqlite3_column_int(stmt, 1);
    photo.filePath =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

    const char *thumbnail =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    photo.thumbnailPath = thumbnail ? thumbnail : "";

    const char *captureMode =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
    photo.captureMode = captureMode ? captureMode : "";

    photo.timestamp =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    photo.width = sqlite3_column_int(stmt, 6);
    photo.height = sqlite3_column_int(stmt, 7);
    photo.printed = sqlite3_column_int(stmt, 8) != 0;
    photo.shared = sqlite3_column_int(stmt, 9) != 0;

    const char *filter =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
    photo.filterApplied = filter ? filter : "";

    const char *metadata =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
    photo.metadata = metadata ? metadata : "";

    photos.push_back(photo);
  }

  sqlite3_finalize(stmt);
  return photos;
}

std::optional<Photo> DatabaseManager::getPhoto(int photoId) {
  std::ostringstream sql;
  sql << "SELECT id, event_id, file_path, thumbnail_path, capture_mode, "
         "timestamp, "
      << "width, height, printed, shared, filter_applied, metadata "
      << "FROM photos WHERE id = " << photoId << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return std::nullopt;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    Photo photo;
    photo.id = sqlite3_column_int(stmt, 0);
    photo.eventId = sqlite3_column_int(stmt, 1);
    photo.filePath =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

    const char *thumbnail =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    photo.thumbnailPath = thumbnail ? thumbnail : "";

    const char *captureMode =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
    photo.captureMode = captureMode ? captureMode : "";

    photo.timestamp =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
    photo.width = sqlite3_column_int(stmt, 6);
    photo.height = sqlite3_column_int(stmt, 7);
    photo.printed = sqlite3_column_int(stmt, 8) != 0;
    photo.shared = sqlite3_column_int(stmt, 9) != 0;

    const char *filter =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
    photo.filterApplied = filter ? filter : "";

    const char *metadata =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
    photo.metadata = metadata ? metadata : "";

    sqlite3_finalize(stmt);
    return photo;
  }

  sqlite3_finalize(stmt);
  return std::nullopt;
}

bool DatabaseManager::markAsPrinted(int photoId) {
  std::ostringstream sql;
  sql << "UPDATE photos SET printed = 1 WHERE id = " << photoId << ";";
  return executeSQL(sql.str());
}

bool DatabaseManager::markAsShared(int photoId) {
  std::ostringstream sql;
  sql << "UPDATE photos SET shared = 1 WHERE id = " << photoId << ";";
  return executeSQL(sql.str());
}

bool DatabaseManager::updatePhotoThumbnail(int photoId,
                                           const std::string &thumbnailPath) {
  std::ostringstream sql;
  sql << "UPDATE photos SET thumbnail_path = '" << escapeString(thumbnailPath)
      << "' WHERE id = " << photoId << ";";
  return executeSQL(sql.str());
}

// ==================== Statistics ====================

int DatabaseManager::getTotalPhotos() {
  const char *sql = "SELECT COUNT(*) FROM photos;";

  sqlite3_stmt *stmt;
  int rc =
      sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;
  }

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count;
}

int DatabaseManager::getTotalPhotosForEvent(int eventId) {
  std::ostringstream sql;
  sql << "SELECT COUNT(*) FROM photos WHERE event_id = " << eventId << ";";

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql.str().c_str(),
                              -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;
  }

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count;
}

int DatabaseManager::getTotalPrints() {
  const char *sql = "SELECT COUNT(*) FROM photos WHERE printed = 1;";

  sqlite3_stmt *stmt;
  int rc =
      sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;
  }

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count;
}

int DatabaseManager::getTotalShares() {
  const char *sql = "SELECT COUNT(*) FROM photos WHERE shared = 1;";

  sqlite3_stmt *stmt;
  int rc =
      sqlite3_prepare_v2(static_cast<sqlite3 *>(db_), sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;
  }

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count;
}

} // namespace photobooth
