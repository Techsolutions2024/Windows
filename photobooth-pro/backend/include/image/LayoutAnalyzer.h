#pragma once

#include "storage/FileManager.h"
#include <string>
#include <vector>
#include <cstdint>

namespace photobooth {

// Bounding box for a transparent region
struct BoundingBox {
    int x;
    int y;
    int width;
    int height;
    int area;
};

class LayoutAnalyzer {
public:
    LayoutAnalyzer();
    ~LayoutAnalyzer();

    // Analyze PNG and detect transparent slots
    // Returns vector of slot coordinates detected from alpha channel
    std::vector<SlotInfo> analyzePNG(const std::string& pngPath);

    // Analyze PNG from memory
    std::vector<SlotInfo> analyzePNG(const std::vector<uint8_t>& pngData);

    // Create LayoutConfig from analysis
    LayoutConfig analyzeLayout(const std::string& pngPath, const std::string& layoutName);

    // Get image dimensions without full analysis
    bool getImageDimensions(const std::string& pngPath, int& width, int& height);

    // Settings for analysis
    void setMinSlotArea(int area) { minSlotArea_ = area; }
    void setAlphaThreshold(uint8_t threshold) { alphaThreshold_ = threshold; }
    void setMergeDistance(int distance) { mergeDistance_ = distance; }

private:
    int minSlotArea_;           // Minimum area to consider as slot (default: 10000 px)
    uint8_t alphaThreshold_;    // Alpha below this = transparent (default: 10)
    int mergeDistance_;         // Distance to merge nearby regions (default: 5)

    // Internal helpers
    std::vector<BoundingBox> findTransparentRegions(
        const uint8_t* alphaChannel, int width, int height);

    std::vector<BoundingBox> mergeOverlappingBoxes(
        const std::vector<BoundingBox>& boxes);

    void floodFillMask(
        const uint8_t* alpha, uint8_t* visited,
        int width, int height,
        int startX, int startY,
        int& minX, int& maxX, int& minY, int& maxY);
};

// Image composition functions
class ImageCompositor {
public:
    ImageCompositor();
    ~ImageCompositor();

    // Compose photos into layout slots
    // layoutPath: PNG with transparent slots
    // photos: vector of photo paths to place in slots
    // outputPath: where to save the composed image
    bool compose(const std::string& layoutPath,
                const std::vector<std::string>& photos,
                const LayoutConfig& config,
                const std::string& outputPath);

    // Compose with raw photo data
    bool compose(const std::string& layoutPath,
                const std::vector<std::vector<uint8_t>>& photoData,
                const LayoutConfig& config,
                std::vector<uint8_t>& output);

    // Resize and crop image to fit slot (center crop)
    std::vector<uint8_t> fitImageToSlot(
        const std::vector<uint8_t>& imageData,
        int slotWidth, int slotHeight);

    // Resize image maintaining aspect ratio
    std::vector<uint8_t> resizeImage(
        const std::vector<uint8_t>& imageData,
        int maxWidth, int maxHeight);

private:
    // Internal composition with OpenCV
    bool composeWithOpenCV(
        const std::string& layoutPath,
        const std::vector<std::string>& photos,
        const LayoutConfig& config,
        const std::string& outputPath);
};

} // namespace photobooth
