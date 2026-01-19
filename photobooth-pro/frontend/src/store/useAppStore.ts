import { create } from 'zustand'
import { AppSettings, Camera, Event, Photo } from '../types'

interface AppState {
  // Events
  events: Event[]
  currentEvent: Event | null
  
  // Photos
  photos: Photo[]
  currentPhoto: Photo | null
  
  // Cameras
  cameras: Camera[]
  selectedCamera: Camera | null
  
  // Settings
  settings: AppSettings
  
  // Live view
  liveViewActive: boolean
  liveViewFrame: string | null
  
  // Capture state
  isCapturing: boolean
  countdown: number
  
  // Actions
  setEvents: (events: Event[]) => void
  setCurrentEvent: (event: Event | null) => void
  addEvent: (event: Event) => void
  deleteEvent: (eventId: number) => void
  
  setPhotos: (photos: Photo[]) => void
  setCurrentPhoto: (photo: Photo | null) => void
  addPhoto: (photo: Photo) => void
  
  setCameras: (cameras: Camera[]) => void
  setSelectedCamera: (camera: Camera | null) => void
  
  updateSettings: (settings: Partial<AppSettings>) => void
  
  setLiveViewActive: (active: boolean) => void
  setLiveViewFrame: (frame: string | null) => void
  
  setIsCapturing: (capturing: boolean) => void
  setCountdown: (count: number) => void
}

export const useAppStore = create<AppState>((set) => ({
  // Initial state
  events: [],
  currentEvent: null,
  photos: [],
  currentPhoto: null,
  cameras: [],
  selectedCamera: null,
  settings: {
    selectedCamera: null,
    cameraSettings: {
      iso: 400,
      aperture: 'f/5.6',
      shutterSpeed: '1/125',
      whiteBalance: 'Auto',
      mirror: false,
      rotation: 0,
    },
    countdownDuration: 3,
    captureMode: 'photo',
    autoPreview: true,
    autoPrint: false,
    printCopies: 1,
    screenOrientation: 'landscape',
  },
  liveViewActive: false,
  liveViewFrame: null,
  isCapturing: false,
  countdown: 0,

  // Actions
  setEvents: (events) => set({ events }),
  setCurrentEvent: (event) => set({ currentEvent: event }),
  addEvent: (event) => set((state) => ({ events: [...state.events, event] })),
  deleteEvent: (eventId) => set((state) => ({
    events: state.events.filter((e) => e.id !== eventId),
  })),

  setPhotos: (photos) => set({ photos }),
  setCurrentPhoto: (photo) => set({ currentPhoto: photo }),
  addPhoto: (photo) => set((state) => ({ photos: [...state.photos, photo] })),

  setCameras: (cameras) => set({ cameras }),
  setSelectedCamera: (camera) => set({ selectedCamera: camera }),

  updateSettings: (newSettings) => set((state) => ({
    settings: { ...state.settings, ...newSettings },
  })),

  setLiveViewActive: (active) => set({ liveViewActive: active }),
  setLiveViewFrame: (frame) => set({ liveViewFrame: frame }),

  setIsCapturing: (capturing) => set({ isCapturing: capturing }),
  setCountdown: (count) => set({ countdown: count }),
}))
