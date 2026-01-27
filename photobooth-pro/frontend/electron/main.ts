import { app, BrowserWindow, ipcMain, protocol, net } from 'electron'
import { spawn, ChildProcess } from 'child_process'
import * as path from 'path'
import * as fs from 'fs'
import * as url from 'url'

const __dirname = path.dirname(url.fileURLToPath(import.meta.url))

// Backend server process
let backendProcess: ChildProcess | null = null
let mainWindow: BrowserWindow | null = null

// Configuration
const isDev = process.env.NODE_ENV === 'development'
const DATA_ROOT = path.join(app.getPath('userData'), 'Photobooth_Data')

// Get backend executable path
function getBackendPath(): string {
  if (isDev) {
    // Development: use local build
    return path.join(__dirname, '../../backend/build/Release/photobooth-server.exe')
  } else {
    // Production: use bundled executable
    return path.join(process.resourcesPath, 'backend/photobooth-server.exe')
  }
}

// Spawn backend server
function startBackendServer(eventId?: string): boolean {
  const backendPath = getBackendPath()

  if (!fs.existsSync(backendPath)) {
    console.error('Backend executable not found:', backendPath)
    return false
  }

  // Kill existing process if running
  stopBackendServer()

  const args: string[] = []
  if (eventId) {
    args.push('--event', eventId)
  }
  args.push('--data-dir', DATA_ROOT)

  console.log('Starting backend server:', backendPath, args.join(' '))

  backendProcess = spawn(backendPath, args, {
    cwd: path.dirname(backendPath),
    stdio: ['pipe', 'pipe', 'pipe'],
    windowsHide: true
  })

  backendProcess.stdout?.on('data', (data: Buffer) => {
    console.log('[Backend]', data.toString().trim())
    // Forward to renderer
    mainWindow?.webContents.send('backend-log', data.toString())
  })

  backendProcess.stderr?.on('data', (data: Buffer) => {
    console.error('[Backend Error]', data.toString().trim())
    mainWindow?.webContents.send('backend-error', data.toString())
  })

  backendProcess.on('close', (code: number | null) => {
    console.log('Backend process exited with code:', code)
    backendProcess = null
    mainWindow?.webContents.send('backend-closed', code)
  })

  backendProcess.on('error', (err: Error) => {
    console.error('Failed to start backend:', err)
    backendProcess = null
  })

  return true
}

// Stop backend server
function stopBackendServer(): void {
  if (backendProcess) {
    console.log('Stopping backend server...')

    // Send graceful shutdown signal
    if (process.platform === 'win32') {
      // Windows: send CTRL+C equivalent
      backendProcess.kill('SIGINT')
    } else {
      backendProcess.kill('SIGTERM')
    }

    // Force kill after timeout
    setTimeout(() => {
      if (backendProcess && !backendProcess.killed) {
        console.log('Force killing backend...')
        backendProcess.kill('SIGKILL')
      }
    }, 3000)

    backendProcess = null
  }
}

// Register custom protocol for local file access
function registerFileProtocol(): void {
  protocol.handle('photo-local', async (request) => {
    // URL format: photo-local://eventId/folder/filename
    // Maps to: DATA_ROOT/events/eventId/folder/filename
    const urlPath = request.url.replace('photo-local://', '')
    const filePath = path.join(DATA_ROOT, 'events', urlPath)

    console.log('[Protocol] Requesting:', filePath)

    try {
      // Security: ensure path is within DATA_ROOT
      const resolvedPath = path.resolve(filePath)
      if (!resolvedPath.startsWith(DATA_ROOT)) {
        console.error('[Protocol] Access denied - path outside data root')
        return new Response('Forbidden', { status: 403 })
      }

      if (!fs.existsSync(resolvedPath)) {
        return new Response('Not Found', { status: 404 })
      }

      return net.fetch(url.pathToFileURL(resolvedPath).toString())
    } catch (error) {
      console.error('[Protocol] Error:', error)
      return new Response('Internal Error', { status: 500 })
    }
  })
}

// Create main window
function createWindow(): void {
  mainWindow = new BrowserWindow({
    width: 1920,
    height: 1080,
    fullscreen: !isDev,
    kiosk: !isDev,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: false
    },
    frame: isDev,
    autoHideMenuBar: true
  })

  // Load app
  if (isDev) {
    mainWindow.loadURL('http://localhost:5173')
    mainWindow.webContents.openDevTools()
  } else {
    mainWindow.loadFile(path.join(__dirname, '../dist/index.html'))
  }

  mainWindow.on('closed', () => {
    mainWindow = null
  })
}

// IPC Handlers
function setupIpcHandlers(): void {
  // Event management
  ipcMain.handle('initialize-event', async (_event, eventId: string) => {
    const eventPath = path.join(DATA_ROOT, 'events', eventId)

    try {
      // Create directory structure
      const dirs = [
        eventPath,
        path.join(eventPath, 'layouts'),
        path.join(eventPath, 'captured_photos'),
        path.join(eventPath, 'config')
      ]

      for (const dir of dirs) {
        if (!fs.existsSync(dir)) {
          fs.mkdirSync(dir, { recursive: true })
        }
      }

      // Create default settings
      const settingsPath = path.join(eventPath, 'event_settings.json')
      if (!fs.existsSync(settingsPath)) {
        const settings = {
          eventId,
          eventName: `Event ${eventId}`,
          createdAt: new Date().toISOString(),
          currentLayout: '',
          photoCount: 0
        }
        fs.writeFileSync(settingsPath, JSON.stringify(settings, null, 2))
      }

      return { success: true, path: eventPath }
    } catch (error) {
      console.error('Failed to initialize event:', error)
      return { success: false, error: String(error) }
    }
  })

  // List events
  ipcMain.handle('list-events', async () => {
    const eventsDir = path.join(DATA_ROOT, 'events')
    if (!fs.existsSync(eventsDir)) {
      return []
    }

    const entries = fs.readdirSync(eventsDir, { withFileTypes: true })
    return entries
      .filter(entry => entry.isDirectory())
      .map(entry => entry.name)
  })

  // Get event settings
  ipcMain.handle('get-event-settings', async (_event, eventId: string) => {
    const settingsPath = path.join(DATA_ROOT, 'events', eventId, 'event_settings.json')
    if (!fs.existsSync(settingsPath)) {
      return null
    }
    return JSON.parse(fs.readFileSync(settingsPath, 'utf-8'))
  })

  // Save layout PNG
  ipcMain.handle('save-layout', async (_event, eventId: string, fileName: string, data: ArrayBuffer) => {
    const layoutPath = path.join(DATA_ROOT, 'events', eventId, 'layouts', fileName)

    try {
      // Ensure directory exists
      const dir = path.dirname(layoutPath)
      if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true })
      }

      fs.writeFileSync(layoutPath, Buffer.from(data))
      return { success: true, path: layoutPath }
    } catch (error) {
      return { success: false, error: String(error) }
    }
  })

  // List layouts
  ipcMain.handle('list-layouts', async (_event, eventId: string) => {
    const layoutsDir = path.join(DATA_ROOT, 'events', eventId, 'layouts')
    if (!fs.existsSync(layoutsDir)) {
      return []
    }

    const files = fs.readdirSync(layoutsDir)
    return files.filter(f => f.toLowerCase().endsWith('.png'))
  })

  // Get file path for local protocol
  ipcMain.handle('get-local-url', async (_event, eventId: string, folder: string, fileName: string) => {
    return `photo-local://${eventId}/${folder}/${fileName}`
  })

  // Read file as base64 (for preview)
  ipcMain.handle('read-file-base64', async (_event, filePath: string) => {
    const fullPath = path.join(DATA_ROOT, 'events', filePath)
    if (!fs.existsSync(fullPath)) {
      return null
    }
    return fs.readFileSync(fullPath).toString('base64')
  })

  // Save slots config
  ipcMain.handle('save-slots-config', async (_event, eventId: string, config: object) => {
    const configPath = path.join(DATA_ROOT, 'events', eventId, 'config', 'slots_config.json')

    try {
      const dir = path.dirname(configPath)
      if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true })
      }

      fs.writeFileSync(configPath, JSON.stringify(config, null, 2))
      return { success: true }
    } catch (error) {
      return { success: false, error: String(error) }
    }
  })

  // Load slots config
  ipcMain.handle('load-slots-config', async (_event, eventId: string) => {
    const configPath = path.join(DATA_ROOT, 'events', eventId, 'config', 'slots_config.json')
    if (!fs.existsSync(configPath)) {
      return null
    }
    return JSON.parse(fs.readFileSync(configPath, 'utf-8'))
  })

  // Backend control
  ipcMain.handle('start-backend', async (_event, eventId?: string) => {
    return startBackendServer(eventId)
  })

  ipcMain.handle('stop-backend', async () => {
    stopBackendServer()
    return true
  })

  ipcMain.handle('is-backend-running', async () => {
    return backendProcess !== null && !backendProcess.killed
  })

  // Get data root path
  ipcMain.handle('get-data-root', async () => {
    return DATA_ROOT
  })

  // List captured photos
  ipcMain.handle('list-captured-photos', async (_event, eventId: string) => {
    const photosDir = path.join(DATA_ROOT, 'events', eventId, 'captured_photos')
    if (!fs.existsSync(photosDir)) {
      return []
    }

    const files = fs.readdirSync(photosDir)
    return files
      .filter(f => /\.(jpg|jpeg|png)$/i.test(f))
      .map(f => ({
        name: f,
        path: path.join(photosDir, f),
        url: `photo-local://${eventId}/captured_photos/${f}`
      }))
  })

  // Delete file
  ipcMain.handle('delete-file', async (_event, eventId: string, folder: string, fileName: string) => {
    const filePath = path.join(DATA_ROOT, 'events', eventId, folder, fileName)
    try {
      if (fs.existsSync(filePath)) {
        fs.unlinkSync(filePath)
      }
      return { success: true }
    } catch (error) {
      return { success: false, error: String(error) }
    }
  })

  // Exit kiosk mode (dev only)
  ipcMain.handle('exit-kiosk', async () => {
    if (isDev && mainWindow) {
      mainWindow.setKiosk(false)
      mainWindow.setFullScreen(false)
    }
    return true
  })

  // Quit application
  ipcMain.handle('quit-app', async () => {
    app.quit()
    return true
  })
}

// App lifecycle
app.whenReady().then(() => {
  // Register protocol before creating window
  registerFileProtocol()

  // Setup IPC
  setupIpcHandlers()

  // Create window
  createWindow()

  // Start backend server
  const savedEventId = process.env.EVENT_ID || undefined
  startBackendServer(savedEventId)

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow()
    }
  })
})

app.on('window-all-closed', () => {
  stopBackendServer()
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('before-quit', () => {
  stopBackendServer()
})

// Handle uncaught exceptions
process.on('uncaughtException', (error) => {
  console.error('Uncaught exception:', error)
  stopBackendServer()
})
