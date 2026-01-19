#pragma once

#include <string>
#include <vector>

namespace photobooth {

class FileManager {
public:
    FileManager(const std::string& baseDir = "photos");
    ~FileManager();

    bool initialize();
    
    // Directory management
    std::string createEventDirectory(const std::string& eventName);
    bool deleteEventDirectory(const std::string& eventPath);
    
    // File operations
    std::string saveImage(const std::vector<uint8_t>& imageData, 
                         const std::string& eventPath,
                         const std::string& filename);
    bool deleteImage(const std::string& filePath);
    std::vector<uint8_t> loadImage(const std::string& filePath);
    
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
    std::string generateUniqueFilename(const std::string& prefix);
};

} // namespace photobooth
