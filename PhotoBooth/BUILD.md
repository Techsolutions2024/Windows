# PhotoBooth Pro - Build Instructions

## Prerequisites

### Windows
- Visual Studio 2022 (Community Edition or higher)
- .NET 6.0 SDK or higher
- CMake 3.15 or higher
- Canon EDSDK 13.19.10 (included)
- OpenCV 4.x (optional, for advanced image processing)

### macOS
- Xcode 14 or higher
- .NET 6.0 SDK or higher
- CMake 3.15 or higher
- Canon EDSDK 13.19.10 (included)
- OpenCV 4.x (optional)

### Linux
- GCC 9 or higher / Clang 10 or higher
- .NET 6.0 SDK or higher
- CMake 3.15 or higher
- Canon EDSDK 13.19.10 (included)
- OpenCV 4.x (optional)

## Building the C++ Backend

### Windows

```bash
# Navigate to backend directory
cd PhotoBooth/Backend

# Create build directory
mkdir build
cd build

# Generate Visual Studio solution
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Or open PhotoBoothBackend.sln in Visual Studio and build
```

### macOS

```bash
cd PhotoBooth/Backend
mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4

# Install
sudo make install
```

### Linux

```bash
cd PhotoBooth/Backend
mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4

# Install
sudo make install
```

## Building the C# Frontend

### Windows

```bash
# Navigate to frontend directory
cd PhotoBooth/Frontend/PhotoBoothApp

# Restore NuGet packages
dotnet restore

# Build
dotnet build --configuration Release

# Run
dotnet run --configuration Release

# Or publish standalone executable
dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true
```

### macOS

```bash
cd PhotoBooth/Frontend/PhotoBoothApp

dotnet restore
dotnet build --configuration Release

# Publish for macOS
dotnet publish -c Release -r osx-x64 --self-contained true
```

### Linux

```bash
cd PhotoBooth/Frontend/PhotoBoothApp

dotnet restore
dotnet build --configuration Release

# Publish for Linux
dotnet publish -c Release -r linux-x64 --self-contained true
```

## Installing OpenCV (Optional)

### Windows

1. Download OpenCV from https://opencv.org/releases/
2. Extract to C:\opencv
3. Add to system PATH: C:\opencv\build\x64\vc16\bin
4. Set environment variable: OPENCV_DIR=C:\opencv\build

### macOS

```bash
brew install opencv
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install libopencv-dev
```

## Project Structure After Build

```
PhotoBooth/
├── Backend/
│   └── build/
│       ├── Release/
│       │   ├── EDSDKWrapper.dll (Windows) / .so (Linux) / .dylib (macOS)
│       │   ├── ImageProcessor.dll/.so/.dylib
│       │   ├── PrintManager.dll (Windows only)
│       │   ├── IPCServer.dll/.so/.dylib
│       │   ├── EDSDK.dll (Windows)
│       │   └── PhotoBoothBackend.exe/.out
│       └── ...
├── Frontend/
│   └── PhotoBoothApp/
│       └── bin/Release/net6.0-windows/
│           ├── PhotoBoothPro.exe
│           ├── PhotoBoothPro.dll
│           └── ... (dependencies)
└── ...
```

## Creating Installer

### Windows (Using Inno Setup)

1. Install Inno Setup from https://jrsoftware.org/isinfo.php
2. Create installer script:

```iss
[Setup]
AppName=PhotoBooth Pro
AppVersion=1.0.0
DefaultDirName={pf}\PhotoBoothPro
DefaultGroupName=PhotoBooth Pro
OutputDir=Installer
OutputBaseFilename=PhotoBoothProSetup

[Files]
Source: "Frontend\PhotoBoothApp\bin\Release\net6.0-windows\*"; DestDir: "{app}"; Flags: recursesubdirs
Source: "Backend\build\Release\*.dll"; DestDir: "{app}\Backend"
Source: "Backend\build\Release\*.exe"; DestDir: "{app}\Backend"

[Icons]
Name: "{group}\PhotoBooth Pro"; Filename: "{app}\PhotoBoothPro.exe"
Name: "{commondesktop}\PhotoBooth Pro"; Filename: "{app}\PhotoBoothPro.exe"

[Run]
Filename: "{app}\PhotoBoothPro.exe"; Description: "Launch PhotoBooth Pro"; Flags: postinstall nowait skipifsilent
```

3. Compile with Inno Setup

### macOS (Creating .app bundle)

```bash
# Create app structure
mkdir -p PhotoBoothPro.app/Contents/MacOS
mkdir -p PhotoBoothPro.app/Contents/Resources

# Copy binaries
cp -r Frontend/PhotoBoothApp/bin/Release/net6.0/* PhotoBoothPro.app/Contents/MacOS/
cp -r Backend/build/*.dylib PhotoBoothPro.app/Contents/MacOS/

# Create Info.plist
cat > PhotoBoothPro.app/Contents/Info.plist << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>PhotoBoothPro</string>
    <key>CFBundleIdentifier</key>
    <string>com.photobooth.pro</string>
    <key>CFBundleName</key>
    <string>PhotoBooth Pro</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
</dict>
</plist>
EOF

# Create DMG
hdiutil create -volname "PhotoBooth Pro" -srcfolder PhotoBoothPro.app -ov -format UDZO PhotoBoothPro.dmg
```

## Troubleshooting

### EDSDK Not Found

Ensure EDSDK files are in the correct location:
- Windows: `EDSDK/` or `EDSDK_64/` directory
- macOS: `EDSDK/Framework/` directory
- Linux: `EDSDK/Library/` directory

### OpenCV Not Found

If OpenCV is not found during CMake configuration:
- Set `OpenCV_DIR` environment variable to OpenCV installation path
- Or disable OpenCV features by commenting out OpenCV-dependent code

### .NET Runtime Not Found

Install .NET 6.0 Runtime from:
https://dotnet.microsoft.com/download/dotnet/6.0

### Camera Not Detected

1. Ensure camera is connected and powered on
2. Install Canon camera drivers
3. Close other camera applications (EOS Utility, etc.)
4. Try different USB port
5. Check Windows Device Manager for camera device

## Development Setup

### Visual Studio Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools
- C# (Microsoft)
- .NET Core Tools

### Visual Studio

1. Open `Backend/build/PhotoBoothBackend.sln` for C++ backend
2. Open `Frontend/PhotoBoothApp/PhotoBoothApp.csproj` for C# frontend

## Testing

### Backend Tests

```bash
cd Backend/build
ctest --output-on-failure
```

### Frontend Tests

```bash
cd Frontend/PhotoBoothApp
dotnet test
```

## Performance Optimization

### Release Build

Always use Release configuration for production:
- C++: `-DCMAKE_BUILD_TYPE=Release`
- C#: `--configuration Release`

### Compiler Optimizations

For C++ backend, enable optimizations:
- Windows: `/O2 /Oi /Ot /GL`
- GCC/Clang: `-O3 -march=native`

## Deployment Checklist

- [ ] Build backend in Release mode
- [ ] Build frontend in Release mode
- [ ] Test camera connection
- [ ] Test live view
- [ ] Test photo capture
- [ ] Test template overlay
- [ ] Test printing (if enabled)
- [ ] Test sharing features
- [ ] Create installer package
- [ ] Test installer on clean system
- [ ] Prepare user documentation
- [ ] Create backup/restore procedures

## Support

For build issues, please check:
1. All prerequisites are installed
2. EDSDK files are in correct location
3. Environment variables are set correctly
4. Firewall/antivirus not blocking
5. Latest Visual Studio/Xcode updates installed
