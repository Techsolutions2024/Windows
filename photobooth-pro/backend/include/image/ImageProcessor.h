#pragma once

#include <string>
#include <vector>

namespace photobooth {

enum class FilterType {
    None,
    BlackAndWhite,
    Sepia,
    Vintage,
    Pop,
    Retro,
    Cool,
    Warm
};

struct ImageEffect {
    FilterType filter = FilterType::None;
    float brightness = 0.0f;  // -1.0 to 1.0
    float contrast = 0.0f;    // -1.0 to 1.0
    float saturation = 0.0f;  // -1.0 to 1.0
};

struct Overlay {
    std::string imagePath;
    int x = 0;
    int y = 0;
    float opacity = 1.0f;
};

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    // Basic operations
    bool loadImage(const std::string& path);
    bool loadImage(const std::vector<uint8_t>& data);
    bool saveImage(const std::string& path);
    std::vector<uint8_t> getImageData();

    // Transformations
    void resize(int width, int height);
    void rotate(int degrees);
    void mirror(bool horizontal);
    void crop(int x, int y, int width, int height);

    // Effects
    void applyFilter(FilterType filter);
    void applyEffect(const ImageEffect& effect);
    void adjustBrightness(float value);
    void adjustContrast(float value);
    void adjustSaturation(float value);

    // Overlays
    void addOverlay(const Overlay& overlay);
    void addWatermark(const std::string& text, int x, int y, float opacity = 0.5f);
    void addLogo(const std::string& logoPath, int x, int y, float scale = 1.0f);

    // Photo strips
    bool createPhotoStrip(const std::vector<std::string>& imagePaths,
                         const std::string& outputPath,
                         int columns, int rows);

    // GIF/Boomerang
    bool createGIF(const std::vector<std::string>& imagePaths,
                   const std::string& outputPath,
                   int delayMs = 100);
    bool createBoomerang(const std::vector<std::string>& imagePaths,
                        const std::string& outputPath);

    // Utilities
    int getWidth() const;
    int getHeight() const;
    void clear();

private:
    void* imageData_; // cv::Mat* or similar
    int width_;
    int height_;

    void applyBlackAndWhite();
    void applySepia();
    void applyVintage();
    void applyPop();
    void applyRetro();
};

} // namespace photobooth
