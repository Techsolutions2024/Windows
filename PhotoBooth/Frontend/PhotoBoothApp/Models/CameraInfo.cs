namespace PhotoBooth.Models
{
    public class CameraInfo
    {
        public string ModelName { get; set; }
        public string SerialNumber { get; set; }
        public string FirmwareVersion { get; set; }
        public int BatteryLevel { get; set; }
        public string LensName { get; set; }
        public bool IsConnected { get; set; }
    }

    public class CameraSettings
    {
        public string ISO { get; set; }
        public string Aperture { get; set; }
        public string ShutterSpeed { get; set; }
        public string WhiteBalance { get; set; }
        public string ImageQuality { get; set; }
        public int SaveTo { get; set; }
    }
}
