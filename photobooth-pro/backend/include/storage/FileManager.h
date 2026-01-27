#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace photobooth {

// Slot coordinate structure for layout analysis
struct SlotInfo {
    int id;
    int x;
    int y;
    int width;
    int height;
};

// Layout configuration
struct LayoutConfig {
    std::string layoutName;
    std::string layoutPath;
    int slotsCount;
    std::vector<SlotInfo> slots;
};

// Event settings
struct EventSettings {
    std::string eventId;
    std::string eventName;
    std::string createdAt;
    std::string currentLayout;
    int photoCount;
};

class FileManager {
public:
    FileManager(const std::string& baseDir = "./Photobooth_Data");
    ~FileManager();

    bool initialize();

    // Event management
    bool initializeEvent(const std::string& eventId);
    bool eventExists(const std::string& eventId);
    std::string getEventPath(const std::string& eventId);
    std::vector<std::string> listEvents();
    bool deleteEvent(const std::string& eventId);

    // Event settings
    bool saveEventSettings(const std::string& eventId, const EventSettings& settings);
    EventSettings loadEventSettings(const std::string& eventId);
    
    // Full JSON configuration
    bool saveEventFullConfig(const std::string& eventId, const std::string& jsonConfig);
    std::string loadEventFullConfig(const std::string& eventId);

    // Layout management
    std::string saveLayout(const std::string& eventId,
                          const std::vector<uint8_t>& pngData,
                          const std::string& layoutName);
    std::vector<std::string> listLayouts(const std::string& eventId);
    bool deleteLayout(const std::string& eventId, const std::string& layoutName);
    std::string getLayoutPath(const std::string& eventId, const std::string& layoutName);

    // Slot configuration (from PNG alpha analysis)
    bool saveSlotsConfig(const std::string& eventId, const LayoutConfig& config);
    LayoutConfig loadSlotsConfig(const std::string& eventId, const std::string& layoutName);

    // Directory management (legacy)
    std::string createEventDirectory(const std::string& eventName);
    bool deleteEventDirectory(const std::string& eventPath);

    // File operations
    std::string saveImage(const std::vector<uint8_t>& imageData,
                         const std::string& eventPath,
                         const std::string& filename);
    std::string saveCapturedPhoto(const std::string& eventId,
                                  const std::vector<uint8_t>& imageData,
                                  const std::string& prefix = "photo");
    bool deleteImage(const std::string& filePath);
    std::vector<uint8_t> loadImage(const std::string& filePath);

    // Captured photos
    std::vector<std::string> listCapturedPhotos(const std::string& eventId);

    // Thumbnail generation
    std::string createThumbnail(const std::string& imagePath, int maxSize = 200);

    // Export
    bool exportEvent(const std::string& eventPath, const std::string& exportPath);

    // Utilities
    std::string getBaseDirectory() const { return baseDir_; }
    bool fileExists(const std::string& path);
    long long getFileSize(const std::string& path);
    std::vector<std::string> listFiles(const std::string& directory);

private:
    std::string baseDir_;

    bool createDirectory(const std::string& path);
    bool createDirectoryTree(const std::string& path);
    std::string generateUniqueFilename(const std::string& prefix);
    std::string getCurrentTimestamp();
    std::string sanitizeFilename(const std::string& name);
};

} // namespace photobooth
