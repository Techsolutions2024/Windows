#include "image/LayoutAnalyzer.h"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <queue>
#include <iostream>

namespace photobooth {

// ============== LayoutAnalyzer ==============

LayoutAnalyzer::LayoutAnalyzer()
    : minSlotArea_(10000)       // 100x100 minimum slot
    , alphaThreshold_(10)       // Nearly fully transparent
    , mergeDistance_(5)         // Merge regions within 5 pixels
{
}

LayoutAnalyzer::~LayoutAnalyzer() {}

bool LayoutAnalyzer::getImageDimensions(const std::string& pngPath, int& width, int& height) {
    cv::Mat img = cv::imread(pngPath, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "Failed to load image: " << pngPath << std::endl;
        return false;
    }
    width = img.cols;
    height = img.rows;
    return true;
}

std::vector<SlotInfo> LayoutAnalyzer::analyzePNG(const std::string& pngPath) {
    std::vector<SlotInfo> slots;

    // Load PNG with alpha channel
    cv::Mat img = cv::imread(pngPath, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "Failed to load PNG: " << pngPath << std::endl;
        return slots;
    }

    // Check if image has alpha channel
    if (img.channels() != 4) {
        std::cerr << "PNG does not have alpha channel: " << pngPath << std::endl;
        return slots;
    }

    int width = img.cols;
    int height = img.rows;

    // Extract alpha channel
    std::vector<cv::Mat> channels;
    cv::split(img, channels);
    cv::Mat alpha = channels[3];

    // Find transparent regions
    std::vector<BoundingBox> regions = findTransparentRegions(
        alpha.data, width, height);

    // Merge overlapping boxes
    regions = mergeOverlappingBoxes(regions);

    // Sort by position (top to bottom, left to right)
    std::sort(regions.begin(), regions.end(), [](const BoundingBox& a, const BoundingBox& b) {
        if (abs(a.y - b.y) < 50) {
            return a.x < b.x;
        }
        return a.y < b.y;
    });

    // Convert to SlotInfo
    int id = 1;
    for (const auto& box : regions) {
        if (box.area >= minSlotArea_) {
            SlotInfo slot;
            slot.id = id++;
            slot.x = box.x;
            slot.y = box.y;
            slot.width = box.width;
            slot.height = box.height;
            slots.push_back(slot);

            std::cout << "Slot " << slot.id << ": x=" << slot.x << " y=" << slot.y
                      << " w=" << slot.width << " h=" << slot.height << std::endl;
        }
    }

    std::cout << "Found " << slots.size() << " slots in layout" << std::endl;
    return slots;
}

std::vector<SlotInfo> LayoutAnalyzer::analyzePNG(const std::vector<uint8_t>& pngData) {
    std::vector<SlotInfo> slots;

    // Decode PNG from memory
    cv::Mat img = cv::imdecode(pngData, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "Failed to decode PNG from memory" << std::endl;
        return slots;
    }

    if (img.channels() != 4) {
        std::cerr << "PNG does not have alpha channel" << std::endl;
        return slots;
    }

    int width = img.cols;
    int height = img.rows;

    std::vector<cv::Mat> channels;
    cv::split(img, channels);
    cv::Mat alpha = channels[3];

    std::vector<BoundingBox> regions = findTransparentRegions(
        alpha.data, width, height);
    regions = mergeOverlappingBoxes(regions);

    std::sort(regions.begin(), regions.end(), [](const BoundingBox& a, const BoundingBox& b) {
        if (abs(a.y - b.y) < 50) return a.x < b.x;
        return a.y < b.y;
    });

    int id = 1;
    for (const auto& box : regions) {
        if (box.area >= minSlotArea_) {
            SlotInfo slot;
            slot.id = id++;
            slot.x = box.x;
            slot.y = box.y;
            slot.width = box.width;
            slot.height = box.height;
            slots.push_back(slot);
        }
    }

    return slots;
}

LayoutConfig LayoutAnalyzer::analyzeLayout(const std::string& pngPath, const std::string& layoutName) {
    LayoutConfig config;
    config.layoutName = layoutName;
    config.layoutPath = pngPath;
    config.slots = analyzePNG(pngPath);
    config.slotsCount = static_cast<int>(config.slots.size());
    return config;
}

std::vector<BoundingBox> LayoutAnalyzer::findTransparentRegions(
    const uint8_t* alphaChannel, int width, int height) {

    std::vector<BoundingBox> boxes;
    std::vector<uint8_t> visited(width * height, 0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;

            // Skip if already visited or not transparent
            if (visited[idx] || alphaChannel[idx] > alphaThreshold_) {
                continue;
            }

            // Found a transparent pixel - flood fill to find bounds
            int minX = x, maxX = x, minY = y, maxY = y;
            floodFillMask(alphaChannel, visited.data(), width, height,
                         x, y, minX, maxX, minY, maxY);

            int boxWidth = maxX - minX + 1;
            int boxHeight = maxY - minY + 1;
            int area = boxWidth * boxHeight;

            if (area >= minSlotArea_) {
                BoundingBox box;
                box.x = minX;
                box.y = minY;
                box.width = boxWidth;
                box.height = boxHeight;
                box.area = area;
                boxes.push_back(box);
            }
        }
    }

    return boxes;
}

void LayoutAnalyzer::floodFillMask(
    const uint8_t* alpha, uint8_t* visited,
    int width, int height,
    int startX, int startY,
    int& minX, int& maxX, int& minY, int& maxY) {

    std::queue<std::pair<int, int>> queue;
    queue.push({startX, startY});

    const int dx[] = {-1, 0, 1, 0};
    const int dy[] = {0, -1, 0, 1};

    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();

        if (x < 0 || x >= width || y < 0 || y >= height) continue;

        int idx = y * width + x;
        if (visited[idx] || alpha[idx] > alphaThreshold_) continue;

        visited[idx] = 1;

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);

        for (int i = 0; i < 4; i++) {
            queue.push({x + dx[i], y + dy[i]});
        }
    }
}

std::vector<BoundingBox> LayoutAnalyzer::mergeOverlappingBoxes(
    const std::vector<BoundingBox>& boxes) {

    if (boxes.empty()) return boxes;

    std::vector<BoundingBox> result = boxes;
    bool merged = true;

    while (merged) {
        merged = false;
        std::vector<BoundingBox> newResult;
        std::vector<bool> used(result.size(), false);

        for (size_t i = 0; i < result.size(); i++) {
            if (used[i]) continue;

            BoundingBox current = result[i];

            for (size_t j = i + 1; j < result.size(); j++) {
                if (used[j]) continue;

                const BoundingBox& other = result[j];

                // Check if boxes overlap or are close
                int expandedX1 = current.x - mergeDistance_;
                int expandedY1 = current.y - mergeDistance_;
                int expandedX2 = current.x + current.width + mergeDistance_;
                int expandedY2 = current.y + current.height + mergeDistance_;

                int otherX2 = other.x + other.width;
                int otherY2 = other.y + other.height;

                bool overlaps =
                    expandedX1 < otherX2 && expandedX2 > other.x &&
                    expandedY1 < otherY2 && expandedY2 > other.y;

                if (overlaps) {
                    // Merge boxes
                    int newX = std::min(current.x, other.x);
                    int newY = std::min(current.y, other.y);
                    int newX2 = std::max(current.x + current.width, otherX2);
                    int newY2 = std::max(current.y + current.height, otherY2);

                    current.x = newX;
                    current.y = newY;
                    current.width = newX2 - newX;
                    current.height = newY2 - newY;
                    current.area = current.width * current.height;

                    used[j] = true;
                    merged = true;
                }
            }

            newResult.push_back(current);
        }

        result = newResult;
    }

    return result;
}

// ============== ImageCompositor ==============

ImageCompositor::ImageCompositor() {}

ImageCompositor::~ImageCompositor() {}

bool ImageCompositor::compose(
    const std::string& layoutPath,
    const std::vector<std::string>& photos,
    const LayoutConfig& config,
    const std::string& outputPath) {

    return composeWithOpenCV(layoutPath, photos, config, outputPath);
}

bool ImageCompositor::compose(
    const std::string& layoutPath,
    const std::vector<std::vector<uint8_t>>& photoData,
    const LayoutConfig& config,
    std::vector<uint8_t>& output) {

    // Load layout
    cv::Mat layout = cv::imread(layoutPath, cv::IMREAD_UNCHANGED);
    if (layout.empty()) {
        std::cerr << "Failed to load layout: " << layoutPath << std::endl;
        return false;
    }

    // Create output image (BGR without alpha for final JPEG)
    cv::Mat result(layout.rows, layout.cols, CV_8UC3, cv::Scalar(255, 255, 255));

    // Place photos in slots
    size_t photoIndex = 0;
    for (const auto& slot : config.slots) {
        if (photoIndex >= photoData.size()) break;

        // Decode photo
        cv::Mat photo = cv::imdecode(photoData[photoIndex], cv::IMREAD_COLOR);
        if (photo.empty()) {
            photoIndex++;
            continue;
        }

        // Resize photo to fit slot (center crop)
        cv::Mat fitted = cv::Mat(slot.height, slot.width, CV_8UC3);
        double scaleX = static_cast<double>(slot.width) / photo.cols;
        double scaleY = static_cast<double>(slot.height) / photo.rows;
        double scale = std::max(scaleX, scaleY);

        int newWidth = static_cast<int>(photo.cols * scale);
        int newHeight = static_cast<int>(photo.rows * scale);

        cv::Mat resized;
        cv::resize(photo, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

        // Center crop
        int cropX = (newWidth - slot.width) / 2;
        int cropY = (newHeight - slot.height) / 2;
        cv::Rect cropRect(cropX, cropY, slot.width, slot.height);
        fitted = resized(cropRect).clone();

        // Place in result
        cv::Rect slotRect(slot.x, slot.y, slot.width, slot.height);
        fitted.copyTo(result(slotRect));

        photoIndex++;
    }

    // Overlay layout (only non-transparent pixels)
    if (layout.channels() == 4) {
        for (int y = 0; y < layout.rows; y++) {
            for (int x = 0; x < layout.cols; x++) {
                cv::Vec4b pixel = layout.at<cv::Vec4b>(y, x);
                uint8_t alpha = pixel[3];
                if (alpha > 10) {
                    cv::Vec3b& dst = result.at<cv::Vec3b>(y, x);
                    float a = alpha / 255.0f;
                    dst[0] = static_cast<uint8_t>(pixel[0] * a + dst[0] * (1 - a));
                    dst[1] = static_cast<uint8_t>(pixel[1] * a + dst[1] * (1 - a));
                    dst[2] = static_cast<uint8_t>(pixel[2] * a + dst[2] * (1 - a));
                }
            }
        }
    }

    // Encode to JPEG
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    cv::imencode(".jpg", result, output, params);

    return true;
}

bool ImageCompositor::composeWithOpenCV(
    const std::string& layoutPath,
    const std::vector<std::string>& photos,
    const LayoutConfig& config,
    const std::string& outputPath) {

    // Load layout with alpha
    cv::Mat layout = cv::imread(layoutPath, cv::IMREAD_UNCHANGED);
    if (layout.empty()) {
        std::cerr << "Failed to load layout: " << layoutPath << std::endl;
        return false;
    }

    // Create output image
    cv::Mat result(layout.rows, layout.cols, CV_8UC3, cv::Scalar(255, 255, 255));

    // Place photos in slots
    size_t photoIndex = 0;
    for (const auto& slot : config.slots) {
        if (photoIndex >= photos.size()) break;

        cv::Mat photo = cv::imread(photos[photoIndex], cv::IMREAD_COLOR);
        if (photo.empty()) {
            std::cerr << "Failed to load photo: " << photos[photoIndex] << std::endl;
            photoIndex++;
            continue;
        }

        // Calculate scale to cover slot (maintaining aspect ratio)
        double scaleX = static_cast<double>(slot.width) / photo.cols;
        double scaleY = static_cast<double>(slot.height) / photo.rows;
        double scale = std::max(scaleX, scaleY);

        int newWidth = static_cast<int>(photo.cols * scale);
        int newHeight = static_cast<int>(photo.rows * scale);

        cv::Mat resized;
        cv::resize(photo, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

        // Center crop to slot size
        int cropX = (newWidth - slot.width) / 2;
        int cropY = (newHeight - slot.height) / 2;
        cropX = std::max(0, cropX);
        cropY = std::max(0, cropY);

        cv::Rect cropRect(cropX, cropY,
                         std::min(slot.width, resized.cols - cropX),
                         std::min(slot.height, resized.rows - cropY));
        cv::Mat cropped = resized(cropRect);

        // Copy to result at slot position
        cv::Rect destRect(slot.x, slot.y, cropped.cols, cropped.rows);
        cropped.copyTo(result(destRect));

        photoIndex++;
    }

    // Overlay the layout PNG on top (preserving transparency)
    if (layout.channels() == 4) {
        std::vector<cv::Mat> channels;
        cv::split(layout, channels);
        cv::Mat alpha = channels[3];

        // Convert layout to BGR for blending
        cv::Mat layoutBGR;
        cv::cvtColor(layout, layoutBGR, cv::COLOR_BGRA2BGR);

        // Alpha blend
        for (int y = 0; y < layout.rows && y < result.rows; y++) {
            for (int x = 0; x < layout.cols && x < result.cols; x++) {
                uint8_t a = alpha.at<uint8_t>(y, x);
                if (a > 10) {
                    float af = a / 255.0f;
                    cv::Vec3b& dst = result.at<cv::Vec3b>(y, x);
                    cv::Vec3b src = layoutBGR.at<cv::Vec3b>(y, x);
                    dst[0] = static_cast<uint8_t>(src[0] * af + dst[0] * (1 - af));
                    dst[1] = static_cast<uint8_t>(src[1] * af + dst[1] * (1 - af));
                    dst[2] = static_cast<uint8_t>(src[2] * af + dst[2] * (1 - af));
                }
            }
        }
    }

    // Save result
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    return cv::imwrite(outputPath, result, params);
}

std::vector<uint8_t> ImageCompositor::fitImageToSlot(
    const std::vector<uint8_t>& imageData,
    int slotWidth, int slotHeight) {

    cv::Mat img = cv::imdecode(imageData, cv::IMREAD_COLOR);
    if (img.empty()) return {};

    double scaleX = static_cast<double>(slotWidth) / img.cols;
    double scaleY = static_cast<double>(slotHeight) / img.rows;
    double scale = std::max(scaleX, scaleY);

    int newWidth = static_cast<int>(img.cols * scale);
    int newHeight = static_cast<int>(img.rows * scale);

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

    int cropX = (newWidth - slotWidth) / 2;
    int cropY = (newHeight - slotHeight) / 2;
    cv::Rect cropRect(cropX, cropY, slotWidth, slotHeight);
    cv::Mat cropped = resized(cropRect);

    std::vector<uint8_t> output;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    cv::imencode(".jpg", cropped, output, params);

    return output;
}

std::vector<uint8_t> ImageCompositor::resizeImage(
    const std::vector<uint8_t>& imageData,
    int maxWidth, int maxHeight) {

    cv::Mat img = cv::imdecode(imageData, cv::IMREAD_COLOR);
    if (img.empty()) return {};

    double scaleX = static_cast<double>(maxWidth) / img.cols;
    double scaleY = static_cast<double>(maxHeight) / img.rows;
    double scale = std::min(scaleX, scaleY);

    if (scale >= 1.0) {
        return imageData; // No need to resize
    }

    int newWidth = static_cast<int>(img.cols * scale);
    int newHeight = static_cast<int>(img.rows * scale);

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

    std::vector<uint8_t> output;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    cv::imencode(".jpg", resized, output, params);

    return output;
}

} // namespace photobooth
