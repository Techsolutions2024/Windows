import { useEffect, useRef, useState, useCallback } from 'react'
import { useAppStore } from '../store/useAppStore'

interface LiveViewDisplayProps {
  filter?: string
}

const CSS_FILTERS: Record<string, string> = {
  none: 'none',
  bw: 'grayscale(1)',
  sepia: 'sepia(0.8)',
  vintage: 'sepia(0.3) saturate(1.4) brightness(1.1)',
  cool: 'hue-rotate(10deg) saturate(1.2) brightness(1.05)',
  warm: 'sepia(0.2) saturate(1.3) brightness(1.05)',
}

// Worker message types
interface WorkerFrameMessage {
  type: 'frame'
  bitmap: ImageBitmap
  fps: number
}

interface WorkerStatusMessage {
  type: 'connected' | 'disconnected' | 'error' | 'status'
  reason?: string
  message?: string
  connected?: boolean
  fps?: number
}

type WorkerMessage = WorkerFrameMessage | WorkerStatusMessage

export default function LiveViewDisplay({ filter = 'none' }: LiveViewDisplayProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const workerRef = useRef<Worker | null>(null)
  const bitmapRef = useRef<ImageBitmap | null>(null)
  const rafRef = useRef<number>(0)
  const { setLiveViewActive } = useAppStore()
  const [isStreaming, setIsStreaming] = useState(false)
  const [showGrid, setShowGrid] = useState(false)
  const [fps, setFps] = useState(0)
  const [connectionStatus, setConnectionStatus] = useState<'connecting' | 'connected' | 'disconnected'>('connecting')

  // Render loop - runs at display refresh rate (60fps)
  const renderLoop = useCallback(() => {
    const canvas = canvasRef.current
    const bitmap = bitmapRef.current

    if (canvas && bitmap) {
      const ctx = canvas.getContext('2d', {
        alpha: false,
        desynchronized: true, // Reduce latency on supported browsers
      })

      if (ctx) {
        // Resize canvas to match bitmap on first frame or resolution change
        if (canvas.width !== bitmap.width || canvas.height !== bitmap.height) {
          canvas.width = bitmap.width
          canvas.height = bitmap.height
        }

        // Draw the frame
        ctx.drawImage(bitmap, 0, 0)
      }
    }

    rafRef.current = requestAnimationFrame(renderLoop)
  }, [])

  // Handle messages from worker
  const handleWorkerMessage = useCallback((event: MessageEvent<WorkerMessage>) => {
    const message = event.data

    switch (message.type) {
      case 'frame': {
        // Received decoded ImageBitmap from worker
        const { bitmap, fps: workerFps } = message as WorkerFrameMessage

        // Swap bitmap (release old one)
        const old = bitmapRef.current
        bitmapRef.current = bitmap
        if (old) old.close()

        // Update FPS if provided
        if (workerFps > 0) {
          setFps(workerFps)
        }

        setIsStreaming(true)
        break
      }

      case 'connected':
        setConnectionStatus('connected')
        setLiveViewActive(true)
        break

      case 'disconnected':
        setConnectionStatus('disconnected')
        setIsStreaming(false)
        setLiveViewActive(false)
        // Worker will auto-reconnect
        setTimeout(() => setConnectionStatus('connecting'), 100)
        break

      case 'error':
        console.error('[LiveView] Worker error:', (message as WorkerStatusMessage).message)
        break

      case 'status':
        const status = message as WorkerStatusMessage
        if (status.connected !== undefined) {
          setConnectionStatus(status.connected ? 'connected' : 'disconnected')
        }
        if (status.fps !== undefined) {
          setFps(status.fps)
        }
        break
    }
  }, [setLiveViewActive])

  // Initialize worker and start streaming
  useEffect(() => {
    // Create worker using Vite's worker syntax
    const worker = new Worker(
      new URL('../workers/liveview.worker.ts', import.meta.url),
      { type: 'module' }
    )

    workerRef.current = worker

    // Listen for messages from worker
    worker.onmessage = handleWorkerMessage

    worker.onerror = (error) => {
      console.error('[LiveView] Worker error:', error)
      setConnectionStatus('disconnected')
    }

    // Start connection
    worker.postMessage({ type: 'connect' })

    // Start render loop
    rafRef.current = requestAnimationFrame(renderLoop)

    // Cleanup
    return () => {
      // Stop render loop
      cancelAnimationFrame(rafRef.current)

      // Disconnect and terminate worker
      if (workerRef.current) {
        workerRef.current.postMessage({ type: 'disconnect' })
        // Give worker time to clean up
        setTimeout(() => {
          workerRef.current?.terminate()
          workerRef.current = null
        }, 100)
      }

      // Release bitmap
      if (bitmapRef.current) {
        bitmapRef.current.close()
        bitmapRef.current = null
      }

      setLiveViewActive(false)
    }
  }, [handleWorkerMessage, renderLoop, setLiveViewActive])

  const cssFilter = CSS_FILTERS[filter] || 'none'

  // Get status text and color
  const getStatusInfo = () => {
    switch (connectionStatus) {
      case 'connected':
        return { text: 'Live', color: 'bg-green-500' }
      case 'connecting':
        return { text: 'Connecting', color: 'bg-yellow-500' }
      case 'disconnected':
        return { text: 'Disconnected', color: 'bg-red-500' }
    }
  }

  const statusInfo = getStatusInfo()

  return (
    <div className="relative w-full h-full flex items-center justify-center bg-black overflow-hidden">
      {/* Hardware-accelerated Canvas with CSS filter */}
      <canvas
        ref={canvasRef}
        style={{
          filter: cssFilter,
          willChange: 'contents', // Hint for GPU compositing
        }}
        className="max-w-full max-h-full object-contain"
      />

      {/* Placeholder when not streaming */}
      {!isStreaming && (
        <div className="absolute inset-0 flex flex-col items-center justify-center bg-gradient-to-br from-gray-800 to-gray-900">
          <div className="text-gray-500 text-5xl mb-4">📷</div>
          <div className="text-gray-400 text-xl mb-2">Live View</div>
          <div className="text-gray-500 text-sm">
            {connectionStatus === 'connecting' ? 'Connecting to camera...' : 'Waiting for stream...'}
          </div>
          {connectionStatus === 'connecting' && (
            <div className="mt-4 w-8 h-8 border-2 border-blue-500 border-t-transparent rounded-full animate-spin" />
          )}
        </div>
      )}

      {/* Grid Overlay */}
      {showGrid && isStreaming && (
        <div className="absolute inset-0 pointer-events-none">
          <div className="w-full h-full grid grid-cols-3 grid-rows-3">
            {[...Array(9)].map((_, i) => (
              <div key={i} className="border border-white/20"></div>
            ))}
          </div>
          {/* Center crosshair */}
          <div className="absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2">
            <div className="w-8 h-0.5 bg-white/40" />
            <div className="w-0.5 h-8 bg-white/40 absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2" />
          </div>
        </div>
      )}

      {/* Status Bar */}
      <div className="absolute top-4 left-4 flex items-center gap-3">
        <div className="flex items-center gap-2 bg-black/50 px-3 py-1.5 rounded-full backdrop-blur-sm">
          <div className={`w-2.5 h-2.5 rounded-full ${statusInfo.color} ${connectionStatus === 'connected' ? 'animate-pulse' : ''}`} />
          <span className="text-white/90 text-sm font-medium">
            {statusInfo.text}
          </span>
          {isStreaming && fps > 0 && (
            <>
              <span className="text-white/30">|</span>
              <span className="text-white/70 text-xs font-mono">{fps} FPS</span>
            </>
          )}
        </div>
      </div>

      {/* Controls */}
      <div className="absolute top-4 right-4 flex items-center gap-2">
        {/* Grid Toggle */}
        <button
          onClick={() => setShowGrid(!showGrid)}
          className={`px-3 py-1.5 rounded-lg text-sm transition-all ${
            showGrid
              ? 'bg-blue-500/80 text-white'
              : 'bg-white/10 hover:bg-white/20 text-white/70'
          }`}
        >
          Grid
        </button>
      </div>

      {/* Performance indicator (debug) */}
      {process.env.NODE_ENV === 'development' && isStreaming && (
        <div className="absolute bottom-4 left-4 text-xs text-white/50 font-mono bg-black/50 px-2 py-1 rounded">
          Worker: Active | Canvas: {canvasRef.current?.width}x{canvasRef.current?.height}
        </div>
      )}
    </div>
  )
}
