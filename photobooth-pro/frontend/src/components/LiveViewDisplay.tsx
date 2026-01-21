import { useEffect, useRef, useState } from 'react'
import { useAppStore } from '../store/useAppStore'
import { useLiveView } from '../services/websocket'

interface LiveViewDisplayProps {
  filter?: string
}

export default function LiveViewDisplay({ filter = 'none' }: LiveViewDisplayProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const { setLiveViewActive, setLiveViewFrame } = useAppStore()
  const { frame, isActive, isConnected } = useLiveView()
  const [dimensions, setDimensions] = useState({ width: 1280, height: 720 })
  const [showGrid, setShowGrid] = useState(false)

  // Update store when WebSocket state changes
  useEffect(() => {
    setLiveViewActive(isActive)
    if (frame) {
      setLiveViewFrame(frame)
    }
  }, [isActive, frame, setLiveViewActive, setLiveViewFrame])

  // Render frame from WebSocket
  useEffect(() => {
    if (frame && canvasRef.current) {
      const canvas = canvasRef.current
      const ctx = canvas.getContext('2d')
      if (ctx) {
        const img = new Image()
        img.onload = () => {
          // Clear canvas
          ctx.clearRect(0, 0, canvas.width, canvas.height)

          // Draw image maintaining aspect ratio
          const imgAspect = img.width / img.height
          const canvasAspect = canvas.width / canvas.height

          let drawWidth, drawHeight, drawX, drawY

          if (imgAspect > canvasAspect) {
            drawWidth = canvas.width
            drawHeight = canvas.width / imgAspect
            drawX = 0
            drawY = (canvas.height - drawHeight) / 2
          } else {
            drawHeight = canvas.height
            drawWidth = canvas.height * imgAspect
            drawX = (canvas.width - drawWidth) / 2
            drawY = 0
          }

          ctx.drawImage(img, drawX, drawY, drawWidth, drawHeight)
          applyFilter(ctx, filter, drawX, drawY, drawWidth, drawHeight)
        }
        // Frame is base64 encoded JPEG from WebSocket
        img.src = `data:image/jpeg;base64,${frame}`
      }
    } else if (!frame && canvasRef.current) {
      drawPlaceholder()
    }
  }, [frame, filter])

  // Initial placeholder
  useEffect(() => {
    if (!frame) {
      drawPlaceholder()
    }
  }, [])

  const drawPlaceholder = () => {
    const canvas = canvasRef.current
    if (!canvas) return

    const ctx = canvas.getContext('2d')
    if (!ctx) return

    // Draw gradient background
    const gradient = ctx.createLinearGradient(0, 0, canvas.width, canvas.height)
    gradient.addColorStop(0, '#1F2937')
    gradient.addColorStop(1, '#111827')
    ctx.fillStyle = gradient
    ctx.fillRect(0, 0, canvas.width, canvas.height)

    // Draw camera icon placeholder
    ctx.fillStyle = '#374151'
    ctx.font = '48px Arial'
    ctx.textAlign = 'center'
    ctx.textBaseline = 'middle'
    ctx.fillText('📷', canvas.width / 2, canvas.height / 2 - 40)

    ctx.fillStyle = '#9CA3AF'
    ctx.font = '24px Arial'
    ctx.fillText('Live View', canvas.width / 2, canvas.height / 2 + 20)
    ctx.font = '16px Arial'

    if (!isConnected) {
      ctx.fillStyle = '#EF4444'
      ctx.fillText('WebSocket disconnected', canvas.width / 2, canvas.height / 2 + 50)
    } else {
      ctx.fillText('Waiting for camera stream...', canvas.width / 2, canvas.height / 2 + 50)
    }
  }

  const applyFilter = (
    ctx: CanvasRenderingContext2D,
    filterType: string,
    x: number,
    y: number,
    width: number,
    height: number
  ) => {
    if (filterType === 'none') return

    const imageData = ctx.getImageData(x, y, width, height)
    const data = imageData.data

    switch (filterType) {
      case 'bw':
        for (let i = 0; i < data.length; i += 4) {
          const avg = (data[i] + data[i + 1] + data[i + 2]) / 3
          data[i] = data[i + 1] = data[i + 2] = avg
        }
        break
      case 'sepia':
        for (let i = 0; i < data.length; i += 4) {
          const r = data[i], g = data[i + 1], b = data[i + 2]
          data[i] = Math.min(255, r * 0.393 + g * 0.769 + b * 0.189)
          data[i + 1] = Math.min(255, r * 0.349 + g * 0.686 + b * 0.168)
          data[i + 2] = Math.min(255, r * 0.272 + g * 0.534 + b * 0.131)
        }
        break
      case 'vintage':
        for (let i = 0; i < data.length; i += 4) {
          data[i] = Math.min(255, data[i] * 1.2)
          data[i + 1] = Math.min(255, data[i + 1] * 1.1)
          data[i + 2] = Math.min(255, data[i + 2] * 0.9)
        }
        break
      case 'cool':
        for (let i = 0; i < data.length; i += 4) {
          data[i] = Math.min(255, data[i] * 0.9)
          data[i + 2] = Math.min(255, data[i + 2] * 1.2)
        }
        break
      case 'warm':
        for (let i = 0; i < data.length; i += 4) {
          data[i] = Math.min(255, data[i] * 1.2)
          data[i + 2] = Math.min(255, data[i + 2] * 0.9)
        }
        break
    }

    ctx.putImageData(imageData, x, y)
  }

  return (
    <div className="relative w-full h-full flex items-center justify-center bg-black">
      <canvas
        ref={canvasRef}
        width={dimensions.width}
        height={dimensions.height}
        className="max-w-full max-h-full object-contain"
      />

      {/* Grid Overlay */}
      {showGrid && (
        <div className="absolute inset-0 pointer-events-none">
          <div className="w-full h-full grid grid-cols-3 grid-rows-3 opacity-20">
            {[...Array(9)].map((_, i) => (
              <div key={i} className="border border-white/20"></div>
            ))}
          </div>
        </div>
      )}

      {/* Connection Status */}
      <div className="absolute top-4 left-4 flex items-center gap-2">
        <div className={`w-3 h-3 rounded-full ${isConnected ? 'bg-green-500' : 'bg-red-500'} animate-pulse`} />
        <span className="text-white/70 text-sm">
          {isConnected ? (isActive ? 'Live' : 'Connected') : 'Disconnected'}
        </span>
      </div>

      {/* Grid Toggle */}
      <button
        onClick={() => setShowGrid(!showGrid)}
        className="absolute top-4 right-4 px-3 py-1.5 bg-white/10 hover:bg-white/20 rounded-lg text-white/70 text-sm transition-colors"
      >
        Grid {showGrid ? 'On' : 'Off'}
      </button>

      {/* Status Message */}
      {!isActive && isConnected && (
        <div className="absolute bottom-4 left-1/2 transform -translate-x-1/2 px-4 py-2 bg-yellow-500/20 border border-yellow-500/50 rounded-lg">
          <span className="text-yellow-500 text-sm">Waiting for live view stream...</span>
        </div>
      )}
    </div>
  )
}
