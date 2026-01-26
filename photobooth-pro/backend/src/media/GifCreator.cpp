#include "media/GifCreator.h"
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#endif

namespace fs = std::filesystem;

namespace photobooth {

GifCreator::GifCreator() {}

GifCreator::~GifCreator() {}

std::string GifCreator::createGif(const std::vector<std::string> &imagePaths,
                                  const std::string &outputPath,
                                  const GifOptions &options) {
  if (imagePaths.empty()) {
    std::cerr << "GifCreator: No images provided" << std::endl;
    return "";
  }

  // Tạo thư mục output nếu chưa tồn tại
  fs::path outPath(outputPath);
  if (outPath.has_parent_path()) {
    fs::create_directories(outPath.parent_path());
  }

  // Tạo thư mục tạm cho ảnh đã resize
  std::string tempDir = createTempDirectory();

  // Resize tất cả ảnh về cùng kích thước
  std::vector<std::string> resizedPaths =
      resizeImages(imagePaths, tempDir, options.width, options.height);

  if (resizedPaths.empty()) {
    std::cerr << "GifCreator: Failed to resize images" << std::endl;
    cleanupTempDirectory(tempDir);
    return "";
  }

  bool success = false;

  // Thử ImageMagick trước
  if (isImageMagickAvailable()) {
    std::cout << "Using ImageMagick to create GIF..." << std::endl;
    success = createGifWithImageMagick(resizedPaths, outputPath, options);
  }
  // Fallback sang gifsicle
  else if (isGifsicleAvailable()) {
    std::cout << "Using gifsicle to create GIF..." << std::endl;
    success = createGifWithGifsicle(resizedPaths, outputPath, options);
  } else {
    std::cerr << "GifCreator: Neither ImageMagick nor gifsicle found!"
              << std::endl;
    std::cerr << "Please install ImageMagick: choco install imagemagick"
              << std::endl;
  }

  // Cleanup
  cleanupTempDirectory(tempDir);

  if (success && fs::exists(outputPath)) {
    std::cout << "GIF created successfully: " << outputPath << std::endl;
    return outputPath;
  }

  return "";
}

bool GifCreator::isImageMagickAvailable() {
  std::string output;
  int result = executeCommand("magick -version", output);
  return result == 0;
}

bool GifCreator::isGifsicleAvailable() {
  std::string output;
  int result = executeCommand("gifsicle --version", output);
  return result == 0;
}

std::vector<std::string>
GifCreator::resizeImages(const std::vector<std::string> &imagePaths,
                         const std::string &tempDir, int width, int height) {
  std::vector<std::string> resizedPaths;

  for (size_t i = 0; i < imagePaths.size(); i++) {
    const auto &path = imagePaths[i];

    // Đọc ảnh
    cv::Mat img = cv::imread(path);
    if (img.empty()) {
      std::cerr << "Failed to read image: " << path << std::endl;
      continue;
    }

    // Resize
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(width, height));

    // Lưu vào thư mục tạm
    std::ostringstream oss;
    oss << tempDir << "/frame_" << std::setfill('0') << std::setw(4) << i
        << ".jpg";
    std::string resizedPath = oss.str();

    if (cv::imwrite(resizedPath, resized)) {
      resizedPaths.push_back(resizedPath);
    } else {
      std::cerr << "Failed to write resized image: " << resizedPath
                << std::endl;
    }
  }

  return resizedPaths;
}

bool GifCreator::createGifWithImageMagick(
    const std::vector<std::string> &imagePaths, const std::string &outputPath,
    const GifOptions &options) {
  std::string command =
      buildImageMagickCommand(imagePaths, outputPath, options);
  std::string output;

  std::cout << "Executing: " << command << std::endl;
  int result = executeCommand(command, output);

  if (result != 0) {
    std::cerr << "ImageMagick failed: " << output << std::endl;
    return false;
  }

  return fs::exists(outputPath);
}

bool GifCreator::createGifWithGifsicle(
    const std::vector<std::string> &imagePaths, const std::string &outputPath,
    const GifOptions &options) {
  // gifsicle yêu cầu input là GIF, nên phải convert PNG->GIF trước
  // Đây là implementation đơn giản, có thể cải thiện

  std::vector<std::string> gifPaths;

  // Convert mỗi JPEG sang GIF riêng lẻ
  for (size_t i = 0; i < imagePaths.size(); i++) {
    std::string tempGif = imagePaths[i] + ".gif";
    std::ostringstream cmd;
    cmd << "magick \"" << imagePaths[i] << "\" \"" << tempGif << "\"";

    std::string output;
    if (executeCommand(cmd.str(), output) == 0) {
      gifPaths.push_back(tempGif);
    }
  }

  if (gifPaths.empty()) {
    return false;
  }

  // Merge các GIF lại
  std::ostringstream cmd;
  cmd << "gifsicle --delay=" << options.frameDelay
      << " --loop=" << options.loopCount;

  if (options.optimize) {
    cmd << " --optimize=3";
  }

  for (const auto &gif : gifPaths) {
    cmd << " \"" << gif << "\"";
  }

  cmd << " > \"" << outputPath << "\"";

  std::string output;
  int result = executeCommand(cmd.str(), output);

  // Cleanup temp GIFs
  for (const auto &gif : gifPaths) {
    fs::remove(gif);
  }

  return result == 0 && fs::exists(outputPath);
}

std::string
GifCreator::buildImageMagickCommand(const std::vector<std::string> &imagePaths,
                                    const std::string &outputPath,
                                    const GifOptions &options) {
  std::ostringstream cmd;

  // ImageMagick 7 uses 'magick' command
  cmd << "magick";

  // Delay between frames (in 1/100 seconds)
  cmd << " -delay " << options.frameDelay;

  // Loop count (0 = infinite)
  cmd << " -loop " << options.loopCount;

  // Add all input images
  for (const auto &path : imagePaths) {
    cmd << " \"" << path << "\"";
  }

  // Optimization
  if (options.optimize) {
    cmd << " -layers Optimize";
  }

  // Output file
  cmd << " \"" << outputPath << "\"";

  return cmd.str();
}

int GifCreator::executeCommand(const std::string &command,
                               std::string &output) {
  output.clear();

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return -1;
  }

  std::array<char, 128> buffer;
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    output += buffer.data();
  }

  int result = pclose(pipe);

#ifdef _WIN32
  // Windows pclose returns exit code directly
  return result;
#else
  // Unix pclose returns status that needs WEXITSTATUS
  return WEXITSTATUS(result);
#endif
}

std::string GifCreator::createTempDirectory() {
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch())
                       .count();

  std::ostringstream oss;
  oss << fs::temp_directory_path().string() << "/photobooth_gif_" << timestamp;

  std::string tempDir = oss.str();
  fs::create_directories(tempDir);

  return tempDir;
}

void GifCreator::cleanupTempDirectory(const std::string &dir) {
  try {
    if (fs::exists(dir)) {
      fs::remove_all(dir);
    }
  } catch (const std::exception &e) {
    std::cerr << "Failed to cleanup temp directory: " << e.what() << std::endl;
  }
}

} // namespace photobooth
