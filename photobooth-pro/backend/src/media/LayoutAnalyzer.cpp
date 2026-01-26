#include "media/LayoutAnalyzer.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>


namespace fs = std::filesystem;

namespace photobooth {

nlohmann::json LayoutInfo::toJson() const {
  nlohmann::json j;
  j["imagePath"] = fs::path(imagePath).filename().string();
  j["width"] = width;
  j["height"] = height;
  j["photoCount"] = photoCount;

  nlohmann::json slotsArray = nlohmann::json::array();
  for (const auto &slot : slots) {
    slotsArray.push_back({{"id", slot.id},
                          {"x", slot.x},
                          {"y", slot.y},
                          {"width", slot.width},
                          {"height", slot.height},
                          {"rotation", slot.rotation}});
  }
  j["slots"] = slotsArray;
  return j;
}

LayoutInfo LayoutInfo::fromJson(const nlohmann::json &j) {
  LayoutInfo info;
  info.imagePath = j.value("imagePath", "");
  info.width = j.value("width", 0);
  info.height = j.value("height", 0);
  info.photoCount = j.value("photoCount", 0);

  if (j.contains("slots") && j["slots"].is_array()) {
    for (const auto &slotJson : j["slots"]) {
      LayoutSlot slot;
      slot.id = slotJson.value("id", 0);
      slot.x = slotJson.value("x", 0);
      slot.y = slotJson.value("y", 0);
      slot.width = slotJson.value("width", 0);
      slot.height = slotJson.value("height", 0);
      slot.rotation = slotJson.value("rotation", 0);
      info.slots.push_back(slot);
    }
  }
  return info;
}

LayoutInfo LayoutAnalyzer::analyzeAndSave(const std::string &imagePath,
                                          const std::string &jsonOutputPath) {
  LayoutInfo info;
  info.imagePath = imagePath;

  // 1. Load image using OpenCV
  cv::Mat img = cv::imread(imagePath, cv::IMREAD_UNCHANGED);

  if (img.empty()) {
    throw std::runtime_error("Failed to load layout image: " + imagePath);
  }

  info.width = img.cols;
  info.height = img.rows;

  // Check if image has alpha channel
  if (img.channels() != 4) {
    std::cerr << "Warning: Layout image has no alpha channel. Cannot detect "
                 "transparent slots."
              << std::endl;
    // Return info with 0 slots
    info.photoCount = 0;
  } else {
    // 2. Extract Alpha Channel
    std::vector<cv::Mat> channels;
    cv::split(img, channels);
    cv::Mat alpha = channels[3];

    // 3. Create a binary mask where Transparent (Alpha=0) is White (255) and
    // Opaque is Black (0) We use threshold to find pixels with alpha < 10
    // (almost fully transparent)
    cv::Mat mask;
    cv::threshold(alpha, mask, 10, 255, cv::THRESH_BINARY_INV);

    // 4. Find Contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    // 5. Process Contours to find bounding boxes
    int idCounter = 1;
    for (const auto &contour : contours) {
      // Calculate area to filter noise
      double area = cv::contourArea(contour);

      // Filter noise: Slot must be at least 5% of total image area? Or just a
      // reasonable fixed size Let's say at least 100x100 pixels = 10000
      if (area > 10000) {
        cv::Rect rect = cv::boundingRect(contour);

        // Fine tune: ensure rect is fully inside image
        rect = rect & cv::Rect(0, 0, img.cols, img.rows);

        // Create LayoutSlot
        LayoutSlot slot;
        // ID will be assigned after sorting
        slot.x = rect.x;
        slot.y = rect.y;
        slot.width = rect.width;
        slot.height = rect.height;
        slot.rotation = 0; // OpenCV boundingRect is always upright

        info.slots.push_back(slot);
      }
    }

    // Sort slots logically (Top-Left to Bottom-Right)
    sortSlots(info.slots);

    // Assign IDs based on sorted order
    for (size_t i = 0; i < info.slots.size(); i++) {
      info.slots[i].id = i + 1;
    }

    info.photoCount = static_cast<int>(info.slots.size());
  }

  // 6. Save to JSON
  nlohmann::json j = info.toJson();
  std::ofstream o(jsonOutputPath);
  o << std::setw(4) << j << std::endl;

  std::cout << "Layout analysis complete. Found " << info.photoCount
            << " slots. Saved to " << jsonOutputPath << std::endl;

  return info;
}

void LayoutAnalyzer::sortSlots(std::vector<LayoutSlot> &slots) {
  std::sort(slots.begin(), slots.end(),
            [](const LayoutSlot &a, const LayoutSlot &b) {
              // Logic: Sort by Y (rows), then by X (columns)
              // Fuzzy compare for Y to handle slightly misaligned rows

              const int Y_TOLERANCE = 50; // pixels

              if (std::abs(a.y - b.y) > Y_TOLERANCE) {
                return a.y < b.y;
              } else {
                return a.x < b.x;
              }
            });
}

} // namespace photobooth
