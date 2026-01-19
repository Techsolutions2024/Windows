#pragma once

#include <string>
#include <vector>
#include <memory>

namespace photobooth {

struct Event {
    int id;
    std::string name;
    std::string createdAt;
    int photoCount;
    std::string thumbnailPath;
};

struct Photo {
    int id;
    int eventId;
    std::string filePath;
    std::string captureMode;
    std::string timestamp;
    int width;
    int height;
    bool printed;
    bool shared;
};

class DatabaseManager {
public:
    DatabaseManager(const std::string& dbPath = "photobooth.db");
    ~DatabaseManager();

    bool initialize();
    void close();

    // Event management
    int createEvent(const std::string& name);
    bool deleteEvent(int eventId);
    bool renameEvent(int eventId, const std::string& newName);
    std::vector<Event> getAllEvents();
    Event getEvent(int eventId);

    // Photo management
    int savePhoto(const Photo& photo);
    bool deletePhoto(int photoId);
    std::vector<Photo> getPhotosForEvent(int eventId);
    Photo getPhoto(int photoId);
    bool markAsPrinted(int photoId);
    bool markAsShared(int photoId);

    // Statistics
    int getTotalPhotos();
    int getTotalPrints();
    int getTotalShares();

private:
    std::string dbPath_;
    void* db_; // sqlite3*
    
    bool executeSQL(const std::string& sql);
    bool createTables();
};

} // namespace photobooth
