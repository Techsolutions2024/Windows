using System;

namespace PhotoBooth.Models
{
    public class EventProfile
    {
        public string Id { get; set; }
        public string EventName { get; set; }
        public string EventType { get; set; }
        public DateTime EventDate { get; set; }
        public string Venue { get; set; }
        
        // Photo Settings
        public int PhotoCount { get; set; } = 4;
        public int CountdownSeconds { get; set; } = 3;
        public int IntervalBetweenPhotos { get; set; } = 2;
        
        // Template
        public string TemplateId { get; set; }
        
        // Features
        public bool EnablePrint { get; set; } = true;
        public bool EnableShare { get; set; } = true;
        public bool EnableGreenScreen { get; set; } = false;
        public bool EnableGif { get; set; } = false;
        public bool EnableVideoMessage { get; set; } = false;
        
        // Green Screen
        public string GreenScreenBackgroundPath { get; set; }
        public int ChromaKeyTolerance { get; set; } = 50;
        
        // Camera Settings
        public CameraSettings CameraSettings { get; set; } = new CameraSettings();
        
        // Branding
        public string LogoPath { get; set; }
        public string WatermarkText { get; set; }
        
        // Sharing
        public bool CollectEmails { get; set; } = true;
        public string EmailSubject { get; set; }
        public string EmailBody { get; set; }
        
        // Print Settings
        public PrintSettings PrintSettings { get; set; } = new PrintSettings();
        
        // Storage
        public string PhotoStoragePath { get; set; }
        public bool BackupToCloud { get; set; } = false;
        public string CloudStorageProvider { get; set; }
    }

    public class PrintSettings
    {
        public string PrinterName { get; set; }
        public PaperSize PaperSize { get; set; } = PaperSize.Photo4x6;
        public PrintQuality Quality { get; set; } = PrintQuality.High;
        public int Copies { get; set; } = 1;
        public bool AutoPrint { get; set; } = false;
    }

    public enum PaperSize
    {
        Photo4x6,
        Photo5x7,
        Photo8x10,
        A4,
        Letter
    }

    public enum PrintQuality
    {
        Draft,
        Normal,
        High,
        Best
    }

    public class PrinterInfo
    {
        public string Name { get; set; }
        public string Status { get; set; }
        public bool IsDefault { get; set; }
        public bool IsOnline { get; set; }
    }

    public class PrinterStatus
    {
        public bool IsOnline { get; set; }
        public bool HasPaper { get; set; }
        public bool HasInk { get; set; }
        public int QueueCount { get; set; }
        public string StatusMessage { get; set; }
    }
}
