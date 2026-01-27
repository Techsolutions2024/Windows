import { useState, useEffect } from 'react'
import { useNavigate, useSearchParams } from 'react-router-dom'
import { ArrowLeft, Camera, Monitor, Printer, Wifi, Database, Info, Timer, Image as ImageIcon, Save } from 'lucide-react'
import { useAppStore } from '../store/useAppStore'
import CameraSettings from '../components/CameraSettings'
import CameraInfo from '../components/CameraInfo'
import CaptureSettings from '../components/CaptureSettings'
import LayoutSettings from '../components/LayoutSettings'
import api from '../services/api'

export default function SettingsPage() {
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const { settings, updateSettings, currentEvent, setCurrentEvent } = useAppStore()
  const [activeTab, setActiveTab] = useState('camera')
  const [saving, setSaving] = useState(false)

  // Load event from URL if present
  useEffect(() => {
    const loadEvent = async () => {
      const eventId = searchParams.get('eventId')
      if (eventId) {
        try {
          const response = await api.getEvent(parseInt(eventId))
          if (response.success && response.data) {
            setCurrentEvent(response.data)

            // Also load event-specific camera settings if available
            if (response.data.config?.cameraSettings) {
              try {
                const eventCameraSettings = JSON.parse(response.data.config.cameraSettings)
                updateSettings({ cameraSettings: eventCameraSettings })
              } catch (e) {
                console.error("Failed to parse event camera settings", e)
              }
            }
          }
        } catch (error) {
          console.error("Failed to load event", error)
        }
      }
    }
    loadEvent()
  }, [searchParams, setCurrentEvent, updateSettings])

  const handleSaveConfiguration = async () => {
    if (!currentEvent) {
      alert('No event selected')
      return
    }

    try {
      setSaving(true)

      // aggregated config
      const fullConfig = {
        cameraSettings: JSON.stringify(settings.cameraSettings),
        printSettings: JSON.stringify({
          autoPrint: settings.autoPrint,
          printCopies: settings.printCopies
        }),
        displaySettings: JSON.stringify({
          screenOrientation: settings.screenOrientation,
          autoPreview: settings.autoPreview
        }),
        layoutSettings: JSON.stringify(settings.layout),
        captureSettings: JSON.stringify({
          countdownDuration: settings.countdownDuration,
          captureMode: settings.captureMode,
          photoInterval: settings.photoInterval
        })
      };

      // We explicitly send these fields as well because the backend EventConfig has specific columns for them
      // but we also want to save the full structure to the file system.
      // The backend needs to be updated to handle these extra fields or we pack them into existing ones.
      // For now, we will pack 'printSettings', 'displaySettings' etc into the 'propsConfig' or 'effectsConfig' 
      // if there are no dedicated columns, OR we strictly rely on the backend saving the "generic" config to file.

      // Let's send the specific cameraSettings as before, plus a new 'fullConfig' field if the API supports it.
      // Since I can't easily change the frontend API type definition without seeing it (api.ts), 
      // I will assume updateEventConfig accepts an object.

      await api.updateEventConfig(currentEvent.id, {
        cameraSettings: fullConfig.cameraSettings,
        // We map other settings to available fields in EventConfig if possible, 
        // or we rely on the backend to pick up extra JSON fields.
        // Mapped fields based on HTTPServer.cpp:
        countdownSeconds: settings.countdownDuration,
        photoCount: settings.layout.layouts.find(l => l.id === settings.layout.selectedLayoutId)?.photoCount || 1,
        layoutTemplate: settings.layout.layouts.find(l => l.id === settings.layout.selectedLayoutId)?.path || '',

        // Custom fields that we want the backend to save to file:
        // (The backend HTTPServer.cpp needs to look for these)
        printSettings: fullConfig.printSettings,
        displaySettings: fullConfig.displaySettings,
        layoutSettings: fullConfig.layoutSettings,
        captureSettings: fullConfig.captureSettings
      })

      // We also update the global camera settings just in case
      await api.updateCameraSettings(settings.cameraSettings)

      alert('Event configuration saved successfully!')
    } catch (error) {
      console.error('Failed to save configuration:', error)
      alert('Failed to save configuration')
    } finally {
      setSaving(false)
    }
  }

  const tabs = [
    { id: 'camera', name: 'Camera', icon: Camera },
    { id: 'capture', name: 'Capture', icon: Timer },
    { id: 'layout', name: 'Layout', icon: ImageIcon },
    { id: 'display', name: 'Display', icon: Monitor },
    { id: 'print', name: 'Print', icon: Printer },
    { id: 'network', name: 'Network', icon: Wifi },
    { id: 'storage', name: 'Storage', icon: Database },
    { id: 'about', name: 'About', icon: Info },
  ]

  return (
    <div className="w-full h-screen bg-dark-darker flex flex-col">
      {/* Top Bar */}
      <div className="bg-dark border-b border-gray-700 px-6 py-3 flex items-center justify-between">
        <div className="flex items-center gap-4">
          <button
            onClick={() => navigate('/')}
            className="p-2 hover:bg-dark-lighter rounded-lg transition"
          >
            <ArrowLeft className="w-5 h-5" />
          </button>
          <h1 className="text-xl font-semibold">Settings</h1>
        </div>

        {/* Save Configuration Button */}
        <button
          onClick={handleSaveConfiguration}
          disabled={saving || !currentEvent}
          className={`flex items-center gap-2 px-4 py-2 rounded-lg font-medium transition-colors ${saving || !currentEvent
            ? 'bg-gray-600 cursor-not-allowed opacity-50'
            : 'bg-blue-500 hover:bg-blue-600'
            } text-white`}
        >
          <Save className="w-4 h-4" />
          {saving ? 'Saving...' : 'Save Configuration'}
        </button>
      </div>

      {/* Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Sidebar */}
        <div className="w-64 bg-dark border-r border-gray-700 p-4">
          <div className="space-y-1">
            {tabs.map((tab) => {
              const Icon = tab.icon
              return (
                <button
                  key={tab.id}
                  onClick={() => setActiveTab(tab.id)}
                  className={`w-full flex items-center gap-3 px-4 py-3 rounded-lg transition ${activeTab === tab.id
                    ? 'bg-primary text-white'
                    : 'hover:bg-dark-lighter text-gray-300'
                    }`}
                >
                  <Icon className="w-5 h-5" />
                  <span>{tab.name}</span>
                </button>
              )
            })}
          </div>
        </div>

        {/* Main Content */}
        <div className="flex-1 overflow-y-auto p-8">
          {activeTab === 'camera' && (
            <div className="max-w-3xl space-y-6">
              <h2 className="text-2xl font-semibold mb-6">Camera Settings</h2>

              {/* Camera Status - Single Line */}
              <div>
                <label className="block text-sm font-medium text-gray-400 mb-2">Camera Status</label>
                <CameraInfo />
              </div>

              {/* Camera Parameters - Group Box */}
              <div className="bg-dark p-6 rounded-xl border border-gray-700">
                <h3 className="text-lg font-semibold text-white mb-2">Camera Parameters</h3>
                <p className="text-sm text-gray-400 mb-6">
                  Configure manual camera settings for Canon DSLR. These settings will be saved per event.
                </p>
                <CameraSettings />
              </div>
            </div>
          )}

          {activeTab === 'capture' && <CaptureSettings />}
          {activeTab === 'layout' && <LayoutSettings />}

          {activeTab === 'display' && (
            <div className="max-w-2xl">
              <h2 className="text-2xl font-semibold mb-6">Display Settings</h2>

              {/* Screen Orientation */}
              <div className="mb-6">
                <label className="block text-sm font-medium mb-2">Screen Orientation</label>
                <div className="grid grid-cols-2 gap-3">
                  <button
                    onClick={() => updateSettings({ screenOrientation: 'landscape' })}
                    className={`px-4 py-3 rounded-lg border-2 transition ${settings.screenOrientation === 'landscape'
                      ? 'border-primary bg-primary/10'
                      : 'border-gray-700 hover:border-gray-600'
                      }`}
                  >
                    <div className="text-sm font-medium">Landscape</div>
                    <div className="text-xs text-gray-400">1920 x 1080</div>
                  </button>
                  <button
                    onClick={() => updateSettings({ screenOrientation: 'portrait' })}
                    className={`px-4 py-3 rounded-lg border-2 transition ${settings.screenOrientation === 'portrait'
                      ? 'border-primary bg-primary/10'
                      : 'border-gray-700 hover:border-gray-600'
                      }`}
                  >
                    <div className="text-sm font-medium">Portrait</div>
                    <div className="text-xs text-gray-400">1080 x 1920</div>
                  </button>
                </div>
              </div>

              {/* Fullscreen */}
              <div className="flex items-center justify-between p-4 bg-dark-lighter rounded-lg mb-4">
                <div>
                  <div className="font-medium">Fullscreen Mode</div>
                  <div className="text-sm text-gray-400">Launch in fullscreen</div>
                </div>
                <button className="relative w-12 h-6 bg-primary rounded-full">
                  <div className="absolute top-0.5 right-0.5 w-5 h-5 bg-white rounded-full"></div>
                </button>
              </div>

              {/* Show Grid */}
              <div className="flex items-center justify-between p-4 bg-dark-lighter rounded-lg">
                <div>
                  <div className="font-medium">Show Grid Overlay</div>
                  <div className="text-sm text-gray-400">Display composition grid</div>
                </div>
                <button className="relative w-12 h-6 bg-gray-600 rounded-full">
                  <div className="absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full"></div>
                </button>
              </div>
            </div>
          )}

          {activeTab === 'print' && (
            <div className="max-w-2xl">
              <h2 className="text-2xl font-semibold mb-6">Print Settings</h2>

              {/* Printer Selection */}
              <div className="mb-6">
                <label className="block text-sm font-medium mb-2">Select Printer</label>
                <select className="w-full px-4 py-3 bg-dark-lighter rounded-lg">
                  <option>Canon SELPHY CP1500</option>
                  <option>DNP DS620</option>
                  <option>Mitsubishi CP-D90DW</option>
                </select>
              </div>

              {/* Print Size */}
              <div className="mb-6">
                <label className="block text-sm font-medium mb-2">Default Print Size</label>
                <div className="grid grid-cols-3 gap-3">
                  {['4x6', '5x7', '6x8'].map((size) => (
                    <button
                      key={size}
                      className="px-4 py-3 bg-dark-lighter hover:bg-gray-600 rounded-lg transition"
                    >
                      {size}"
                    </button>
                  ))}
                </div>
              </div>

              {/* Auto Print */}
              <div className="flex items-center justify-between p-4 bg-dark-lighter rounded-lg mb-4">
                <div>
                  <div className="font-medium">Auto Print</div>
                  <div className="text-sm text-gray-400">Print automatically after capture</div>
                </div>
                <button
                  onClick={() => updateSettings({ autoPrint: !settings.autoPrint })}
                  className={`relative w-12 h-6 rounded-full ${settings.autoPrint ? 'bg-primary' : 'bg-gray-600'
                    }`}
                >
                  <div
                    className={`absolute top-0.5 w-5 h-5 bg-white rounded-full transition-transform ${settings.autoPrint ? 'right-0.5' : 'left-0.5'
                      }`}
                  />
                </button>
              </div>

              {/* Default Copies */}
              <div>
                <label className="block text-sm font-medium mb-2">Default Copies</label>
                <input
                  type="number"
                  min="1"
                  max="10"
                  value={settings.printCopies}
                  onChange={(e) => updateSettings({ printCopies: parseInt(e.target.value) })}
                  className="w-full px-4 py-3 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
                />
              </div>
            </div>
          )}

          {activeTab === 'network' && (
            <div className="max-w-2xl">
              <h2 className="text-2xl font-semibold mb-6">Network Settings</h2>

              {/* Server Status */}
              <div className="p-4 bg-dark-lighter rounded-lg mb-6">
                <div className="flex items-center justify-between mb-2">
                  <span className="font-medium">Backend Server</span>
                  <div className="flex items-center gap-2">
                    <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                    <span className="text-sm text-green-500">Connected</span>
                  </div>
                </div>
                <div className="text-sm text-gray-400">
                  <div>HTTP: http://localhost:8080</div>
                  <div>WebSocket: ws://localhost:8081</div>
                </div>
              </div>

              {/* WiFi Settings */}
              <div className="mb-6">
                <label className="block text-sm font-medium mb-2">WiFi Network</label>
                <select className="w-full px-4 py-3 bg-dark-lighter rounded-lg">
                  <option>Home Network</option>
                  <option>Event WiFi</option>
                  <option>Mobile Hotspot</option>
                </select>
              </div>
            </div>
          )}

          {activeTab === 'storage' && (
            <div className="max-w-2xl">
              <h2 className="text-2xl font-semibold mb-6">Storage Settings</h2>

              {/* Storage Location */}
              <div className="mb-6">
                <label className="block text-sm font-medium mb-2">Photo Storage Location</label>
                <div className="flex gap-2">
                  <input
                    type="text"
                    value="D:/PhotoboothPro/Photos"
                    readOnly
                    className="flex-1 px-4 py-3 bg-dark-lighter rounded-lg"
                  />
                  <button className="px-4 py-3 bg-primary hover:bg-primary-dark rounded-lg transition">
                    Browse
                  </button>
                </div>
              </div>

              {/* Storage Stats */}
              <div className="p-4 bg-dark-lighter rounded-lg mb-6">
                <div className="flex items-center justify-between mb-2">
                  <span className="font-medium">Storage Used</span>
                  <span className="text-primary">2.4 GB / 100 GB</span>
                </div>
                <div className="w-full bg-dark h-2 rounded-full overflow-hidden">
                  <div className="bg-primary h-full" style={{ width: '2.4%' }}></div>
                </div>
              </div>

              {/* Auto Cleanup */}
              <div className="flex items-center justify-between p-4 bg-dark-lighter rounded-lg">
                <div>
                  <div className="font-medium">Auto Cleanup</div>
                  <div className="text-sm text-gray-400">Delete photos older than 30 days</div>
                </div>
                <button className="relative w-12 h-6 bg-gray-600 rounded-full">
                  <div className="absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full"></div>
                </button>
              </div>
            </div>
          )}

          {activeTab === 'about' && (
            <div className="max-w-2xl">
              <h2 className="text-2xl font-semibold mb-6">About Photobooth Pro</h2>

              <div className="text-center mb-8">
                <div className="w-24 h-24 bg-primary rounded-2xl mx-auto mb-4 flex items-center justify-center">
                  <Camera className="w-12 h-12 text-white" />
                </div>
                <h3 className="text-2xl font-bold mb-2">Photobooth Pro</h3>
                <p className="text-gray-400">Version 1.0.0</p>
              </div>

              <div className="space-y-4">
                <div className="p-4 bg-dark-lighter rounded-lg">
                  <div className="font-medium mb-1">License</div>
                  <div className="text-sm text-gray-400">Commercial License</div>
                </div>

                <div className="p-4 bg-dark-lighter rounded-lg">
                  <div className="font-medium mb-1">Supported Cameras</div>
                  <div className="text-sm text-gray-400">
                    Canon R100, Canon EOS Series, Webcams
                  </div>
                </div>

                <div className="p-4 bg-dark-lighter rounded-lg">
                  <div className="font-medium mb-1">Technology Stack</div>
                  <div className="text-sm text-gray-400">
                    React, TypeScript, C++, Canon EDSDK
                  </div>
                </div>

                <div className="p-4 bg-dark-lighter rounded-lg">
                  <div className="font-medium mb-1">Developer</div>
                  <div className="text-sm text-gray-400">Photobooth Pro Team</div>
                </div>
              </div>

              <div className="mt-8 flex gap-3">
                <button className="flex-1 px-4 py-3 bg-dark-lighter hover:bg-gray-600 rounded-lg transition">
                  Check for Updates
                </button>
                <button className="flex-1 px-4 py-3 bg-dark-lighter hover:bg-gray-600 rounded-lg transition">
                  Documentation
                </button>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
