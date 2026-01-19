export interface Event {
  id: number
  name: string
  createdAt: string
  photoCount: number
  thumbnailPath?: string
}

export interface Photo {
  id: number
  eventId: number
  filePath: string
  captureMode: CaptureMode
  timestamp: string
  width: number
  height: number
  printed: boolean
  shared: boolean
}

export type CaptureMode = 'photo' | 'gif' | 'boomerang' | 'video'

export interface CameraSettings {
  iso: number
  aperture: string
  shutterSpeed: string
  whiteBalance: string
  mirror: boolean
  rotation: 0 | 90 | 180 | 270
}

export interface Camera {
  name: string
  type: 'canon' | 'webcam'
  connected: boolean
}

export interface AppSettings {
  selectedCamera: string | null
  cameraSettings: CameraSettings
  countdownDuration: number
  captureMode: CaptureMode
  autoPreview: boolean
  autoPrint: boolean
  printCopies: number
  screenOrientation: 'landscape' | 'portrait'
}

export interface FilterType {
  id: string
  name: string
  preview?: string
}

export interface ShareOption {
  type: 'email' | 'sms' | 'qr' | 'print'
  enabled: boolean
  icon: string
  color: string
}
