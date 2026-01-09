# Canon Camera Control - Python

A modern Python GUI application for controlling Canon cameras using the Canon EDSDK.

## Features

- **Live View** - Real-time camera preview with overlay options (grid, focus point)
- **Remote Capture** - Take photos remotely with full shutter control
- **Camera Settings** - Control ISO, Aperture, Shutter Speed, White Balance, etc.
- **File Management** - Browse, download, and delete files from camera memory cards
- **Video Recording** - Start/stop video recording
- **Bulb Mode** - Manual bulb exposure control

## Requirements

- Python 3.8 or higher
- Windows 10/11 (64-bit recommended)
- Canon camera compatible with EDSDK
- Canon EDSDK 13.19.10 (included)

## Installation

1. Install Python dependencies:
```bash
pip install -r requirements.txt
```

2. Ensure the EDSDK DLLs are in place:
   - 64-bit: `EDSDK_64/Dll/EDSDK.dll`
   - 32-bit: `EDSDK/Dll/EDSDK.dll`

3. Run the application:
```bash
python main.py
```

## Usage

### Connecting Camera

1. Connect your Canon camera via USB
2. Turn on the camera
3. Click **File > Connect Camera** or press `Ctrl+O`

### Live View

1. Connect to camera
2. Click **Start Live View** button
3. Use Grid and Focus Point toggles for overlays
4. Click on the live view to set focus point

### Taking Photos

- Click the large **CAPTURE** button
- Or press **Space** key
- Use shutter controls for half-press (focus) and full-press

### Video Recording

1. Click **Movie Mode** to switch camera to video mode
2. Click **REC** button to start/stop recording

### Downloading Files

1. Go to the **Files** tab
2. Select a memory card
3. Select files and click **Download Selected** or **Download All**

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Connect Camera |
| `Space` | Capture |
| `Ctrl+L` | Start Live View |
| `F5` | Refresh Settings |

## Project Structure

```
CameraControlPython/
├── main.py              # Entry point
├── requirements.txt     # Python dependencies
├── edsdk/              # EDSDK Python wrapper
│   ├── __init__.py
│   ├── edsdk_types.py  # Type definitions
│   ├── edsdk_wrapper.py # DLL wrapper
│   └── camera.py       # High-level camera API
└── ui/                 # PyQt6 GUI components
    ├── __init__.py
    ├── main_window.py  # Main window
    ├── live_view_widget.py
    ├── settings_panel.py
    ├── file_browser.py
    └── styles.py       # Dark theme stylesheet
```

## Supported Cameras

This application supports all Canon cameras compatible with EDSDK 13.19.10, including:

- EOS R series (R, RP, R3, R5, R6, R7, R8, R10, R50, R100, etc.)
- EOS 5D/6D/7D series
- EOS 90D, 850D, etc.
- PowerShot G series
- And more...

See the EDSDK documentation for the full list.

## Troubleshooting

### Camera not detected
- Ensure camera is turned on and connected via USB
- Try a different USB port
- Check that no other application (like EOS Utility) is connected

### Live View not working
- Some cameras require specific settings for PC remote shooting
- Ensure camera is in a compatible mode (P, Av, Tv, M)

### DLL not found
- Ensure EDSDK DLLs are in the correct location
- For 64-bit Python, use EDSDK_64 DLLs

## License

This application uses the Canon EDSDK which is subject to Canon's SDK License Agreement.
The Python wrapper and GUI code are provided as-is for educational purposes.

## Acknowledgments

- Canon Inc. for the EDSDK
- PyQt6 for the GUI framework
