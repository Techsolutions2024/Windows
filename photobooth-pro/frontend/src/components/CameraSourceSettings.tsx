import { useState, useEffect } from 'react'
import { Camera, EventConfig } from '../types'
import api from '../services/api'

interface CameraSourceSettingsProps {
  eventId?: number
  config?: Partial<EventConfig>
  onChange?: (config: Partial<EventConfig>) => void
}

export default function CameraSourceSettings({
  eventId,
  config,
  onChange
}: CameraSourceSettingsProps) {
  const [cameras, setCameras] = useState<Camera[]>([])
  const [loading, setLoading] = useState(true)
  const [cameraSource, setCameraSource] = useState<'canon' | 'webcam'>(
    config?.cameraSource || 'canon'
  )
  const [webcamIndex, setWebcamIndex] = useState(config?.webcamIndex || 0)
  const [testingCamera, setTestingCamera] = useState(false)

  useEffect(() => {
    fetchCameras()
  }, [])

  useEffect(() => {
    if (config) {
      setCameraSource(config.cameraSource || 'canon')
      setWebcamIndex(config.webcamIndex || 0)
    }
  }, [config])

  const fetchCameras = async () => {
    try {
      setLoading(true)
      const response = await api.getCameras()
      if (response.success) {
        setCameras(response.data)
      }
    } catch (error) {
      console.error('Failed to fetch cameras:', error)
    } finally {
      setLoading(false)
    }
  }

  const handleCameraSourceChange = (source: 'canon' | 'webcam') => {
    setCameraSource(source)
    onChange?.({ cameraSource: source, webcamIndex })
  }

  const handleWebcamIndexChange = (index: number) => {
    setWebcamIndex(index)
    onChange?.({ cameraSource, webcamIndex: index })
  }

  const testCamera = async () => {
    try {
      setTestingCamera(true)
      const response = await api.selectCamera({
        type: cameraSource,
        webcamIndex: cameraSource === 'webcam' ? webcamIndex : undefined
      })
      if (response.success) {
        // Start live view briefly to test
        await api.startLiveView()
        setTimeout(async () => {
          await api.stopLiveView()
          setTestingCamera(false)
        }, 3000)
      } else {
        setTestingCamera(false)
        alert('Failed to connect to camera')
      }
    } catch (error) {
      console.error('Test camera error:', error)
      setTestingCamera(false)
      alert('Failed to test camera')
    }
  }

  const canonCameras = cameras.filter((c) => c.type === 'canon')
  const webcams = cameras.filter((c) => c.type === 'webcam')

  return (
    <div className="space-y-6">
      <h3 className="text-lg font-semibold text-white">Camera Source</h3>

      {/* Camera Type Selection */}
      <div className="grid grid-cols-2 gap-4">
        <button
          onClick={() => handleCameraSourceChange('canon')}
          className={`p-4 rounded-xl border-2 transition-all ${
            cameraSource === 'canon'
              ? 'border-blue-500 bg-blue-500/20'
              : 'border-gray-600 bg-gray-800 hover:border-gray-500'
          }`}
        >
          <div className="text-3xl mb-2">📷</div>
          <div className="font-medium text-white">Canon DSLR</div>
          <div className="text-sm text-gray-400 mt-1">
            {canonCameras.length > 0
              ? `${canonCameras.length} camera(s) detected`
              : 'No cameras detected'}
          </div>
        </button>

        <button
          onClick={() => handleCameraSourceChange('webcam')}
          className={`p-4 rounded-xl border-2 transition-all ${
            cameraSource === 'webcam'
              ? 'border-blue-500 bg-blue-500/20'
              : 'border-gray-600 bg-gray-800 hover:border-gray-500'
          }`}
        >
          <div className="text-3xl mb-2">🎥</div>
          <div className="font-medium text-white">Webcam</div>
          <div className="text-sm text-gray-400 mt-1">
            {webcams.length > 0
              ? `${webcams.length} webcam(s) available`
              : 'No webcams detected'}
          </div>
        </button>
      </div>

      {/* Webcam Selection */}
      {cameraSource === 'webcam' && webcams.length > 0 && (
        <div className="space-y-3">
          <label className="block text-sm font-medium text-gray-300">
            Select Webcam
          </label>
          <select
            value={webcamIndex}
            onChange={(e) => handleWebcamIndexChange(Number(e.target.value))}
            className="w-full px-4 py-3 bg-gray-800 border border-gray-600 rounded-lg text-white focus:ring-2 focus:ring-blue-500 focus:border-transparent"
          >
            {webcams.map((cam) => (
              <option key={cam.webcamIndex} value={cam.webcamIndex}>
                {cam.name}
              </option>
            ))}
          </select>
        </div>
      )}

      {/* Canon Camera List */}
      {cameraSource === 'canon' && canonCameras.length > 0 && (
        <div className="space-y-3">
          <label className="block text-sm font-medium text-gray-300">
            Detected Canon Cameras
          </label>
          <div className="space-y-2">
            {canonCameras.map((cam, idx) => (
              <div
                key={idx}
                className={`p-3 rounded-lg border ${
                  cam.connected
                    ? 'border-green-500 bg-green-500/10'
                    : 'border-gray-600 bg-gray-800'
                }`}
              >
                <div className="flex items-center justify-between">
                  <span className="text-white">{cam.name}</span>
                  <span
                    className={`text-sm ${
                      cam.connected ? 'text-green-400' : 'text-gray-400'
                    }`}
                  >
                    {cam.connected ? 'Connected' : 'Available'}
                  </span>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* No Cameras Warning */}
      {loading ? (
        <div className="text-center py-8">
          <div className="animate-spin w-8 h-8 border-2 border-blue-500 border-t-transparent rounded-full mx-auto"></div>
          <p className="text-gray-400 mt-2">Detecting cameras...</p>
        </div>
      ) : cameras.length === 0 ? (
        <div className="text-center py-8 bg-yellow-500/10 border border-yellow-500/30 rounded-lg">
          <div className="text-3xl mb-2">⚠️</div>
          <p className="text-yellow-500">No cameras detected</p>
          <p className="text-gray-400 text-sm mt-1">
            Please connect a camera and refresh
          </p>
          <button
            onClick={fetchCameras}
            className="mt-4 px-4 py-2 bg-yellow-500/20 hover:bg-yellow-500/30 text-yellow-500 rounded-lg transition-colors"
          >
            Refresh
          </button>
        </div>
      ) : null}

      {/* Test Camera Button */}
      <button
        onClick={testCamera}
        disabled={testingCamera || cameras.length === 0}
        className={`w-full py-3 rounded-lg font-medium transition-colors ${
          testingCamera
            ? 'bg-blue-500/50 cursor-not-allowed'
            : 'bg-blue-500 hover:bg-blue-600'
        } text-white`}
      >
        {testingCamera ? (
          <span className="flex items-center justify-center gap-2">
            <div className="animate-spin w-4 h-4 border-2 border-white border-t-transparent rounded-full"></div>
            Testing Camera...
          </span>
        ) : (
          'Test Camera Connection'
        )}
      </button>

      {/* Info */}
      <div className="text-sm text-gray-400 space-y-1">
        <p>
          <strong>Canon DSLR:</strong> Requires Canon EOS camera connected via USB.
          Supports full manual controls (ISO, aperture, shutter speed).
        </p>
        <p>
          <strong>Webcam:</strong> Uses your computer's built-in or external webcam.
          Limited to auto exposure settings.
        </p>
      </div>
    </div>
  )
}
