import { useEffect, useRef, useState } from 'react'
import { useAppStore } from '../store/useAppStore'

interface LiveViewDisplayProps {
  filter?: string
}

export default function LiveViewDisplay({ filter = 'none' }: LiveViewDisplayProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const { liveViewFrame, liveViewActive } = useAppStore()
  const [dimensions, setDimensions] = useState({ width: 1920, height: 1080 })

  useEffect(() => {
    // TODO: Connect to WebSocket for live view frames
    // const ws = new WebSocket('ws://localhost:8081')
    // ws.onmessage = (event) => {
    //   // Update live view frame
    // }
    
    // For now, show placeholder
    drawPlaceholder()
  }, [])

  useEffect(() => {
    if (liveViewFrame && canvasRef.current) {
      const canvas = canvasRef.current
      const ctx = canvas.getContext('2d')
      if (ctx) {
        const img = new Image()
        img.onload = () => {
          ctx.drawImage(img, 0, 0, canvas.width, canvas.height)
          applyFilter(ctx, filter)
        }
        img.src = liveViewFrame
      }
    }
  }, [liveViewFrame, filter])

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
    ctx.fillText('Waiting for camera connection...', canvas.width / 2, canvas.height / 2 + 50)
  }

  const applyFilter = (ctx: CanvasRenderingContext2D, filterType: string) => {
    if (filterType === 'none') return

    const imageData = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height)
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
    }

    ctx.putImageData(imageData, 0, 0)
  }

  return (
    <div className="w-full h-full flex items-center justify-center bg-black">
      <canvas
        ref={canvasRef}
        width={dimensions.width}
        height={dimensions.height}
        className="max-w-full max-h-full object-contain"
      />
      
      {/* Grid Overlay (optional) */}
      <div className="absolute inset-0 pointer-events-none">
        <div className="w-full h-full grid grid-cols-3 grid-rows-3 opacity-20">
          {[...Array(9)].map((_, i) => (
            <div key={i} className="border border-white/20"></div>
          ))}
        </div>
      </div>

      {/* Status Indicator */}
      {!liveViewActive && (
        <div className="absolute top-4 left-4 px-3 py-1.5 bg-yellow-500/20 border border-yellow-500/50 rounded-lg">
          <span className="text-yellow-500 text-sm">Live View Inactive</span>
        </div>
      )}
    </div>
  )
}
