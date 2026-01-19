import { Camera } from 'lucide-react'
import { useAppStore } from '../store/useAppStore'

interface CaptureControlsProps {
  onCapture: () => void
}

export default function CaptureControls({ onCapture }: CaptureControlsProps) {
  const { settings, isCapturing } = useAppStore()

  const handleClick = () => {
    if (!isCapturing) {
      onCapture()
    }
  }

  return (
    <div className="flex flex-col items-center gap-4">
      {/* Main Capture Button */}
      <button
        onClick={handleClick}
        disabled={isCapturing}
        className={`w-20 h-20 rounded-full border-4 flex items-center justify-center transition-all ${
          isCapturing
            ? 'border-gray-500 bg-gray-600 cursor-not-allowed'
            : 'border-primary bg-primary hover:bg-primary-dark hover:scale-110 active:scale-95'
        }`}
      >
        <Camera className="w-10 h-10 text-white" />
      </button>

      {/* Countdown Duration Selector */}
      <div className="flex items-center gap-2 bg-dark/80 backdrop-blur-sm rounded-full px-4 py-2">
        <span className="text-sm text-gray-300">Countdown:</span>
        <div className="flex gap-1">
          {[0, 3, 5, 10].map((seconds) => (
            <button
              key={seconds}
              onClick={() => useAppStore.getState().updateSettings({ countdownDuration: seconds })}
              className={`px-3 py-1 rounded-full text-sm transition ${
                settings.countdownDuration === seconds
                  ? 'bg-primary text-white'
                  : 'bg-dark-lighter text-gray-300 hover:bg-gray-600'
              }`}
            >
              {seconds === 0 ? 'Off' : `${seconds}s`}
            </button>
          ))}
        </div>
      </div>

      {/* Capture Mode Info */}
      <div className="bg-dark/80 backdrop-blur-sm rounded-lg px-4 py-2">
        <span className="text-sm text-gray-300">
          Mode: <span className="text-primary font-medium capitalize">{settings.captureMode}</span>
        </span>
      </div>
    </div>
  )
}
