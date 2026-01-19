#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace PhotoBooth {
namespace ImageProcessing {

struct PhotoZone {
    int x;
    int y;
    int width;
    int height;
    int index;
    bool hasBorder;
    cv::Scalar borderColor;
    int borderWidth;
    int rotation;
};

struct TextOverlay {
    std::string text;
    int x;
    int y;
    std::string fontFamily;
    int fontSize;
    cv::Scalar color;
    bool bold;
    bool italic;
    int rotation;
};

struct GraphicOverlay {
    std::string imagePath;
    int x;
    int y;
    int width;
    int height;
    int rotation;
    double opacity;
};

class OverlayProcessor {
public:
    OverlayProcessor();
    ~OverlayProcessor();

    // Template operations
    bool LoadTemplate(const std::string& templatePath);
    bool ApplyTemplate(const cv::Mat& sourceImage, cv::Mat& outputImage);
    
    // Photo zone operations
    bool AddPhotoToZone(const cv::Mat& photo, int zoneIndex);
    bool CreateCollage(const std::vector<cv::Mat>& photos, cv::Mat& outputImage);
    
    // Overlay operations
    bool AddTextOverlay(cv::Mat& image, const TextOverlay& text);
    bool AddGraphicOverlay(cv::Mat& image, const GraphicOverlay& graphic);
    bool AddLogo(cv::Mat& image, const cv::Mat& logo, int x, int y, int width, int height);
    
    // Utility functions
    bool ResizeImage(const cv::Mat& input, cv::Mat& output, int width, int height);
    bool RotateImage(const cv::Mat& input, cv::Mat& output, double angle);
    bool CropImage(const cv::Mat& input, cv::Mat& output, int x, int y, int width, int height);
    
    // Getters/Setters
    void SetPhotoZones(const std::vector<PhotoZone>& zones);
    std::vector<PhotoZone> GetPhotoZones() const;
    
    void SetBackgroundImage(const cv::Mat& background);
    cv::Mat GetBackgroundImage() const;

private:
    cv::Mat backgroundImage_;
    std::vector<PhotoZone> photoZones_;
    std::vector<cv::Mat> zonePhotos_;
    
    // Helper methods
    void DrawBorder(cv::Mat& image, const PhotoZone& zone);
    void BlendImages(const cv::Mat& src, cv::Mat& dst, int x, int y, double alpha);
};

} // namespace ImageProcessing
} // namespace PhotoBooth
