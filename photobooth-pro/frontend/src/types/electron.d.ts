// Types for Electron IPC API exposed via preload

export interface EventSettings {
  eventId: string
  eventName: string
  createdAt: string
  currentLayout: string
  photoCount: number
}

export interface SlotInfo {
  id: number
  x: number
  y: number
  width: number
  height: number
}

export interface LayoutConfig {
  layout_name: string
  layout_path: string
  slots_count: number
  slots: SlotInfo[]
}

export interface PhotoInfo {
  name: string
  path: string
  url: string
}

export interface OperationResult {
  success: boolean
  path?: string
  error?: string
}

export interface ElectronAPI {
  // Event management
  initializeEvent: (eventId: string) => Promise<OperationResult>
  listEvents: () => Promise<string[]>
  getEventSettings: (eventId: string) => Promise<EventSettings | null>

  // Layout management
  saveLayout: (eventId: string, fileName: string, data: ArrayBuffer) => Promise<OperationResult>
  listLayouts: (eventId: string) => Promise<string[]>

  // File access
  getLocalUrl: (eventId: string, folder: string, fileName: string) => Promise<string>
  readFileBase64: (filePath: string) => Promise<string | null>

  // Slots configuration
  saveSlotsConfig: (eventId: string, config: LayoutConfig) => Promise<OperationResult>
  loadSlotsConfig: (eventId: string) => Promise<LayoutConfig | null>

  // Captured photos
  listCapturedPhotos: (eventId: string) => Promise<PhotoInfo[]>

  // File operations
  deleteFile: (eventId: string, folder: string, fileName: string) => Promise<OperationResult>

  // Backend control
  startBackend: (eventId?: string) => Promise<boolean>
  stopBackend: () => Promise<boolean>
  isBackendRunning: () => Promise<boolean>

  // Paths
  getDataRoot: () => Promise<string>

  // App control
  exitKiosk: () => Promise<boolean>
  quitApp: () => Promise<boolean>

  // Event listeners
  onBackendLog: (callback: (log: string) => void) => () => void
  onBackendError: (callback: (error: string) => void) => () => void
  onBackendClosed: (callback: (code: number | null) => void) => () => void
}

declare global {
  interface Window {
    electronAPI?: ElectronAPI
  }
}

// Helper to check if running in Electron
export function isElectron(): boolean {
  return typeof window !== 'undefined' && window.electronAPI !== undefined
}

// Helper to get Electron API with fallback
export function getElectronAPI(): ElectronAPI | null {
  return window.electronAPI || null
}
