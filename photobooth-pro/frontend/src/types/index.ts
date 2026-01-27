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
  webcamIndex?: number
}

export interface EventConfig {
  eventId: number
  captureMode: CaptureMode
  photoEnabled: boolean
  gifEnabled: boolean
  boomerangEnabled: boolean
  videoEnabled: boolean
  countdownSeconds: number
  photoCount: number
  layoutTemplate: string
  cameraSource: 'canon' | 'webcam'
  webcamIndex: number
  cameraSettings?: string // JSON string of camera settings
  effectsConfig?: string
  propsConfig?: string
  beautyFilterConfig?: string
  watermarkConfig?: string
  postProcessConfig?: string
  printSettings?: string
  displaySettings?: string
  layoutSettings?: string
  captureSettings?: string
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
  photoInterval: number
  layout: {
    selectedLayoutId: string | null
    layouts: Layout[]
  }
}

export interface Layout {
  id: string
  name: string
  path: string
  photoCount: number
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
