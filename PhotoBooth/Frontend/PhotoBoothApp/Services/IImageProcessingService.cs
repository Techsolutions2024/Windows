using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using PhotoBooth.Models;

namespace PhotoBooth.Services
{
    public interface IImageProcessingService
    {
        Task<BitmapImage> ApplyTemplateAsync(BitmapImage sourceImage, Template template);
        Task<BitmapImage> ApplyGreenScreenAsync(BitmapImage sourceImage, BitmapImage backgroundImage, int tolerance = 50);
        Task<BitmapImage> ApplyFilterAsync(BitmapImage sourceImage, PhotoFilter filter);
        Task<BitmapImage> CreateCollageAsync(BitmapImage[] images, Template template);
        Task<BitmapImage> ResizeImageAsync(BitmapImage sourceImage, int width, int height);
        Task<BitmapImage> CropImageAsync(BitmapImage sourceImage, int x, int y, int width, int height);
        Task<BitmapImage> AddTextOverlayAsync(BitmapImage sourceImage, string text, TextOverlaySettings settings);
        Task<BitmapImage> AddLogoAsync(BitmapImage sourceImage, BitmapImage logo, LogoPosition position, int size);
        Task<string> CreateGifAsync(BitmapImage[] images, int delayMs = 500);
        Task<bool> SaveImageAsync(BitmapImage image, string filePath, ImageFormat format = ImageFormat.JPEG);
    }

    public enum PhotoFilter
    {
        None,
        BlackAndWhite,
        Sepia,
        Vintage,
        Cool,
        Warm,
        HighContrast,
        Soft
    }

    public enum LogoPosition
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    }

    public enum ImageFormat
    {
        JPEG,
        PNG,
        BMP
    }

    public class TextOverlaySettings
    {
        public string FontFamily { get; set; } = "Arial";
        public int FontSize { get; set; } = 24;
        public string Color { get; set; } = "#FFFFFF";
        public int X { get; set; }
        public int Y { get; set; }
        public bool Bold { get; set; }
        public bool Italic { get; set; }
    }
}
