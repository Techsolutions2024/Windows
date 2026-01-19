# PhotoBooth Pro - Quick Start Guide

## 🚀 Get Started in 5 Minutes

### Step 1: Build the Project

#### Windows (Recommended)

**Backend:**
```bash
cd PhotoBooth/Backend
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

**Frontend:**
```bash
cd PhotoBooth/Frontend/PhotoBoothApp
dotnet restore
dotnet build --configuration Release
```

### Step 2: Connect Camera

1. Connect Canon DSLR via USB
2. Turn on camera
3. Set to PC Remote mode (if available)

### Step 3: Run Application

```bash
cd PhotoBooth/Frontend/PhotoBoothApp
dotnet run
```

### Step 4: Start Photobooth

1. Click **"Connect Camera"**
2. Click **"Start Live View"**
3. Click **"Guest Display"** (opens full-screen)
4. Click **"Start Session"**
5. **Touch screen to begin!**

---

## 📋 Essential Features

### Camera Control
- ✅ Auto-detect Canon cameras
- ✅ Live view preview
- ✅ Remote capture
- ✅ Adjust ISO, Aperture, Shutter

### Photo Capture
- ✅ 1-4 photo sequences
- ✅ Countdown timer (0-10s)
- ✅ Auto-save photos

### Templates
- ✅ Custom templates
- ✅ Photo zones
- ✅ Text overlays
- ✅ Logo/graphics

### Green Screen
- ✅ Chroma key
- ✅ Background replacement
- ✅ Edge refinement

### Printing
- ✅ Direct printing
- ✅ Multiple printers
- ✅ Paper size selection

### Sharing
- ✅ Email with photos
- ✅ QR code download
- ✅ Social media

---

## 🎯 Quick Tips

### Before Event
1. ✅ Test camera connection
2. ✅ Test printing
3. ✅ Prepare templates
4. ✅ Check lighting

### During Event
1. ✅ Monitor battery
2. ✅ Check printer paper
3. ✅ Assist guests
4. ✅ Backup photos

### After Event
1. ✅ Export photos
2. ✅ Generate report
3. ✅ Backup everything
4. ✅ Clean equipment

---

## 🔧 Common Issues

### Camera Not Found
```
Solution:
1. Check USB connection
2. Close EOS Utility
3. Restart PhotoBooth Pro
```

### Live View Not Working
```
Solution:
1. Set camera to PC Remote
2. Use P/Av/Tv/M mode
3. Disable auto power-off
```

### Printer Not Working
```
Solution:
1. Check printer power
2. Update drivers
3. Clear print queue
```

---

## 📁 File Locations

| Type | Location |
|------|----------|
| Photos | `C:\PhotoBoothPro\Photos\` |
| Templates | `C:\PhotoBoothPro\Templates\` |
| Settings | `C:\PhotoBoothPro\Settings\` |
| Logs | `C:\PhotoBoothPro\Logs\` |

---

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Connect Camera |
| `Ctrl+L` | Start Live View |
| `Space` | Capture Photo |
| `F11` | Fullscreen |
| `Esc` | Exit Fullscreen |

---

## 📚 Documentation

- **README.md** - Project overview
- **BUILD.md** - Build instructions
- **USER_GUIDE.md** - Complete manual
- **FEATURES.md** - Feature list
- **HUONG_DAN_TIENG_VIET.md** - Vietnamese guide

---

## 🎨 Creating Templates

1. Settings > Templates > New Template
2. Set size (4x6, 5x7, etc.)
3. Add background image
4. Add photo zones (drag & resize)
5. Add text (event name, date)
6. Add logo/graphics
7. Preview & Save

---

## 🖨️ Printer Setup

1. Settings > Printer
2. Detect Printers
3. Select printer
4. Choose paper size
5. Set quality
6. Test print

---

## 📧 Support

- **Email**: support@photoboothpro.com
- **Website**: https://photoboothpro.com
- **Docs**: https://docs.photoboothpro.com

---

## ✅ Checklist

### Pre-Event
- [ ] Camera connected
- [ ] Live view working
- [ ] Template selected
- [ ] Printer configured
- [ ] Test capture
- [ ] Test print
- [ ] Lighting checked
- [ ] Props ready

### During Event
- [ ] Session started
- [ ] Guest display open
- [ ] Monitor battery
- [ ] Check printer
- [ ] Assist guests
- [ ] Backup photos

### Post-Event
- [ ] Export photos
- [ ] Generate report
- [ ] Backup files
- [ ] Clean equipment
- [ ] Review logs

---

**Ready to create amazing photobooth experiences!** 🎉

For detailed information, see **USER_GUIDE.md**
