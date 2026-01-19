# HTML to React Conversion Plan

## 📊 Overview
Converting 7 HTML files to React/TypeScript components with unified Sky Blue theme (#0ea5e9)

## 🎨 Theme Standardization

### Original Themes:
- **layout.html**: Pink (#f20da6) → Convert to Sky Blue
- **home.html**: Blue (#0091ff) → Convert to Sky Blue  
- **mode.html**: Sky Blue (#0ea5e9) ✓ Keep
- **liveview.html**: Sky Blue (#135bec) → Standardize to #0ea5e9
- **print.html**: Sky Blue (#135bec) → Standardize to #0ea5e9
- **qrcode.html**: Sky Blue (#135bec) → Standardize to #0ea5e9
- **config.html**: Blue (#0d59f2) → Convert to Sky Blue

### Unified Theme:
```js
colors: {
  primary: '#0ea5e9',        // Sky Blue
  'primary-dark': '#0284c7', // Darker Sky Blue
  'primary-light': '#38bdf8', // Lighter Sky Blue
  background: {
    dark: '#121212',
    surface: '#1e1e1e',
    light: '#f8fafc'
  },
  border: {
    dark: '#333333',
    light: '#e2e8f0'
  }
}
```

## 📁 File Mapping

### 1. Template Library (layout.html)
**New File**: `frontend/src/pages/TemplateLibraryPage.tsx`
**Features**:
- Sidebar navigation (All Templates, Weddings, Birthdays, Corporate)
- Template grid with hover effects
- Search bar
- Grid/List view toggle
- Import template button
- Storage indicator

**Components to Create**:
- `TemplateCard.tsx` - Individual template card
- `TemplateGrid.tsx` - Grid layout
- `TemplateSidebar.tsx` - Left sidebar navigation

### 2. Event Management (home.html)
**Update File**: `frontend/src/pages/EventsPage.tsx`
**Improvements**:
- Add toolbar (Select All, Rename, Delete, Duplicate, New event)
- Add sort/filter controls
- Better event card design with thumbnails
- Right sidebar with event configuration
- Start screen preview
- Capture mode selection (Photo/GIF/Boomerang/Video)

**Components to Create**:
- `EventToolbar.tsx` - Top action buttons
- `EventCard.tsx` - Improved event card
- `EventConfigSidebar.tsx` - Right sidebar

### 3. Mode Selection (mode.html)
**New File**: `frontend/src/pages/ModeSelectionPage.tsx`
**Features**:
- Full-screen start screen
- 4 large circular buttons (Print/GIF/Boomerang/Video)
- Logo placeholder
- QR code button
- Settings button
- Gradient background
- Touch-friendly design

**Components to Create**:
- `ModeButton.tsx` - Large circular mode button
- `StartScreenLayout.tsx` - Full-screen layout wrapper

### 4. Live Capture (liveview.html)
**Update File**: `frontend/src/pages/CapturePage.tsx`
**Improvements**:
- Add photo strip tray at bottom (4 slots)
- Flying thumbnail animation
- Countdown overlay on main view
- Session status indicator
- Better live view frame
- Decorative side elements

**Components to Create**:
- `PhotoStripTray.tsx` - Bottom 4-photo tray
- `FlyingThumbnail.tsx` - Animated thumbnail
- `PhotoSlot.tsx` - Individual photo slot
- `SessionStatus.tsx` - Top status bar

### 5. Print Result (print.html)
**New File**: `frontend/src/pages/PrintResultPage.tsx`
**Features**:
- 2x2 photo collage preview
- Blurred background
- Action sidebar (Print/Email/QR Code)
- Retake/Done buttons
- Event branding
- Kiosk ID display

**Components to Create**:
- `CollagePreview.tsx` - 2x2 photo grid
- `ShareActionSidebar.tsx` - Right sidebar with share options
- `CollageFrame.tsx` - White frame with rotation effect

### 6. QR Code Modal (qrcode.html)
**New File**: `frontend/src/components/QRCodeModal.tsx`
**Features**:
- Modal overlay with blur
- Large QR code display
- Fallback URL
- Auto-reset timer
- Station info
- Close button

**Components to Create**:
- `QRCodeDisplay.tsx` - QR code with frame
- `ModalOverlay.tsx` - Reusable modal wrapper

### 7. Settings (config.html)
**Update File**: `frontend/src/pages/SettingsPage.tsx`
**Improvements**:
- Left sidebar navigation (General/Display/Capture/Print/Social/Devices)
- Tabbed sections
- Calendar picker
- Toggle switches
- Range sliders
- Printer selection
- System status indicator

**Components to Create**:
- `SettingsSidebar.tsx` - Left navigation
- `SettingsSection.tsx` - Section wrapper
- `ToggleCard.tsx` - Toggle with icon
- `RangeSlider.tsx` - Custom slider
- `CalendarPicker.tsx` - Inline calendar

## 🔄 Conversion Steps

### Step 1: Update Theme (5 min)
```bash
✓ Update tailwind.config.js
✓ Add Sky Blue color palette
✓ Update global CSS
```

### Step 2: Create Shared Components (15 min)
```bash
□ PhotoStripTray.tsx
□ FlyingThumbnail.tsx
□ CollagePreview.tsx
□ QRCodeModal.tsx
□ ModalOverlay.tsx
□ TemplateCard.tsx
□ ModeButton.tsx
```

### Step 3: Create New Pages (30 min)
```bash
□ TemplateLibraryPage.tsx
□ ModeSelectionPage.tsx
□ PrintResultPage.tsx
```

### Step 4: Update Existing Pages (20 min)
```bash
□ EventsPage.tsx - Add toolbar, sidebar, better cards
□ CapturePage.tsx - Add photo strip tray, flying animation
□ SettingsPage.tsx - Add sidebar, sections, controls
```

### Step 5: Update Router (5 min)
```bash
□ Add /templates route
□ Add /mode-selection route
□ Add /print-result route
□ Update navigation
```

### Step 6: Testing (10 min)
```bash
□ Test all routes
□ Test responsive design
□ Test dark mode
□ Test animations
```

## 📦 New Dependencies Needed

```json
{
  "qrcode.react": "^3.1.0",        // QR code generation
  "framer-motion": "^10.16.16",    // Animations (already have)
  "react-calendar": "^4.6.1"       // Calendar picker (optional)
}
```

## 🎯 Priority Order

1. **High Priority** (Core Photobooth):
   - ModeSelectionPage (start screen)
   - CapturePage improvements (photo strip)
   - PrintResultPage (result screen)
   - QRCodeModal (sharing)

2. **Medium Priority** (Management):
   - EventsPage improvements
   - SettingsPage improvements

3. **Low Priority** (Extra Feature):
   - TemplateLibraryPage (differentiator)

## 📝 Notes

- All components use TypeScript
- All components use Tailwind CSS
- All components support dark mode
- All animations use Framer Motion
- All icons use Material Symbols
- All components are responsive
- All components follow existing patterns

## ⏱️ Estimated Time

- **Total**: ~90 minutes
- **Core Features**: ~45 minutes
- **Management Features**: ~30 minutes
- **Template Library**: ~15 minutes

## 🚀 Ready to Start?

Run these commands:
```bash
cd photobooth-pro/frontend
npm install qrcode.react
npm run dev
```

Then start converting files in priority order!
