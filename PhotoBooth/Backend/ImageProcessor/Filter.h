#pragma once

#include <opencv2/opencv.hpp>

namespace PhotoBooth {
namespace ImageProcessing {

enum class FilterType {
    None,
    BlackAndWhite,
    Sepia,
    Vintage,
    Cool,
    Warm,
    HighContrast,
    Soft,
    Vignette,
    Blur,
    Sharpen
};

class FilterProcessor {
public:
    FilterProcessor();
    ~FilterProcessor();

    // Apply filters
    bool ApplyFilter(const cv::Mat& input, cv::Mat& output, FilterType filter);
    
    // Individual filters
    bool ApplyBlackAndWhite(const cv::Mat& input, cv::Mat& output);
    bool ApplySepia(const cv::Mat& input, cv::Mat& output);
    bool ApplyVintage(const cv::Mat& input, cv::Mat& output);
    bool ApplyCool(const cv::Mat& input, cv::Mat& output);
    bool ApplyWarm(const cv::Mat& input, cv::Mat& output);
    bool ApplyHighContrast(const cv::Mat& input, cv::Mat& output);
    bool ApplySoft(const cv::Mat& input, cv::Mat& output);
    bool ApplyVignette(const cv::Mat& input, cv::Mat& output, double strength = 0.5);
    bool ApplyBlur(const cv::Mat& input, cv::Mat& output, int kernelSize = 15);
    bool ApplySharpen(const cv::Mat& input, cv::Mat& output, double amount = 1.5);
    
    // Adjustments
    bool AdjustBrightness(const cv::Mat& input, cv::Mat& output, int value);
    bool AdjustContrast(const cv::Mat& input, cv::Mat& output, double value);
    bool AdjustSaturation(const cv::Mat& input, cv::Mat& output, double value);
    bool AdjustHue(const cv::Mat& input, cv::Mat& output, int value);
    
    // Color grading
    bool ApplyColorGrade(const cv::Mat& input, cv::Mat& output, 
                        const cv::Vec3b& shadows, 
                        const cv::Vec3b& midtones, 
                        const cv::Vec3b& highlights);
    
    // Custom LUT
    bool ApplyLUT(const cv::Mat& input, cv::Mat& output, const cv::Mat& lut);
    bool LoadLUT(const std::string& lutPath);

private:
    cv::Mat currentLUT_;
    
    // Helper methods
    cv::Mat CreateVignetteMask(int width, int height, double strength);
    cv::Mat CreateSepiaKernel();
};

} // namespace ImageProcessing
} // namespace PhotoBooth
