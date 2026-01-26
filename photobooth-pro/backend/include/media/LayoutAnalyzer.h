#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace photobooth {

struct LayoutSlot {
  int id;
  int x;
  int y;
  int width;
  int height;
  int rotation; // 0 for now
};

struct LayoutInfo {
  std::string imagePath;
  int width;
  int height;
  int photoCount;
  std::vector<LayoutSlot> slots;

  nlohmann::json toJson() const;
  static LayoutInfo fromJson(const nlohmann::json &j);
};

class LayoutAnalyzer {
public:
  /**
   * Phân tích file PNG để tìm các vùng trong suốt (slots)
   * @param imagePath Đường dẫn tuyệt đối đến file layout png
   * @param outputPath Đường dẫn để lưu file slots_config.json
   * @return LayoutInfo chứa thông tin các slot tìm được
   */
  static LayoutInfo analyzeAndSave(const std::string &imagePath,
                                   const std::string &jsonOutputPath);

private:
  // Helper để sort các slot theo thứ tự tự nhiên (trên xuống, trái sang phải)
  static void sortSlots(std::vector<LayoutSlot> &slots);
};

} // namespace photobooth
