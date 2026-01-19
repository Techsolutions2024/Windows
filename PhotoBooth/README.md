# Canon PhotoBooth Pro - Event Photobooth Software

Professional photobooth software for events using Canon DSLR cameras with EDSDK integration.

## Features

### Core Photobooth Features
- **Live View Display** - Real-time camera preview on large screen
- **Multi-Shot Sequences** - 1-4 photo sequences with customizable intervals
- **Countdown Timer** - Visual and audio countdown before capture
- **Template Overlays** - Custom frames, borders, and event branding
- **Green Screen** - Chroma key background replacement
- **Touch Screen Support** - Full touch interface for events
- **Instant Printing** - Direct print to photo printers
- **Social Media Sharing** - Email, SMS, QR code sharing
- **Photo Gallery** - Real-time event gallery display
- **Dual Monitor Support** - Operator screen + guest display

### Advanced Features
- **Custom Branding** - Event logos, sponsor branding
- **Photo Filters** - Black & white, sepia, vintage effects
- **GIF/Boomerang Creation** - Animated photo sequences
- **Video Messages** - Record short video clips
- **Data Collection** - Email capture for marketing
- **Analytics Dashboard** - Session statistics and reports
- **Remote Management** - Web-based configuration
- **Backup System** - Auto-backup to cloud storage

## Architecture

### Backend (C++)
- **EDSDKWrapper** - Canon camera control library
- **ImageProcessor** - OpenCV-based image processing
- **PrintManager** - Windows print API integration
- **NetworkServer** - Communication with frontend

### Frontend (C# WPF)
- **MainWindow** - Operator control interface
- **GuestDisplay** - Full-screen photobooth interface
- **TemplateEditor** - Design custom overlays
- **ConfigManager** - Event settings and configuration
- **GalleryViewer** - Real-time photo gallery

### Communication
- Named pipes for IPC between C++ backend and C# frontend
- REST API for remote management
- WebSocket for real-time updates

## System Requirements

### Hardware
- Canon DSLR camera (EOS R series, 5D/6D/7D series, 90D, etc.)
- Windows 10/11 PC (64-bit)
- 8GB RAM minimum (16GB recommended)
- USB 3.0 port for camera connection
- Touch screen display (optional)
- Photo printer (optional)

### Software
- Windows 10/11 (64-bit)
- .NET 6.0 or higher
- Visual C++ Redistributable 2022
- Canon EDSDK 13.19.10

## Installation

1. Install prerequisites:
   - .NET 6.0 Runtime
   - Visual C++ Redistributable 2022

2. Run PhotoBoothSetup.exe

3. Configure camera settings:
   - Connect Canon camera via USB
   - Set camera to PC Remote mode
   - Configure save destination

4. Set up event:
   - Create event profile
   - Upload templates/overlays
   - Configure printer (if using)
   - Test capture sequence

## Project Structure

```
PhotoBooth/
├── Backend/                    # C++ Backend
│   ├── EDSDKWrapper/          # EDSDK C++ wrapper
│   │   ├── Camera.h/cpp       # Camera control
│   │   ├── LiveView.h/cpp     # Live view management
│   │   ├── Capture.h/cpp      # Photo capture
│   │   └── FileManager.h/cpp  # File operations
│   ├── ImageProcessor/        # Image processing
│   │   ├── Overlay.h/cpp      # Template overlays
│   │   ├── GreenScreen.h/cpp  # Chroma key
│   │   ├── Filter.h/cpp       # Photo filters
│   │   └── GifCreator.h/cpp   # GIF generation
│   ├── PrintManager/          # Print integration
│   │   └── Printer.h/cpp      # Print operations
│   └── Server/                # Communication server
│       └── IPCServer.h/cpp    # Named pipe server
├── Frontend/                   # C# WPF Frontend
│   ├── PhotoBoothApp/         # Main application
│   │   ├── MainWindow.xaml    # Operator interface
│   │   ├── GuestDisplay.xaml  # Guest interface
│   │   ├── ViewModels/        # MVVM view models
│   │   └── Services/          # Business logic
│   ├── TemplateEditor/        # Template designer
│   ├── ConfigManager/         # Configuration UI
│   └── Resources/             # Images, templates
├── Shared/                     # Shared code
│   ├── Models/                # Data models
│   └── Protocol/              # Communication protocol
├── EDSDK/                     # Canon EDSDK
│   ├── Header/                # EDSDK headers
│   ├── Dll/                   # EDSDK DLLs
│   └── Library/               # EDSDK libs
└── Docs/                      # Documentation
    ├── UserGuide.md           # User manual
    ├── API.md                 # API documentation
    └── Templates.md           # Template guide
```

## Usage

### Starting a Photobooth Session

1. **Launch Application**
   - Open PhotoBooth Pro
   - Connect camera
   - Select event profile

2. **Configure Session**
   - Choose photo sequence (1-4 shots)
   - Select template/overlay
   - Set countdown duration
   - Enable/disable printing
   - Configure sharing options

3. **Start Photobooth**
   - Click "Start Session"
   - Guest display shows attract screen
   - Touch screen to begin

4. **Photo Capture Flow**
   - Guest touches "Start"
   - Countdown begins (3-2-1)
   - Camera captures photo
   - Repeat for multi-shot sequences
   - Show preview with overlay
   - Print and/or share options

### Creating Custom Templates

1. Open Template Editor
2. Import background image (1920x1080 or 2400x1600)
3. Add photo zones (positions for captured photos)
4. Add text, logos, graphics
5. Save template
6. Assign to event profile

### Printer Setup

1. Go to Settings > Printer
2. Select printer from list
3. Configure paper size (4x6, 5x7, etc.)
4. Set print quality
5. Test print
6. Enable auto-print

## Configuration Files

### Event Profile (JSON)
```json
{
  "eventName": "Wedding Reception",
  "photoCount": 4,
  "countdownSeconds": 3,
  "templateId": "wedding-template-01",
  "enablePrint": true,
  "enableShare": true,
  "enableGreenScreen": false,
  "cameraSettings": {
    "iso": 400,
    "aperture": "f/5.6",
    "shutterSpeed": "1/125"
  }
}
```

## API Reference

### Backend API (Named Pipe)

```cpp
// Camera Control
bool ConnectCamera();
bool StartLiveView();
bool CapturePhoto();
bool StopLiveView();

// Image Processing
bool ApplyTemplate(string templatePath);
bool ApplyGreenScreen(string backgroundPath);
bool ApplyFilter(FilterType filter);

// Print
bool PrintPhoto(string photoPath);
```

### Frontend API (C#)

```csharp
// Session Management
void StartSession(EventProfile profile);
void EndSession();

// Capture Control
Task<CaptureResult> CaptureSequence(int photoCount);
Task<bool> ApplyOverlay(string templateId);

// Sharing
Task<bool> SendEmail(string email, string photoPath);
Task<string> GenerateQRCode(string photoPath);
```

## Troubleshooting

### Camera Not Detected
- Ensure camera is powered on
- Check USB connection
- Close other camera software (EOS Utility)
- Restart application

### Live View Not Working
- Set camera to PC Remote mode
- Check camera mode dial (use P, Av, Tv, or M)
- Disable camera auto-power off

### Print Issues
- Verify printer is online
- Check paper and ink levels
- Test print from Windows
- Update printer drivers

### Performance Issues
- Close unnecessary applications
- Reduce live view resolution
- Disable real-time filters
- Use SSD for photo storage

## Support

For technical support and updates:
- Email: support@photobooth.com
- Website: https://photobooth.com
- Documentation: https://docs.photobooth.com

## License

This software uses Canon EDSDK which is subject to Canon's SDK License Agreement.
PhotoBooth Pro is proprietary software. All rights reserved.

## Credits

- Canon Inc. for EDSDK
- OpenCV for image processing
- .NET Foundation for WPF framework
