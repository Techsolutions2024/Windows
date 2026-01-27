import { useState, useEffect, useCallback } from 'react'
import type {
  ElectronAPI,
  EventSettings,
  LayoutConfig,
  PhotoInfo,
  OperationResult
} from '../types/electron.d'

// Check if running in Electron
export function isElectron(): boolean {
  return typeof window !== 'undefined' && window.electronAPI !== undefined
}

// Get Electron API
function getAPI(): ElectronAPI | null {
  return window.electronAPI || null
}

// Hook for Electron environment detection
export function useIsElectron(): boolean {
  const [isElectronEnv, setIsElectronEnv] = useState(false)

  useEffect(() => {
    setIsElectronEnv(isElectron())
  }, [])

  return isElectronEnv
}

// Hook for event management
export function useEventManager() {
  const api = getAPI()

  const initializeEvent = useCallback(async (eventId: string): Promise<OperationResult> => {
    if (!api) {
      return { success: false, error: 'Not running in Electron' }
    }
    return api.initializeEvent(eventId)
  }, [api])

  const listEvents = useCallback(async (): Promise<string[]> => {
    if (!api) return []
    return api.listEvents()
  }, [api])

  const getEventSettings = useCallback(async (eventId: string): Promise<EventSettings | null> => {
    if (!api) return null
    return api.getEventSettings(eventId)
  }, [api])

  return {
    initializeEvent,
    listEvents,
    getEventSettings,
    isAvailable: !!api
  }
}

// Hook for layout management
export function useLayoutManager(eventId: string | null) {
  const api = getAPI()

  const saveLayout = useCallback(async (
    fileName: string,
    file: File
  ): Promise<OperationResult> => {
    if (!api || !eventId) {
      return { success: false, error: 'Not available' }
    }

    const buffer = await file.arrayBuffer()
    return api.saveLayout(eventId, fileName, buffer)
  }, [api, eventId])

  const listLayouts = useCallback(async (): Promise<string[]> => {
    if (!api || !eventId) return []
    return api.listLayouts(eventId)
  }, [api, eventId])

  const getLayoutUrl = useCallback(async (layoutName: string): Promise<string> => {
    if (!api || !eventId) return ''
    return api.getLocalUrl(eventId, 'layouts', layoutName)
  }, [api, eventId])

  const deleteLayout = useCallback(async (layoutName: string): Promise<OperationResult> => {
    if (!api || !eventId) {
      return { success: false, error: 'Not available' }
    }
    return api.deleteFile(eventId, 'layouts', layoutName)
  }, [api, eventId])

  return {
    saveLayout,
    listLayouts,
    getLayoutUrl,
    deleteLayout,
    isAvailable: !!api && !!eventId
  }
}

// Hook for slots configuration
export function useSlotsConfig(eventId: string | null) {
  const api = getAPI()
  const [config, setConfig] = useState<LayoutConfig | null>(null)
  const [loading, setLoading] = useState(false)

  const loadConfig = useCallback(async () => {
    if (!api || !eventId) return

    setLoading(true)
    try {
      const loaded = await api.loadSlotsConfig(eventId)
      setConfig(loaded)
    } finally {
      setLoading(false)
    }
  }, [api, eventId])

  const saveConfig = useCallback(async (newConfig: LayoutConfig): Promise<boolean> => {
    if (!api || !eventId) return false

    const result = await api.saveSlotsConfig(eventId, newConfig)
    if (result.success) {
      setConfig(newConfig)
    }
    return result.success
  }, [api, eventId])

  useEffect(() => {
    loadConfig()
  }, [loadConfig])

  return {
    config,
    loading,
    saveConfig,
    reloadConfig: loadConfig,
    isAvailable: !!api && !!eventId
  }
}

// Hook for captured photos
export function useCapturedPhotos(eventId: string | null) {
  const api = getAPI()
  const [photos, setPhotos] = useState<PhotoInfo[]>([])
  const [loading, setLoading] = useState(false)

  const loadPhotos = useCallback(async () => {
    if (!api || !eventId) return

    setLoading(true)
    try {
      const loaded = await api.listCapturedPhotos(eventId)
      setPhotos(loaded)
    } finally {
      setLoading(false)
    }
  }, [api, eventId])

  const deletePhoto = useCallback(async (fileName: string): Promise<boolean> => {
    if (!api || !eventId) return false

    const result = await api.deleteFile(eventId, 'captured_photos', fileName)
    if (result.success) {
      setPhotos(prev => prev.filter(p => p.name !== fileName))
    }
    return result.success
  }, [api, eventId])

  useEffect(() => {
    loadPhotos()
  }, [loadPhotos])

  return {
    photos,
    loading,
    reloadPhotos: loadPhotos,
    deletePhoto,
    isAvailable: !!api && !!eventId
  }
}

// Hook for backend control
export function useBackendControl() {
  const api = getAPI()
  const [isRunning, setIsRunning] = useState(false)
  const [logs, setLogs] = useState<string[]>([])
  const [errors, setErrors] = useState<string[]>([])

  useEffect(() => {
    if (!api) return

    // Check initial status
    api.isBackendRunning().then(setIsRunning)

    // Subscribe to logs
    const unsubLog = api.onBackendLog((log) => {
      setLogs(prev => [...prev.slice(-99), log])
    })

    const unsubError = api.onBackendError((error) => {
      setErrors(prev => [...prev.slice(-49), error])
    })

    const unsubClosed = api.onBackendClosed((code) => {
      setIsRunning(false)
      setLogs(prev => [...prev, `Backend exited with code: ${code}`])
    })

    return () => {
      unsubLog()
      unsubError()
      unsubClosed()
    }
  }, [api])

  const startBackend = useCallback(async (eventId?: string): Promise<boolean> => {
    if (!api) return false
    const result = await api.startBackend(eventId)
    if (result) {
      setIsRunning(true)
      setLogs([])
      setErrors([])
    }
    return result
  }, [api])

  const stopBackend = useCallback(async (): Promise<boolean> => {
    if (!api) return false
    const result = await api.stopBackend()
    if (result) {
      setIsRunning(false)
    }
    return result
  }, [api])

  const clearLogs = useCallback(() => {
    setLogs([])
    setErrors([])
  }, [])

  return {
    isRunning,
    logs,
    errors,
    startBackend,
    stopBackend,
    clearLogs,
    isAvailable: !!api
  }
}

// Hook for app control
export function useAppControl() {
  const api = getAPI()

  const exitKiosk = useCallback(async (): Promise<boolean> => {
    if (!api) return false
    return api.exitKiosk()
  }, [api])

  const quitApp = useCallback(async (): Promise<boolean> => {
    if (!api) return false
    return api.quitApp()
  }, [api])

  const getDataRoot = useCallback(async (): Promise<string> => {
    if (!api) return ''
    return api.getDataRoot()
  }, [api])

  return {
    exitKiosk,
    quitApp,
    getDataRoot,
    isAvailable: !!api
  }
}

// Combined hook for common operations
export function useElectron() {
  const isElectronEnv = useIsElectron()
  const eventManager = useEventManager()
  const backendControl = useBackendControl()
  const appControl = useAppControl()

  return {
    isElectron: isElectronEnv,
    ...eventManager,
    ...backendControl,
    ...appControl
  }
}

export default useElectron
