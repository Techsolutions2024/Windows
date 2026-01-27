import { contextBridge, ipcRenderer, IpcRendererEvent } from 'electron'

// Type definitions for exposed API
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

// Expose protected methods to renderer
const electronAPI = {
  // Event management
  initializeEvent: (eventId: string): Promise<OperationResult> =>
    ipcRenderer.invoke('initialize-event', eventId),

  listEvents: (): Promise<string[]> =>
    ipcRenderer.invoke('list-events'),

  getEventSettings: (eventId: string): Promise<EventSettings | null> =>
    ipcRenderer.invoke('get-event-settings', eventId),

  // Layout management
  saveLayout: (eventId: string, fileName: string, data: ArrayBuffer): Promise<OperationResult> =>
    ipcRenderer.invoke('save-layout', eventId, fileName, data),

  listLayouts: (eventId: string): Promise<string[]> =>
    ipcRenderer.invoke('list-layouts', eventId),

  // File access
  getLocalUrl: (eventId: string, folder: string, fileName: string): Promise<string> =>
    ipcRenderer.invoke('get-local-url', eventId, folder, fileName),

  readFileBase64: (filePath: string): Promise<string | null> =>
    ipcRenderer.invoke('read-file-base64', filePath),

  // Slots configuration
  saveSlotsConfig: (eventId: string, config: LayoutConfig): Promise<OperationResult> =>
    ipcRenderer.invoke('save-slots-config', eventId, config),

  loadSlotsConfig: (eventId: string): Promise<LayoutConfig | null> =>
    ipcRenderer.invoke('load-slots-config', eventId),

  // Captured photos
  listCapturedPhotos: (eventId: string): Promise<PhotoInfo[]> =>
    ipcRenderer.invoke('list-captured-photos', eventId),

  // File operations
  deleteFile: (eventId: string, folder: string, fileName: string): Promise<OperationResult> =>
    ipcRenderer.invoke('delete-file', eventId, folder, fileName),

  // Backend control
  startBackend: (eventId?: string): Promise<boolean> =>
    ipcRenderer.invoke('start-backend', eventId),

  stopBackend: (): Promise<boolean> =>
    ipcRenderer.invoke('stop-backend'),

  isBackendRunning: (): Promise<boolean> =>
    ipcRenderer.invoke('is-backend-running'),

  // Paths
  getDataRoot: (): Promise<string> =>
    ipcRenderer.invoke('get-data-root'),

  // App control
  exitKiosk: (): Promise<boolean> =>
    ipcRenderer.invoke('exit-kiosk'),

  quitApp: (): Promise<boolean> =>
    ipcRenderer.invoke('quit-app'),

  // Event listeners for backend output
  onBackendLog: (callback: (log: string) => void) => {
    const handler = (_event: IpcRendererEvent, log: string) => callback(log)
    ipcRenderer.on('backend-log', handler)
    return () => ipcRenderer.removeListener('backend-log', handler)
  },

  onBackendError: (callback: (error: string) => void) => {
    const handler = (_event: IpcRendererEvent, error: string) => callback(error)
    ipcRenderer.on('backend-error', handler)
    return () => ipcRenderer.removeListener('backend-error', handler)
  },

  onBackendClosed: (callback: (code: number | null) => void) => {
    const handler = (_event: IpcRendererEvent, code: number | null) => callback(code)
    ipcRenderer.on('backend-closed', handler)
    return () => ipcRenderer.removeListener('backend-closed', handler)
  }
}

// Expose to renderer
contextBridge.exposeInMainWorld('electronAPI', electronAPI)

// Type declaration for renderer
declare global {
  interface Window {
    electronAPI: typeof electronAPI
  }
}
