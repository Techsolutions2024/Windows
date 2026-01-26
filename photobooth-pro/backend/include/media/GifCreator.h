#pragma once

#include <string>
#include <vector>

namespace photobooth {

/**
 * GifCreator - Tạo ảnh GIF động từ nhiều ảnh JPEG
 * Sử dụng ImageMagick hoặc gifsicle thông qua system call
 */
class GifCreator {
public:
  struct GifOptions {
    int frameDelay = 10;  // Delay giữa các frame (x10ms, vd: 10 = 100ms)
    int loopCount = 0;    // 0 = infinite loop
    int width = 800;      // Chiều rộng output
    int height = 600;     // Chiều cao output
    int quality = 80;     // Chất lượng JPEG (1-100)
    bool optimize = true; // Tối ưu file size
  };

  GifCreator();
  ~GifCreator();

  /**
   * Tạo GIF từ danh sách ảnh
   * @param imagePaths Danh sách đường dẫn ảnh input
   * @param outputPath Đường dẫn file GIF output
   * @param options Tùy chọn tạo GIF
   * @return Đường dẫn file GIF đã tạo, hoặc empty string nếu lỗi
   */
  std::string createGif(const std::vector<std::string> &imagePaths,
                        const std::string &outputPath,
                        const GifOptions &options = GifOptions());

  /**
   * Kiểm tra ImageMagick có sẵn không
   */
  static bool isImageMagickAvailable();

  /**
   * Kiểm tra gifsicle có sẵn không
   */
  static bool isGifsicleAvailable();

private:
  /**
   * Resize tất cả ảnh về cùng kích thước
   * @param imagePaths Danh sách ảnh input
   * @param tempDir Thư mục tạm để lưu ảnh đã resize
   * @param width Chiều rộng target
   * @param height Chiều cao target
   * @return Danh sách đường dẫn ảnh đã resize
   */
  std::vector<std::string>
  resizeImages(const std::vector<std::string> &imagePaths,
               const std::string &tempDir, int width, int height);

  /**
   * Tạo GIF bằng ImageMagick
   */
  bool createGifWithImageMagick(const std::vector<std::string> &imagePaths,
                                const std::string &outputPath,
                                const GifOptions &options);

  /**
   * Tạo GIF bằng gifsicle (yêu cầu convert PNG trước)
   */
  bool createGifWithGifsicle(const std::vector<std::string> &imagePaths,
                             const std::string &outputPath,
                             const GifOptions &options);

  /**
   * Build ImageMagick command
   */
  std::string
  buildImageMagickCommand(const std::vector<std::string> &imagePaths,
                          const std::string &outputPath,
                          const GifOptions &options);

  /**
   * Execute system command và capture output
   */
  int executeCommand(const std::string &command, std::string &output);

  /**
   * Tạo thư mục tạm
   */
  std::string createTempDirectory();

  /**
   * Xóa thư mục tạm
   */
  void cleanupTempDirectory(const std::string &dir);
};

} // namespace photobooth
