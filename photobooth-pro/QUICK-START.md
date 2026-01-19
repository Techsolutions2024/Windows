# 🚀 Quick Start Guide - Photobooth Pro

## 📋 Prerequisites

- Node.js 18+ 
- npm or yarn
- CMake 3.15+ (for C++ backend)
- Visual Studio 2019+ or MinGW (Windows)
- Canon EDSDK 13.19.10 (already included)

## ⚡ Quick Start (Frontend Only)

### 1. Install Dependencies

```bash
cd photobooth-pro/frontend
npm install
```

### 2. Run Development Server

```bash
npm run dev
```

The app will open at `http://localhost:5173`

### 3. Available Routes

- **`/`** - Event Management Dashboard
- **`/mode-selection`** - Start Screen (Mode Selection) ✨ NEW
- **`/capture`** - Live Capture Screen
- **`/print-result`** - Photo Result & Sharing ✨ NEW
- **`/settings`** - Settings Page

## 🎯 Testing New Features

### Test Mode Selection Page

1. Navigate to `http://localhost:5173/mode-selection`
2. You should see:
   - 4 large circular buttons (Print, GIF, Boomerang, Video)
   - Logo placeholder (top left)
   - QR code button (top right, red)
   - Settings button (top right, blue)
   - Animated entrance
   - Pulsing footer text

3. Click any mode button → Should navigate to capture page with mode parameter

### Test Print Result Page

1. Navigate to `http://localhost:5173/print-result`
2. You should see:
   - 2x2 photo collage (tilted frame effect)
   - Event header with branding
   - Right sidebar with 3 share options:
     - Print Photo (primary blue button)
     - Send to Email
     - Scan QR Code
   - Bottom buttons: Retake All, Done
   - Kiosk ID in bottom left

3. Click "Scan QR Code" → QR modal should appear
4. Click outside modal or X button → Modal should close

### Test QR Code Modal

1. From Print Result page, click "Scan QR Code"
2. You should see:
   - Blurred backdrop
   - White modal with QR code
   - Large scannable QR code (320x320px)
   - Fallback URL: "pix.io/ABCD-123"
   - Done button
   - Close button (top right)
   - Station info at bottom

3. Test interactions:
   - Click Done → Modal closes
   - Click X → Modal closes
   - Click backdrop → Modal closes

### Test Photo Strip Tray (Component)

This component will be integrated into CapturePage. To test standalone:

```tsx
// In CapturePage.tsx, add:
import PhotoStripTray from '../components/PhotoStripTray';

const photos = [
  { id: 1, imageUrl: 'https://picsum.photos/400/300?random=1', status: 'filled' },
  { id: 2, imageUrl: 'https://picsum.photos/400/300?random=2', status: 'capturing' },
  { id: 3, status: 'empty' },
  { id: 4, status: 'empty' },
];

// Add to render:
<PhotoStripTray photos={photos} totalSlots={4} />
```

## 🎨 Theme Testing

### Test Sky Blue Theme

All pages should use Sky Blue (#0ea5e9) as primary color:

- Mode Selection buttons hover → Sky Blue
- Print Result "Print Photo" button → Sky Blue background
- QR Modal "Done" button → Sky Blue background
- Photo Strip active slot → Sky Blue border with glow

### Test Dark Mode

1. Click theme toggle button (bottom right on Mode Selection page)
2. All pages should switch to dark theme
3. Colors should remain consistent

## 🔧 Troubleshooting

### TypeScript Errors

If you see TypeScript errors about missing modules:

```bash
cd photobooth-pro/frontend
npm install framer-motion react-router-dom qrcode.react @types/react @types/react-dom
```

### Port Already in Use

If port 5173 is busy:

```bash
# Kill the process or use different port
npm run dev -- --port 3000
```

### Build Errors

```bash
# Clean install
rm -rf node_modules package-lock.json
npm install
npm run dev
```

## 📱 Mobile Testing

### Responsive Design

1. Open DevTools (F12)
2. Toggle device toolbar (Ctrl+Shift+M)
3. Test on different screen sizes:
   - Mobile: 375x667 (iPhone SE)
   - Tablet: 768x1024 (iPad)
   - Desktop: 1920x1080

### Touch Testing

Mode Selection page is optimized for touch:
- Large buttons (128x128px on desktop)
- Touch-friendly spacing
- Tap animations (scale down on press)

## 🎬 Animation Testing

### Mode Selection Animations

- Buttons should fade in sequentially (0.1s delay each)
- Hover → Scale up slightly
- Tap → Scale down
- Footer text → Pulse animation

### Print Result Animations

- Collage → Fade in with rotation
- Photos → Stagger animation (0.1s delay each)
- Buttons → Hover scale effect

### QR Modal Animations

- Backdrop → Fade in
- Modal → Fade + scale + slide up
- Exit → Reverse animation

## 🐛 Known Issues

1. **TypeScript Errors**: Will resolve after npm install completes
2. **Placeholder Images**: Using Unsplash/Picsum - replace with real photos
3. **Backend Not Connected**: Frontend works standalone, backend integration pending

## 📊 Performance Testing

### Check Animation Performance

1. Open DevTools → Performance tab
2. Record while navigating between pages
3. Check for 60fps animations
4. Look for layout shifts

### Check Bundle Size

```bash
npm run build
npm run preview
```

Check `dist/` folder size - should be < 1MB for initial load.

## 🔗 Integration Testing

### Test Navigation Flow

1. Start at `/` (Events Page)
2. Click "Launch Event" → Should go to `/mode-selection`
3. Click "Print" mode → Should go to `/capture?mode=photo`
4. After capture → Should go to `/print-result`
5. Click "Done" → Should go back to `/mode-selection`

### Test State Management

Check Zustand store:

```tsx
// In browser console
window.__ZUSTAND_STORE__
```

## 🎯 Next Steps

After testing frontend:

1. **Backend Integration**
   - Build C++ backend
   - Test Canon SDK connection
   - Test WebSocket communication

2. **Camera Testing**
   - Connect Canon R100
   - Test live view streaming
   - Test photo capture

3. **Print Testing**
   - Connect thermal printer
   - Test print queue
   - Test print quality

## 📚 Documentation

- [HTML Conversion Plan](./HTML-TO-REACT-PLAN.md)
- [Conversion Complete](./HTML-CONVERSION-COMPLETE.md)
- [Architecture](./ARCHITECTURE.md)
- [Setup Guide](./SETUP.md)

## 🆘 Support

If you encounter issues:

1. Check console for errors (F12)
2. Check network tab for failed requests
3. Check React DevTools for component state
4. Review error messages carefully

## ✅ Checklist

Before moving to backend integration:

- [ ] All routes load without errors
- [ ] Mode Selection page displays correctly
- [ ] Print Result page displays correctly
- [ ] QR Modal opens and closes
- [ ] Animations are smooth (60fps)
- [ ] Dark mode works
- [ ] Responsive on mobile
- [ ] No console errors
- [ ] TypeScript compiles without errors

---

**Ready to test?** Run `npm run dev` and start exploring! 🚀
