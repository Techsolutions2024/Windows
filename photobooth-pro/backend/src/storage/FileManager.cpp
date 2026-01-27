#include "storage/FileManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEPARATOR "\\"
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#define PATH_SEPARATOR "/"
#endif

namespace photobooth {

FileManager::FileManager(const std::string& baseDir) : baseDir_(baseDir) {}

FileManager::~FileManager() {}

bool FileManager::initialize() {
    return createDirectoryTree(baseDir_);
}

bool FileManager::createDirectory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool FileManager::createDirectoryTree(const std::string& path) {
    std::string currentPath;
    std::stringstream ss(path);
    std::string segment;

#ifdef _WIN32
    // Handle drive letter (e.g., "C:")
    if (path.length() >= 2 && path[1] == ':') {
        currentPath = path.substr(0, 2);
        ss.str(path.substr(2));
    }
#endif

    char delimiter = (path.find('/') != std::string::npos) ? '/' : '\\';

    while (std::getline(ss, segment, delimiter)) {
        if (segment.empty() || segment == ".") continue;
        if (!currentPath.empty() && currentPath.back() != '/' && currentPath.back() != '\\') {
            currentPath += PATH_SEPARATOR;
        }
        currentPath += segment;

        if (!fileExists(currentPath)) {
            if (!createDirectory(currentPath)) {
                std::cerr << "Failed to create directory: " << currentPath << std::endl;
                return false;
            }
        }
    }
    return true;
}

std::string FileManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

std::string FileManager::sanitizeFilename(const std::string& name) {
    std::string result;
    for (char c : name) {
        if (std::isalnum(c) || c == '_' || c == '-' || c == '.') {
            result += c;
        } else if (c == ' ') {
            result += '_';
        }
    }
    return result;
}

std::string FileManager::getEventPath(const std::string& eventId) {
    return baseDir_ + PATH_SEPARATOR "events" PATH_SEPARATOR + eventId;
}

bool FileManager::eventExists(const std::string& eventId) {
    return fileExists(getEventPath(eventId));
}

bool FileManager::initializeEvent(const std::string& eventId) {
    std::string eventPath = getEventPath(eventId);

    // Create event directory tree
    std::string layoutsPath = eventPath + PATH_SEPARATOR "layouts";
    std::string capturedPath = eventPath + PATH_SEPARATOR "captured_photos";
    std::string configPath = eventPath + PATH_SEPARATOR "config";

    if (!createDirectoryTree(layoutsPath)) return false;
    if (!createDirectoryTree(capturedPath)) return false;
    if (!createDirectoryTree(configPath)) return false;

    // Create default event_settings.json if not exists
    std::string settingsPath = eventPath + PATH_SEPARATOR "event_settings.json";
    if (!fileExists(settingsPath)) {
        EventSettings settings;
        settings.eventId = eventId;
        settings.eventName = "Event " + eventId;
        settings.createdAt = getCurrentTimestamp();
        settings.currentLayout = "";
        settings.photoCount = 0;
        saveEventSettings(eventId, settings);
    }

    std::cout << "Event initialized: " << eventId << std::endl;
    return true;
}

std::vector<std::string> FileManager::listEvents() {
    std::vector<std::string> events;
    std::string eventsDir = baseDir_ + PATH_SEPARATOR "events";

    if (!fileExists(eventsDir)) return events;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = eventsDir + PATH_SEPARATOR "*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            if (name != "." && name != ".." && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                events.push_back(name);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(eventsDir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != ".." && entry->d_type == DT_DIR) {
                events.push_back(name);
            }
        }
        closedir(dir);
    }
#endif

    return events;
}

bool FileManager::deleteEvent(const std::string& eventId) {
    std::string eventPath = getEventPath(eventId);
    // TODO: Implement recursive directory deletion
    return deleteEventDirectory(eventPath);
}

bool FileManager::saveEventSettings(const std::string& eventId, const EventSettings& settings) {
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "event_settings.json";

    std::ostringstream json;
    json << "{\n";
    json << "  \"eventId\": \"" << settings.eventId << "\",\n";
    json << "  \"eventName\": \"" << settings.eventName << "\",\n";
    json << "  \"createdAt\": \"" << settings.createdAt << "\",\n";
    json << "  \"currentLayout\": \"" << settings.currentLayout << "\",\n";
    json << "  \"photoCount\": " << settings.photoCount << "\n";
    json << "}\n";

    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << json.str();
    return true;
}

EventSettings FileManager::loadEventSettings(const std::string& eventId) {
    EventSettings settings;
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "event_settings.json";

    std::ifstream file(path);
    if (!file.is_open()) {
        settings.eventId = eventId;
        return settings;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // Simple JSON parsing (production should use a proper JSON library)
    auto extractString = [&content](const std::string& key) -> std::string {
        size_t keyPos = content.find("\"" + key + "\"");
        if (keyPos == std::string::npos) return "";
        size_t colonPos = content.find(":", keyPos);
        size_t startQuote = content.find("\"", colonPos);
        size_t endQuote = content.find("\"", startQuote + 1);
        if (startQuote != std::string::npos && endQuote != std::string::npos) {
            return content.substr(startQuote + 1, endQuote - startQuote - 1);
        }
        return "";
    };

    auto extractInt = [&content](const std::string& key) -> int {
        size_t keyPos = content.find("\"" + key + "\"");
        if (keyPos == std::string::npos) return 0;
        size_t colonPos = content.find(":", keyPos);
        if (colonPos == std::string::npos) return 0;
        size_t start = content.find_first_of("0123456789", colonPos);
        size_t end = content.find_first_not_of("0123456789", start);
        if (start != std::string::npos) {
            return std::stoi(content.substr(start, end - start));
        }
        return 0;
    };

    settings.eventId = extractString("eventId");
    settings.eventName = extractString("eventName");
    settings.createdAt = extractString("createdAt");
    settings.currentLayout = extractString("currentLayout");
    settings.photoCount = extractInt("photoCount");

    return settings;
}

bool FileManager::saveEventFullConfig(const std::string& eventId, const std::string& jsonConfig) {
    if (!eventExists(eventId)) {
        initializeEvent(eventId);
    }
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "config" PATH_SEPARATOR "config.json";
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << jsonConfig;
    // std::cout << "Full config saved to " << path << std::endl;
    return true;
}

std::string FileManager::loadEventFullConfig(const std::string& eventId) {
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "config" PATH_SEPARATOR "config.json";
    std::ifstream file(path);
    if (!file.is_open()) return "{}";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileManager::saveLayout(const std::string& eventId,
                                    const std::vector<uint8_t>& pngData,
                                    const std::string& layoutName) {
    if (!eventExists(eventId)) {
        initializeEvent(eventId);
    }

    std::string safeName = sanitizeFilename(layoutName);
    if (safeName.empty()) safeName = "layout";
    if (safeName.find(".png") == std::string::npos) safeName += ".png";

    std::string layoutPath = getEventPath(eventId) + PATH_SEPARATOR "layouts" PATH_SEPARATOR + safeName;

    std::ofstream file(layoutPath, std::ios::binary);
    if (!file.is_open()) return "";

    file.write(reinterpret_cast<const char*>(pngData.data()), pngData.size());
    file.close();

    std::cout << "Layout saved: " << layoutPath << std::endl;
    return layoutPath;
}

std::vector<std::string> FileManager::listLayouts(const std::string& eventId) {
    std::vector<std::string> layouts;
    std::string layoutsDir = getEventPath(eventId) + PATH_SEPARATOR "layouts";

    if (!fileExists(layoutsDir)) return layouts;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = layoutsDir + PATH_SEPARATOR "*.png";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            layouts.push_back(findData.cFileName);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(layoutsDir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.size() > 4 && name.substr(name.size() - 4) == ".png") {
                layouts.push_back(name);
            }
        }
        closedir(dir);
    }
#endif

    return layouts;
}

bool FileManager::deleteLayout(const std::string& eventId, const std::string& layoutName) {
    std::string layoutPath = getLayoutPath(eventId, layoutName);
    return std::remove(layoutPath.c_str()) == 0;
}

std::string FileManager::getLayoutPath(const std::string& eventId, const std::string& layoutName) {
    std::string safeName = sanitizeFilename(layoutName);
    if (safeName.find(".png") == std::string::npos) safeName += ".png";
    return getEventPath(eventId) + PATH_SEPARATOR "layouts" PATH_SEPARATOR + safeName;
}

bool FileManager::saveSlotsConfig(const std::string& eventId, const LayoutConfig& config) {
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "config" PATH_SEPARATOR "slots_config.json";

    std::ostringstream json;
    json << "{\n";
    json << "  \"layout_name\": \"" << config.layoutName << "\",\n";
    json << "  \"layout_path\": \"" << config.layoutPath << "\",\n";
    json << "  \"slots_count\": " << config.slotsCount << ",\n";
    json << "  \"slots\": [\n";

    for (size_t i = 0; i < config.slots.size(); ++i) {
        const auto& slot = config.slots[i];
        json << "    {\"id\": " << slot.id
             << ", \"x\": " << slot.x
             << ", \"y\": " << slot.y
             << ", \"width\": " << slot.width
             << ", \"height\": " << slot.height << "}";
        if (i < config.slots.size() - 1) json << ",";
        json << "\n";
    }

    json << "  ]\n";
    json << "}\n";

    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << json.str();

    std::cout << "Slots config saved: " << path << std::endl;
    return true;
}

LayoutConfig FileManager::loadSlotsConfig(const std::string& eventId, const std::string& layoutName) {
    LayoutConfig config;
    std::string path = getEventPath(eventId) + PATH_SEPARATOR "config" PATH_SEPARATOR "slots_config.json";

    std::ifstream file(path);
    if (!file.is_open()) return config;

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // Simple JSON parsing - extract layout_name
    size_t namePos = content.find("\"layout_name\"");
    if (namePos != std::string::npos) {
        size_t startQuote = content.find("\"", content.find(":", namePos));
        size_t endQuote = content.find("\"", startQuote + 1);
        if (startQuote != std::string::npos && endQuote != std::string::npos) {
            config.layoutName = content.substr(startQuote + 1, endQuote - startQuote - 1);
        }
    }

    // Extract slots_count
    size_t countPos = content.find("\"slots_count\"");
    if (countPos != std::string::npos) {
        size_t colonPos = content.find(":", countPos);
        size_t start = content.find_first_of("0123456789", colonPos);
        size_t end = content.find_first_not_of("0123456789", start);
        if (start != std::string::npos) {
            config.slotsCount = std::stoi(content.substr(start, end - start));
        }
    }

    // Extract slots array - simple parsing
    size_t slotsPos = content.find("\"slots\"");
    if (slotsPos != std::string::npos) {
        size_t arrStart = content.find("[", slotsPos);
        size_t arrEnd = content.find("]", arrStart);
        std::string slotsJson = content.substr(arrStart, arrEnd - arrStart + 1);

        size_t pos = 0;
        int id = 1;
        while ((pos = slotsJson.find("{", pos)) != std::string::npos) {
            size_t objEnd = slotsJson.find("}", pos);
            std::string obj = slotsJson.substr(pos, objEnd - pos + 1);

            SlotInfo slot;
            slot.id = id++;

            auto extractInt = [&obj](const std::string& key) -> int {
                size_t keyPos = obj.find("\"" + key + "\"");
                if (keyPos == std::string::npos) return 0;
                size_t colonPos = obj.find(":", keyPos);
                size_t start = obj.find_first_of("-0123456789", colonPos);
                size_t end = obj.find_first_not_of("-0123456789", start);
                if (start != std::string::npos) {
                    return std::stoi(obj.substr(start, end - start));
                }
                return 0;
            };

            slot.x = extractInt("x");
            slot.y = extractInt("y");
            slot.width = extractInt("width");
            slot.height = extractInt("height");

            config.slots.push_back(slot);
            pos = objEnd + 1;
        }
    }

    return config;
}

std::string FileManager::saveCapturedPhoto(const std::string& eventId,
                                           const std::vector<uint8_t>& imageData,
                                           const std::string& prefix) {
    if (!eventExists(eventId)) {
        initializeEvent(eventId);
    }

    // Generate unique filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif

    std::ostringstream oss;
    oss << prefix << "_"
        << std::put_time(&tm_now, "%Y%m%d_%H%M%S")
        << "_" << std::setfill('0') << std::setw(3) << ms.count()
        << ".jpg";

    std::string filename = oss.str();
    std::string filePath = getEventPath(eventId) + PATH_SEPARATOR "captured_photos" PATH_SEPARATOR + filename;

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) return "";

    file.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    file.close();

    // Update photo count
    EventSettings settings = loadEventSettings(eventId);
    settings.photoCount++;
    saveEventSettings(eventId, settings);

    std::cout << "Photo captured: " << filePath << std::endl;
    return filePath;
}

std::vector<std::string> FileManager::listCapturedPhotos(const std::string& eventId) {
    std::vector<std::string> photos;
    std::string photosDir = getEventPath(eventId) + PATH_SEPARATOR "captured_photos";

    if (!fileExists(photosDir)) return photos;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = photosDir + PATH_SEPARATOR "*.*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find(".jpg") != std::string::npos ||
                lower.find(".jpeg") != std::string::npos ||
                lower.find(".png") != std::string::npos) {
                photos.push_back(photosDir + PATH_SEPARATOR + name);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(photosDir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find(".jpg") != std::string::npos ||
                lower.find(".jpeg") != std::string::npos ||
                lower.find(".png") != std::string::npos) {
                photos.push_back(photosDir + "/" + name);
            }
        }
        closedir(dir);
    }
#endif

    std::sort(photos.begin(), photos.end());
    return photos;
}

// Legacy methods
std::string FileManager::createEventDirectory(const std::string& eventName) {
    std::string eventId = sanitizeFilename(eventName);
    if (initializeEvent(eventId)) {
        return getEventPath(eventId);
    }
    return "";
}

bool FileManager::deleteEventDirectory(const std::string& eventPath) {
    // TODO: Implement recursive deletion
    // For now, just try to remove if empty
#ifdef _WIN32
    return RemoveDirectoryA(eventPath.c_str()) != 0;
#else
    return rmdir(eventPath.c_str()) == 0;
#endif
}

std::string FileManager::saveImage(const std::vector<uint8_t>& imageData,
                                   const std::string& eventPath,
                                   const std::string& filename) {
    std::string filePath = eventPath + PATH_SEPARATOR + filename;

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) return "";

    file.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    return filePath;
}

bool FileManager::deleteImage(const std::string& filePath) {
    return std::remove(filePath.c_str()) == 0;
}

std::vector<uint8_t> FileManager::loadImage(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

std::string FileManager::createThumbnail(const std::string& imagePath, int maxSize) {
    // TODO: Implement with OpenCV or STB
    return "";
}

bool FileManager::exportEvent(const std::string& eventPath, const std::string& exportPath) {
    // TODO: Implement event export (zip or copy)
    return false;
}

bool FileManager::fileExists(const std::string& path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES;
#else
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
#endif
}

long long FileManager::getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return -1;
    return file.tellg();
}

std::vector<std::string> FileManager::listFiles(const std::string& directory) {
    std::vector<std::string> files;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = directory + PATH_SEPARATOR "*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            if (name != "." && name != "..") {
                files.push_back(directory + PATH_SEPARATOR + name);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                files.push_back(directory + "/" + name);
            }
        }
        closedir(dir);
    }
#endif

    return files;
}

std::string FileManager::generateUniqueFilename(const std::string& prefix) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return prefix + "_" + std::to_string(ms);
}

} // namespace photobooth
