using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace PhotoBooth.Services
{
    public interface ISharingService
    {
        Task<string> UploadToCloudAsync(string filePath);
        Task<BitmapImage> GenerateQRCodeAsync(string url);
        Task<bool> SendEmailAsync(string toEmail, string subject, string body, string attachmentPath);
        Task<bool> SendSMSAsync(string phoneNumber, string message);
        Task<string> ShortenUrlAsync(string longUrl);
        Task<bool> ShareToSocialMediaAsync(string platform, string filePath, string caption);
    }
}
