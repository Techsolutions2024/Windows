# PhotoBooth Pro - Project Overview

## Executive Summary

PhotoBooth Pro is a professional event photobooth software solution designed for Canon DSLR cameras. Built with a C++ backend for camera control and image processing, and a modern C# WPF frontend for user interaction, it provides a complete photobooth experience for weddings, parties, corporate events, and other occasions.

## Project Structure

```
PhotoBooth/
├── README.md                          # Main project documentation
├── BUILD.md                           # Build instructions
├── FEATURES.md                        # Complete feature list
├── USER_GUIDE.md                      # User manual
├── PROJECT_OVERVIEW.md                # This file
│
├── Backend/                           # C++ Backend
│   ├── CMakeLists.txt                # CMake build configuration
│   ├── main.cpp                      # Backend entry point
│   │
│   ├── EDSDKWrapper/                 # Canon EDSDK wrapper
│   │   ├── Camera.h                  # Camera control interface
│   │   └── Camera.cpp                # Camera implementation
│   │
│   ├── ImageProcessor/               # Image processing
│   │   ├── Overlay.h                 # Template overlay
│   │   ├── GreenScreen.h             # Chroma key
│   │   └── Filter.h                  # Photo filters
│   │
│   ├── PrintManager/                 # Print integration
│   │   └── Printer.h                 # Printer control
│   │
│   └── Server/                       # IPC server
│       └── IPCServer.h               # Named pipe server
│
├── Frontend/                          # C# WPF Frontend
│   └── PhotoBoothApp/
│       ├── PhotoBoothApp.csproj      # Project file
│       ├── App.xaml                  # Application definition
│       ├── App.xaml.cs               # Application code
│       │
│       ├── Views/                    # UI Views
│       │   ├── MainWindow.xaml       # Operator interface
│       │   ├── GuestDisplayWindow.xaml # Guest interface
│       │   ├── SettingsWindow.xaml   # Settings
│       │   └── TemplateEditorWindow.xaml # Template editor
│       │
│       ├── ViewModels/               # MVVM ViewModels
│       │   ├── MainViewModel.cs
│       │   ├── GuestDisplayViewModel.cs
│       │   └── SettingsViewModel.cs
│       │
│       ├── Services/                 # Business logic
│       │   ├── ICameraService.cs     # Camera service interface
│       │   ├── IImageProcessingService.cs
│       │   ├── IPrintService.cs
│       │   ├── ISharingService.cs
│       │   └── Implementations/
│       │
│       ├── Models/                   # Data models
│       │   ├── CameraInfo.cs
│       │   ├── Template.cs
│       │   └── EventProfile.cs
│       │
│       ├── Styles/                   # UI Styles
│       │   ├── Colors.xaml
│       │   ├── Buttons.xaml
│       │   └── Controls.xaml
│       │
│       └── Resources/                # Assets
│           ├── Templates/            # Template files
│           ├── Sounds/               # Audio files
│           └── Images/               # Image assets
│
├── EDSDK/                            # Canon EDSDK
│   ├── Header/                       # EDSDK headers
│   │   ├── EDSDK.h
│   │   ├── EDSDKTypes.h
│   │   └── EDSDKErrors.h
│   ├── Dll/                          # EDSDK DLLs (32-bit)
│   └── Library/                      # EDSDK libs
│
├── EDSDK_64/                         # Canon EDSDK (64-bit)
│   ├── Dll/
│   └── Library/
│
└── Docs/                             # Additional documentation
    ├── API.md                        # API documentation
    ├── Architecture.md               # Architecture details
    └── Templates.md                  # Template guide
```

## Technology Stack

### Backend (C++)
- **Language**: C++17
- **Build System**: CMake 3.15+
- **Camera SDK**: Canon EDSDK 13.19.10
- **Image Processing**: OpenCV 4.x
- **IPC**: Named Pipes (Windows) / Unix Sockets (Linux/macOS)
- **Compiler**: MSVC 2022 (Windows), GCC 9+ (Linux), Clang 10+ (macOS)

### Frontend (C#)
- **Language**: C# 10
- **Framework**: .NET 6.0
- **UI Framework**: WPF (Windows Presentation Foundation)
- **Architecture**: MVVM (Model-View-ViewModel)
- **Dependency Injection**: Microsoft.Extensions.DependencyInjection
- **MVVM Toolkit**: CommunityToolkit.Mvvm
- **JSON**: Newtonsoft.Json
- **QR Codes**: QRCoder

### Additional Libraries
- **Image Processing**: System.Drawing.Common
- **Printing**: Windows Print API
- **Email**: System.Net.Mail
- **Cloud Storage**: Provider-specific SDKs

## Architecture

### System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     PhotoBooth Pro                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │  C# WPF Frontend │◄───────►│  C++ Backend     │        │
│  │                  │  Named  │                  │        │
│  │  - Main Window   │  Pipes  │  - EDSDK Wrapper │        │
│  │  - Guest Display │         │  - Image Proc    │        │
│  │  - Settings      │         │  - Print Manager │        │
│  └──────────────────┘         └──────────────────┘        │
│         │                              │                   │
│         │                              │                   │
│         ▼                              ▼                   │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │   UI Services    │         │   Canon EDSDK    │        │
│  │                  │         │                  │        │
│  │  - Camera Svc    │         │  - Camera API    │        │
│  │  - Image Svc     │         │  - Live View     │        │
│  │  - Print Svc     │         │  - Capture       │        │
│  │  - Share Svc     │         │  - Properties    │        │
│  └──────────────────┘         └──────────────────┘        │
│         │                              │                   │
│         │                              │                   │
│         ▼                              ▼                   │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │   External       │         │   Canon Camera   │        │
│  │   Services       │         │                  │        │
│  │                  │         │   (USB)          │        │
│  │  - Email         │         │                  │        │
│  │  - Cloud Storage │         └──────────────────┘        │
│  │  - Printer       │                                     │
│  └──────────────────┘                                     │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Communication Flow

1. **Frontend → Backend**: Commands (connect, capture, settings)
2. **Backend → Frontend**: Events (photo captured, live view frame)
3. **Backend → Camera**: EDSDK API calls
4. **Camera → Backend**: Image data, events, status

### Data Flow

```
Camera → EDSDK → C++ Backend → Image Processing → C# Frontend → Display
                                      ↓
                                  Storage
                                      ↓
                              Print / Share
```

## Key Features

### 1. Camera Control
- Full Canon DSLR control via EDSDK
- Live view with 30 FPS
- Remote capture
- Adjustable settings (ISO, Aperture, Shutter)

### 2. Image Processing
- Template overlays
- Green screen / Chroma key
- Photo filters
- Collage creation
- GIF generation

### 3. User Interface
- Modern dark theme
- Touch-friendly design
- Dual monitor support
- Real-time preview
- Intuitive controls

### 4. Event Management
- Event profiles
- Custom branding
- Template library
- Session statistics

### 5. Printing & Sharing
- Direct photo printing
- Email sharing
- QR code generation
- Social media integration

## Development Workflow

### 1. Backend Development
```bash
cd Backend
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
./PhotoBoothBackend
```

### 2. Frontend Development
```bash
cd Frontend/PhotoBoothApp
dotnet restore
dotnet run
```

### 3. Testing
- Unit tests for backend (C++)
- Unit tests for frontend (C#)
- Integration tests
- Manual testing with camera

### 4. Debugging
- Visual Studio for C++ backend
- Visual Studio / VS Code for C# frontend
- Attach debugger to running process
- Log files for troubleshooting

## Deployment

### Windows Installer
- Inno Setup for installer creation
- Includes all dependencies
- Registry entries for file associations
- Start menu shortcuts

### Portable Version
- Self-contained executable
- No installation required
- All files in single folder
- USB drive compatible

### Updates
- Auto-update mechanism
- Check for updates on startup
- Download and install updates
- Rollback capability

## Performance Considerations

### Optimization
- Multi-threading for image processing
- Async operations for I/O
- Efficient memory management
- GPU acceleration (optional)

### Resource Usage
- CPU: Moderate (image processing)
- RAM: 2-4 GB typical
- Disk: 100 MB + photos
- Network: Minimal (sharing only)

## Security

### Data Protection
- Local storage only (no cloud by default)
- Optional encryption for sensitive data
- Secure email transmission
- GDPR compliance for email collection

### Camera Access
- Exclusive camera access
- Proper resource cleanup
- Error handling for disconnections

## Extensibility

### Plugin System (Future)
- Custom filters
- Custom templates
- Custom sharing providers
- Custom print layouts

### API (Future)
- REST API for remote control
- WebSocket for real-time updates
- Mobile app integration

## Testing Strategy

### Unit Tests
- Backend: Google Test
- Frontend: xUnit / NUnit

### Integration Tests
- Camera connection
- Image processing pipeline
- Print workflow
- Sharing workflow

### Manual Tests
- Full event simulation
- Multi-hour stress test
- Different camera models
- Different printers

## Known Limitations

1. **Camera Support**: Canon cameras only (EDSDK limitation)
2. **Platform**: Windows primary (macOS/Linux experimental)
3. **Printer**: Windows-compatible printers only
4. **Network**: Local network only (no internet required)

## Future Enhancements

### Short Term (v1.1)
- AI background removal
- Face detection
- Instagram integration
- More templates

### Medium Term (v1.2)
- Video booth mode
- 360° photo booth
- Gesture control
- Mobile app

### Long Term (v2.0)
- Cloud processing
- AI enhancement
- Virtual backgrounds
- NFT integration

## Contributing

### Code Style
- C++: Google C++ Style Guide
- C#: Microsoft C# Coding Conventions
- XAML: Standard WPF conventions

### Pull Requests
1. Fork repository
2. Create feature branch
3. Implement feature
4. Add tests
5. Submit PR

### Bug Reports
- Use GitHub Issues
- Include logs
- Describe steps to reproduce
- Include system information

## License

PhotoBooth Pro uses Canon EDSDK which is subject to Canon's SDK License Agreement.

The application code is proprietary. Contact for licensing information.

## Support

### Documentation
- README.md - Quick start
- BUILD.md - Build instructions
- USER_GUIDE.md - User manual
- FEATURES.md - Feature list

### Contact
- Email: support@photoboothpro.com
- Website: https://photoboothpro.com
- Forum: https://forum.photoboothpro.com

## Credits

### Development Team
- Backend: C++ EDSDK integration
- Frontend: C# WPF application
- Design: UI/UX design
- Testing: QA team

### Third-Party Libraries
- Canon EDSDK - Camera control
- OpenCV - Image processing
- .NET - Application framework
- WPF - UI framework

### Special Thanks
- Canon Inc. for EDSDK
- Open source community
- Beta testers
- Early adopters

---

**PhotoBooth Pro** - Professional Event Photobooth Software

Version 1.0.0 | © 2025 | All Rights Reserved
