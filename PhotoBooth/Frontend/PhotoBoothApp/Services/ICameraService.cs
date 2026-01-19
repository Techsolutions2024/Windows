using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using PhotoBooth.Models;

namespace PhotoBooth.Services
{
    public interface ICameraService
    {
        event EventHandler<CameraConnectedEventArgs> CameraConnected;
        event EventHandler CameraDisconnected;
        event EventHandler<PhotoCapturedEventArgs> PhotoCaptured;
        event EventHandler<LiveViewFrameEventArgs> LiveViewFrameReceived;
        event EventHandler<CameraErrorEventArgs> ErrorOccurred;

        bool IsConnected { get; }
        bool IsLiveViewActive { get; }
        CameraInfo CurrentCamera { get; }

        Task<bool> InitializeAsync();
        Task<List<string>> GetCameraListAsync();
        Task<bool> ConnectCameraAsync(int cameraIndex = 0);
        Task<bool> DisconnectCameraAsync();

        Task<bool> StartLiveViewAsync();
        Task<bool> StopLiveViewAsync();
        Task<BitmapImage> GetLiveViewFrameAsync();

        Task<bool> TakePictureAsync();
        Task<bool> StartBulbCaptureAsync();
        Task<bool> EndBulbCaptureAsync();

        Task<CameraSettings> GetCurrentSettingsAsync();
        Task<bool> ApplySettingsAsync(CameraSettings settings);

        Task<List<string>> GetAvailableISOAsync();
        Task<List<string>> GetAvailableAperturesAsync();
        Task<List<string>> GetAvailableShutterSpeedsAsync();

        Task<bool> SetISOAsync(string iso);
        Task<bool> SetApertureAsync(string aperture);
        Task<bool> SetShutterSpeedAsync(string shutterSpeed);
        Task<bool> SetWhiteBalanceAsync(string whiteBalance);

        Task<int> GetBatteryLevelAsync();
        Task<int> GetAvailableShotsAsync();
    }

    public class CameraConnectedEventArgs : EventArgs
    {
        public CameraInfo Camera { get; set; }
    }

    public class PhotoCapturedEventArgs : EventArgs
    {
        public string FilePath { get; set; }
        public DateTime CaptureTime { get; set; }
    }

    public class LiveViewFrameEventArgs : EventArgs
    {
        public BitmapImage Frame { get; set; }
    }

    public class CameraErrorEventArgs : EventArgs
    {
        public string ErrorMessage { get; set; }
        public int ErrorCode { get; set; }
    }
}
