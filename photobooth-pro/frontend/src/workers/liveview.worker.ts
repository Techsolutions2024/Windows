/**
 * LiveView Web Worker
 * Handles WebSocket connection and JPEG decoding off the main thread
 * Communicates with main thread via postMessage using Transferable objects
 */

// Worker state
let ws: WebSocket | null = null
let isConnected = false
let reconnectTimer: ReturnType<typeof setTimeout> | null = null
let heartbeatTimer: ReturnType<typeof setInterval> | null = null
let lastPongTime = 0
let frameCount = 0
let lastFpsTime = performance.now()

// Configuration
const CONFIG = {
  wsUrl: 'ws://localhost:8081',
  reconnectDelay: 1000,
  heartbeatInterval: 3000,
  heartbeatTimeout: 5000,
}

// Message types from main thread
interface MainThreadMessage {
  type: 'connect' | 'disconnect' | 'ready' | 'config'
  config?: {
    wsUrl?: string
    heartbeatInterval?: number
  }
}

// Message types to main thread
type WorkerMessage =
  | { type: 'connected' }
  | { type: 'disconnected'; reason: string }
  | { type: 'frame'; bitmap: ImageBitmap; fps: number }
  | { type: 'error'; message: string }
  | { type: 'status'; connected: boolean; fps: number }

function postToMain(message: WorkerMessage, transfer?: Transferable[]) {
  if (transfer) {
    // @ts-ignore - TypeScript definition might be mixing Window.postMessage with Worker.postMessage
    self.postMessage(message, transfer)
  } else {
    self.postMessage(message)
  }
}

function startHeartbeat() {
  stopHeartbeat()
  lastPongTime = performance.now()

  heartbeatTimer = setInterval(() => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      // Check if we missed heartbeat response
      const timeSinceLastPong = performance.now() - lastPongTime
      if (timeSinceLastPong > CONFIG.heartbeatTimeout) {
        console.warn('[Worker] Heartbeat timeout, reconnecting...')
        ws.close()
        return
      }

      // Send heartbeat
      ws.send(JSON.stringify({ type: 'ping', timestamp: Date.now() }))
    }
  }, CONFIG.heartbeatInterval)
}

function stopHeartbeat() {
  if (heartbeatTimer) {
    clearInterval(heartbeatTimer)
    heartbeatTimer = null
  }
}

function connect() {
  if (ws && (ws.readyState === WebSocket.CONNECTING || ws.readyState === WebSocket.OPEN)) {
    return
  }

  try {
    ws = new WebSocket(CONFIG.wsUrl)
    ws.binaryType = 'arraybuffer'

    ws.onopen = () => {
      isConnected = true
      console.log('[Worker] WebSocket connected')

      // Request live view stream
      ws!.send(JSON.stringify({ type: 'liveview:start' }))

      // Start heartbeat
      startHeartbeat()

      postToMain({ type: 'connected' })
    }

    ws.onmessage = async (event) => {
      if (event.data instanceof ArrayBuffer) {
        // Binary = JPEG frame from camera
        await decodeAndSendFrame(event.data)

        // Send READY signal for flow control
        if (ws && ws.readyState === WebSocket.OPEN) {
          // Send ready signal to request the next frame
          ws.send(JSON.stringify({ type: 'liveview:ready' }))
        }
      } else {
        // Text = JSON control message
        try {
          const msg = JSON.parse(event.data)
          if (msg.type === 'pong') {
            // Response to our ping
            lastPongTime = performance.now()
          } else if (msg.type === 'server:ping') {
            // Server-initiated ping, respond with pong
            if (ws && ws.readyState === WebSocket.OPEN) {
              ws.send(JSON.stringify({ type: 'ping', timestamp: Date.now() }))
            }
            lastPongTime = performance.now() // Also update our heartbeat tracker
          } else if (msg.type === 'liveview:started') {
            console.log('[Worker] Live view stream started')
          } else if (msg.type === 'liveview:stopped') {
            console.log('[Worker] Live view stream stopped')
          }
        } catch {
          // Ignore parse errors
        }
      }
    }

    ws.onclose = (event) => {
      isConnected = false
      stopHeartbeat()
      console.log('[Worker] WebSocket disconnected:', event.reason || 'Unknown reason')

      postToMain({ type: 'disconnected', reason: event.reason || 'Connection closed' })

      // Schedule reconnect
      if (!reconnectTimer) {
        reconnectTimer = setTimeout(() => {
          reconnectTimer = null
          connect()
        }, CONFIG.reconnectDelay)
      }
    }

    ws.onerror = (error) => {
      console.error('[Worker] WebSocket error:', error)
      postToMain({ type: 'error', message: 'WebSocket connection error' })
    }
  } catch (error) {
    console.error('[Worker] Failed to create WebSocket:', error)
    postToMain({ type: 'error', message: 'Failed to create WebSocket connection' })
  }
}

function disconnect() {
  // Clear reconnect timer
  if (reconnectTimer) {
    clearTimeout(reconnectTimer)
    reconnectTimer = null
  }

  stopHeartbeat()

  if (ws) {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ type: 'liveview:stop' }))
    }
    ws.close()
    ws = null
  }

  isConnected = false
}

async function decodeAndSendFrame(data: ArrayBuffer) {
  try {
    // Create blob from JPEG data
    const blob = new Blob([data], { type: 'image/jpeg' })

    // Decode JPEG to ImageBitmap (hardware accelerated)
    const bitmap = await createImageBitmap(blob, {
      premultiplyAlpha: 'none',
      colorSpaceConversion: 'none',
    })

    // Calculate FPS
    frameCount++
    const now = performance.now()
    let fps = 0
    if (now - lastFpsTime >= 1000) {
      fps = frameCount
      frameCount = 0
      lastFpsTime = now
    }

    // Send bitmap to main thread (Transferable - zero-copy)
    postToMain({ type: 'frame', bitmap, fps }, [bitmap])
  } catch (error) {
    // Invalid JPEG data, skip frame
    console.warn('[Worker] Failed to decode frame:', error)
  }
}

// Handle messages from main thread
self.onmessage = (event: MessageEvent<MainThreadMessage>) => {
  const { type, config } = event.data

  switch (type) {
    case 'connect':
      connect()
      break

    case 'disconnect':
      disconnect()
      break

    case 'ready':
      // Main thread signals it's ready for next frame
      // This is optional additional flow control
      break

    case 'config':
      if (config) {
        if (config.wsUrl) CONFIG.wsUrl = config.wsUrl
        if (config.heartbeatInterval) CONFIG.heartbeatInterval = config.heartbeatInterval
      }
      break
  }
}

// Auto-connect when worker starts
console.log('[Worker] LiveView worker initialized')
