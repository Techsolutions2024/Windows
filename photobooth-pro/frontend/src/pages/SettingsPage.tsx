import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { ArrowLeft, Camera, Monitor, Printer, Wifi, Database, Info, Timer, Image as ImageIcon, Save, Check, AlertCircle, Loader2 } from 'lucide-react'
import { useAppStore } from '../store/useAppStore'
import CameraSettings from '../components/CameraSettings'
import CaptureSettings from '../components/CaptureSettings'
import LayoutSettings from '../components/LayoutSettings'
import { api } from '../services/api'

export default function SettingsPage() {
  const navigate = useNavigate()
  const { settings, selectedEvent, updateSettings } = useAppStore()
  const [activeTab, setActiveTab] = useState('camera')
  const [saveStatus, setSaveStatus] = useState<'idle' | 'saving' | 'success' | 'error'>('idle')
  const [saveMessage, setSaveMessage] = useState('')

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

  // Save all configuration to event folder
  const handleSaveConfig = async () => {
    if (!selectedEvent) {
      setSaveStatus('error')
      setSaveMessage('Please select an event first')
      setTimeout(() => setSaveStatus('idle'), 3000)
      return
    }

    try {
      setSaveStatus('saving')
      setSaveMessage('')

      // Gather all settings to save
      const configToSave = {
        camera: settings.cameraSettings,
        capture: {
          countdownSeconds: settings.countdownSeconds,
          photoEnabled: settings.photoEnabled,
          gifEnabled: settings.gifEnabled,
          boomerangEnabled: settings.boomerangEnabled,
          videoEnabled: settings.videoEnabled,
          autoPreview: settings.autoPreview,
        },
        print: {
          autoPrint: settings.autoPrint,
          printCopies: settings.printCopies,
        },
        display: {
          screenOrientation: settings.screenOrientation,
          fullscreen: settings.fullscreen,
          showGrid: settings.showGrid,
        },
        layout: settings.layoutTemplate,
        savedAt: new Date().toISOString(),
      }

      const response = await api.saveEventConfigToFile(selectedEvent.id, configToSave)

      if (response.success) {
        setSaveStatus('success')
        setSaveMessage(`Saved to: ${response.data?.path || 'event folder'}`)
        setTimeout(() => setSaveStatus('idle'), 3000)
      } else {
        setSaveStatus('error')
        setSaveMessage(response.error || 'Failed to save configuration')
        setTimeout(() => setSaveStatus('idle'), 3000)
      }
    } catch (error) {
      console.error('Failed to save config:', error)
      setSaveStatus('error')
      setSaveMessage('Failed to save configuration')
      setTimeout(() => setSaveStatus('idle'), 3000)
    }
  }

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
          {selectedEvent && (
            <span className="text-sm text-gray-400">
              Event: <span className="text-primary">{selectedEvent.name}</span>
            </span>
          )}
        </div>
      </div>

      {/* Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Sidebar */}
        <div className="w-64 bg-dark border-r border-gray-700 p-4 flex flex-col">
          <div className="space-y-1 flex-1">
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

          {/* Save Button in Sidebar Footer */}
          <div className="pt-4 border-t border-white/10">
            <button
              onClick={handleSaveConfig}
              disabled={saveStatus === 'saving'}
              className={`w-full px-4 py-3 rounded-lg font-bold shadow-lg transition-all flex items-center justify-center gap-2
                ${saveStatus === 'success' ? 'bg-green-600 hover:bg-green-600' :
                  saveStatus === 'error' ? 'bg-red-600 hover:bg-red-600' :
                  'bg-green-600 hover:bg-green-700 shadow-green-900/20'
                }
                ${saveStatus === 'saving' ? 'opacity-75 cursor-not-allowed' : ''}
              `}
            >
              {saveStatus === 'saving' && <Loader2 className="w-5 h-5 animate-spin" />}
              {saveStatus === 'success' && <Check className="w-5 h-5" />}
              {saveStatus === 'error' && <AlertCircle className="w-5 h-5" />}
              {saveStatus === 'idle' && <Save className="w-5 h-5" />}
              <span>
                {saveStatus === 'saving' ? 'Saving...' :
                  saveStatus === 'success' ? 'Saved!' :
                  saveStatus === 'error' ? 'Error!' :
                  'Save Configuration'
                }
              </span>
            </button>
            {saveMessage && (
              <p className={`text-xs mt-2 text-center ${
                saveStatus === 'error' ? 'text-red-400' : 'text-green-400'
              }`}>
                {saveMessage}
              </p>
            )}
          </div>
        </div>

        {/* Main Content */}
        <div className="flex-1 overflow-y-auto p-8">
          {activeTab === 'camera' && (
            <div className="max-w-2xl space-y-8">
              <div>
                <h2 className="text-2xl font-semibold mb-6">Camera Settings</h2>
                <p className="text-sm text-gray-400 mb-4">
                  Configure Canon EDSDK camera settings. Values are read directly from the connected camera.
                </p>

                {/* Camera Parameters */}
                <div className="bg-dark p-6 rounded-xl border border-gray-700">
                  <CameraSettings />
                </div>
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
                    value="./Photobooth_Data/events"
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
                    Canon EOS Series (EDSDK)
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
