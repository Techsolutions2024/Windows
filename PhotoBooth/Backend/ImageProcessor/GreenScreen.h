#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace PhotoBooth {
namespace ImageProcessing {

class GreenScreenProcessor {
public:
    GreenScreenProcessor();
    ~GreenScreenProcessor();

    // Chroma key operations
    bool ApplyChromaKey(const cv::Mat& sourceImage, 
                       const cv::Mat& backgroundImage, 
                       cv::Mat& outputImage,
                       const cv::Scalar& keyColor = cv::Scalar(0, 255, 0),
                       int tolerance = 50);
    
    bool ApplyChromaKeyHSV(const cv::Mat& sourceImage,
                          const cv::Mat& backgroundImage,
                          cv::Mat& outputImage,
                          int hueMin = 40, int hueMax = 80,
                          int satMin = 40, int satMax = 255,
                          int valMin = 40, int valMax = 255);
    
    // Background operations
    bool LoadBackground(const std::string& backgroundPath);
    bool SetBackground(const cv::Mat& background);
    cv::Mat GetBackground() const;
    
    // Color detection
    cv::Scalar DetectKeyColor(const cv::Mat& image, int x, int y, int radius = 10);
    
    // Refinement
    bool RefineEdges(cv::Mat& mask, int kernelSize = 5);
    bool FeatherEdges(cv::Mat& mask, int featherAmount = 10);
    bool RemoveSpeckles(cv::Mat& mask, int minSize = 100);
    
    // Settings
    void SetTolerance(int tolerance);
    int GetTolerance() const;
    
    void SetFeatherAmount(int amount);
    int GetFeatherAmount() const;

private:
    cv::Mat backgroundImage_;
    int tolerance_;
    int featherAmount_;
    
    // Helper methods
    cv::Mat CreateMask(const cv::Mat& image, const cv::Scalar& keyColor, int tolerance);
    cv::Mat CreateMaskHSV(const cv::Mat& image, int hMin, int hMax, int sMin, int sMax, int vMin, int vMax);
    void ApplyMask(const cv::Mat& foreground, const cv::Mat& background, const cv::Mat& mask, cv::Mat& output);
};

} // namespace ImageProcessing
} // namespace PhotoBooth
