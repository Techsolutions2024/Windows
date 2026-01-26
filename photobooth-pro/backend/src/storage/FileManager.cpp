#include "storage/FileManager.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace photobooth {

FileManager::FileManager(const std::string& baseDir) : baseDir_(baseDir) {}

FileManager::~FileManager() {}

bool FileManager::initialize() {
    // Create base directory structure
    if (!createDirectory(baseDir_)) {
        return false;
    }
    if (!createDirectory(baseDir_ + "/events")) {
        return false;
    }
    return true;
}

// ============================================
// Directory Management
// ============================================

std::string FileManager::createEventDirectory(const std::string& eventId) {
    std::string eventPath = baseDir_ + "/events/" + eventId;

    // Create main event folder and subfolders
    createDirectory(eventPath);
    createDirectory(eventPath + "/config");
    createDirectory(eventPath + "/layouts");
    createDirectory(eventPath + "/captured_photos");

    return eventPath;
}

bool FileManager::deleteEventDirectory(const std::string& eventId) {
    std::string eventPath = baseDir_ + "/events/" + eventId;
    try {
        if (fs::exists(eventPath)) {
            fs::remove_all(eventPath);
            return true;
        }
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

std::string FileManager::getEventPath(const std::string& eventId) const {
    return baseDir_ + "/events/" + eventId;
}

// ============================================
// Event Configuration (JSON)
// ============================================

bool FileManager::saveEventConfig(const std::string& eventId, const json& config) {
    std::string configPath = baseDir_ + "/events/" + eventId + "/config/event_settings.json";

    // Ensure directory exists
    createDirectory(baseDir_ + "/events/" + eventId + "/config");

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    try {
        file << config.dump(2);  // Pretty print with 2-space indent
        return file.good();
    } catch (const std::exception&) {
        return false;
    }
}

json FileManager::loadEventConfig(const std::string& eventId) {
    std::string configPath = baseDir_ + "/events/" + eventId + "/config/event_settings.json";

    std::ifstream file(configPath);
    if (!file.is_open()) {
        return json::object();
    }

    try {
        return json::parse(file);
    } catch (const std::exception&) {
        return json::object();
    }
}

bool FileManager::eventConfigExists(const std::string& eventId) const {
    std::string configPath = baseDir_ + "/events/" + eventId + "/config/event_settings.json";
    return fileExists(configPath);
}

std::string FileManager::getEventConfigPath(const std::string& eventId) const {
    return baseDir_ + "/events/" + eventId + "/config/event_settings.json";
}

// ============================================
// Image Operations
// ============================================

std::string FileManager::saveImage(const std::vector<uint8_t>& imageData,
                                   const std::string& eventId,
                                   const std::string& filename) {
    std::string photoPath = baseDir_ + "/events/" + eventId + "/captured_photos/";
    createDirectory(photoPath);

    std::string fullPath = photoPath + filename;

    std::ofstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    file.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    return file.good() ? fullPath : "";
}

std::string FileManager::saveCapturedPhoto(const std::vector<uint8_t>& imageData,
                                           const std::string& eventId) {
    std::string filename = generateUniqueFilename("photo") + ".jpg";
    return saveImage(imageData, eventId, filename);
}

bool FileManager::deleteImage(const std::string& filePath) {
    try {
        if (fs::exists(filePath)) {
            return fs::remove(filePath);
        }
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<uint8_t> FileManager::loadImage(const std::string& filePath) {
    std::vector<uint8_t> data;

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return data;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        data.clear();
    }

    return data;
}

// ============================================
// Layout Template Management
// ============================================

bool FileManager::saveLayoutTemplate(const std::string& eventId,
                                     const std::vector<uint8_t>& imageData,
                                     const std::string& filename) {
    std::string layoutPath = baseDir_ + "/events/" + eventId + "/layouts/";
    createDirectory(layoutPath);

    std::string fullPath = layoutPath + filename;

    std::ofstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    return file.good();
}

std::string FileManager::getLayoutPath(const std::string& eventId, const std::string& filename) const {
    return baseDir_ + "/events/" + eventId + "/layouts/" + filename;
}

std::vector<std::string> FileManager::listLayouts(const std::string& eventId) {
    std::vector<std::string> layouts;
    std::string layoutDir = baseDir_ + "/events/" + eventId + "/layouts";

    try {
        if (fs::exists(layoutDir) && fs::is_directory(layoutDir)) {
            for (const auto& entry : fs::directory_iterator(layoutDir)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        layouts.push_back(entry.path().filename().string());
                    }
                }
            }
        }
    } catch (const std::exception&) {
        // Return empty list on error
    }

    return layouts;
}

bool FileManager::saveLayoutSlotsConfig(const std::string& eventId, const json& slotsConfig) {
    std::string configPath = baseDir_ + "/events/" + eventId + "/layouts/slots_config.json";

    createDirectory(baseDir_ + "/events/" + eventId + "/layouts");

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    try {
        file << slotsConfig.dump(2);
        return file.good();
    } catch (const std::exception&) {
        return false;
    }
}

json FileManager::loadLayoutSlotsConfig(const std::string& eventId) {
    std::string configPath = baseDir_ + "/events/" + eventId + "/layouts/slots_config.json";

    std::ifstream file(configPath);
    if (!file.is_open()) {
        return json::object();
    }

    try {
        return json::parse(file);
    } catch (const std::exception&) {
        return json::object();
    }
}

// ============================================
// Thumbnail Generation
// ============================================

std::string FileManager::createThumbnail(const std::string& imagePath, int maxSize) {
    // TODO: Implement using OpenCV if available
    // For now, return empty string (not implemented)
    return "";
}

// ============================================
// Export
// ============================================

bool FileManager::exportEvent(const std::string& eventId, const std::string& exportPath) {
    std::string eventPath = baseDir_ + "/events/" + eventId;

    try {
        if (!fs::exists(eventPath)) {
            return false;
        }

        // Create export directory
        createDirectory(exportPath);

        // Copy all files
        fs::copy(eventPath, exportPath,
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// ============================================
// Utilities
// ============================================

bool FileManager::fileExists(const std::string& path) const {
    try {
        return fs::exists(path);
    } catch (const std::exception&) {
        return false;
    }
}

long long FileManager::getFileSize(const std::string& path) const {
    try {
        if (fs::exists(path)) {
            return static_cast<long long>(fs::file_size(path));
        }
        return -1;
    } catch (const std::exception&) {
        return -1;
    }
}

std::vector<std::string> FileManager::listFiles(const std::string& directory) {
    std::vector<std::string> files;

    try {
        if (fs::exists(directory) && fs::is_directory(directory)) {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception&) {
        // Return empty list on error
    }

    return files;
}

std::vector<std::string> FileManager::listEventIds() {
    std::vector<std::string> eventIds;
    std::string eventsDir = baseDir_ + "/events";

    try {
        if (fs::exists(eventsDir) && fs::is_directory(eventsDir)) {
            for (const auto& entry : fs::directory_iterator(eventsDir)) {
                if (entry.is_directory()) {
                    eventIds.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception&) {
        // Return empty list on error
    }

    return eventIds;
}

// ============================================
// Private Methods
// ============================================

bool FileManager::createDirectory(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            return fs::create_directories(path);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string FileManager::generateUniqueFilename(const std::string& prefix) {
    return prefix + "_" + generateTimestamp();
}

std::string FileManager::generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
        << "_" << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace photobooth
