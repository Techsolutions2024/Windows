#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace photobooth {

class FileManager {
public:
    FileManager(const std::string& baseDir = "Photobooth_Data");
    ~FileManager();

    bool initialize();

    // Directory management
    std::string createEventDirectory(const std::string& eventId);
    bool deleteEventDirectory(const std::string& eventId);
    std::string getEventPath(const std::string& eventId) const;

    // Event configuration (JSON)
    bool saveEventConfig(const std::string& eventId, const nlohmann::json& config);
    nlohmann::json loadEventConfig(const std::string& eventId);
    bool eventConfigExists(const std::string& eventId) const;
    std::string getEventConfigPath(const std::string& eventId) const;

    // Image operations
    std::string saveImage(const std::vector<uint8_t>& imageData,
                         const std::string& eventId,
                         const std::string& filename);
    std::string saveCapturedPhoto(const std::vector<uint8_t>& imageData,
                                  const std::string& eventId);
    bool deleteImage(const std::string& filePath);
    std::vector<uint8_t> loadImage(const std::string& filePath);

    // Layout template management
    bool saveLayoutTemplate(const std::string& eventId,
                           const std::vector<uint8_t>& imageData,
                           const std::string& filename);
    std::string getLayoutPath(const std::string& eventId, const std::string& filename) const;
    std::vector<std::string> listLayouts(const std::string& eventId);
    bool saveLayoutSlotsConfig(const std::string& eventId, const nlohmann::json& slotsConfig);
    nlohmann::json loadLayoutSlotsConfig(const std::string& eventId);

    // Thumbnail generation
    std::string createThumbnail(const std::string& imagePath, int maxSize = 200);

    // Export
    bool exportEvent(const std::string& eventId, const std::string& exportPath);

    // Utilities
    std::string getBaseDirectory() const { return baseDir_; }
    void setBaseDirectory(const std::string& dir) { baseDir_ = dir; }
    bool fileExists(const std::string& path) const;
    long long getFileSize(const std::string& path) const;
    std::vector<std::string> listFiles(const std::string& directory);
    std::vector<std::string> listEventIds();

private:
    std::string baseDir_;

    bool createDirectory(const std::string& path);
    std::string generateUniqueFilename(const std::string& prefix);
    std::string generateTimestamp();
};

} // namespace photobooth
