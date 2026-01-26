import { useState, useEffect } from 'react'
import { useAppStore } from '../store/useAppStore'
import { api, SDKOption, CanonCameraSettings, CanonSupportedValues } from '../services/api'

// GroupBox component for organizing settings
function GroupBox({ title, children }: { title: string; children: React.ReactNode }) {
  return (
    <div className="border border-gray-700 rounded-lg mb-4">
      <div className="bg-gray-800 px-4 py-2 rounded-t-lg border-b border-gray-700">
        <h3 className="text-sm font-semibold text-gray-200">{title}</h3>
      </div>
      <div className="p-4 space-y-4">
        {children}
      </div>
    </div>
  )
}

// Select component for SDK options
function SDKSelect({
  label,
  options,
  value,
  onChange,
  disabled = false
}: {
  label: string;
  options: SDKOption[];
  value: number;
  onChange: (code: number) => void;
  disabled?: boolean;
}) {
  return (
    <div>
      <label className="block text-sm font-medium mb-1 text-gray-300">{label}</label>
      <select
        value={value}
        onChange={(e) => onChange(parseInt(e.target.value))}
        disabled={disabled || options.length === 0}
        className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary disabled:opacity-50"
      >
        {options.length === 0 ? (
          <option value={0}>No options available</option>
        ) : (
          options.map((opt) => (
            <option key={opt.code} value={opt.code}>
              {opt.label}
            </option>
          ))
        )}
      </select>
    </div>
  )
}

// Toggle component
function Toggle({
  label,
  checked,
  onChange,
  description
}: {
  label: string;
  checked: boolean;
  onChange: (checked: boolean) => void;
  description?: string;
}) {
  return (
    <div className="flex items-center justify-between">
      <div>
        <label className="text-sm font-medium text-gray-300">{label}</label>
        {description && <p className="text-xs text-gray-500">{description}</p>}
      </div>
      <button
        onClick={() => onChange(!checked)}
        className={`relative w-12 h-6 rounded-full transition ${
          checked ? 'bg-primary' : 'bg-gray-600'
        }`}
      >
        <div
          className={`absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full transition-transform ${
            checked ? 'translate-x-6' : 'translate-x-0'
          }`}
        />
      </button>
    </div>
  )
}

export default function CameraSettings() {
  const { settings, updateSettings } = useAppStore()

  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  // SDK supported values
  const [supportedValues, setSupportedValues] = useState<CanonSupportedValues>({
    iso: [],
    aperture: [],
    shutterSpeed: [],
    exposureComp: [],
    whiteBalance: [],
    pictureStyle: [],
    afMode: [],
    imageQuality: [],
    driveMode: [],
    aeMode: []
  })

  // Current camera settings
  const [cameraSettings, setCameraSettings] = useState<CanonCameraSettings>({
    isoCode: 0x48,
    apertureCode: 0x30,
    shutterSpeedCode: 0x70,
    exposureCompCode: 0x00,
    meteringModeCode: 0,
    aeModeCode: 0,
    whiteBalanceCode: 0,
    pictureStyleCode: 0x81,
    afModeCode: 0,
    imageQualityCode: 0,
    driveModeCode: 0,
    evfOutputDevice: 2,
    evfZoom: 1,
    mirror: false,
    rotation: 0
  })

  // Fetch supported values and current settings on mount
  useEffect(() => {
    const fetchData = async () => {
      try {
        setLoading(true)
        setError(null)

        // Fetch supported values
        const supportedResponse = await api.getSupportedCameraValues()
        if (supportedResponse.success && supportedResponse.data) {
          setSupportedValues(supportedResponse.data)
        }

        // Fetch current settings
        const settingsResponse = await api.getExtendedCameraSettings()
        if (settingsResponse.success && settingsResponse.data) {
          setCameraSettings(settingsResponse.data)
        }
      } catch (err) {
        setError('Failed to fetch camera settings')
        console.error('Failed to fetch camera settings:', err)
      } finally {
        setLoading(false)
      }
    }

    fetchData()
  }, [])

  // Update a single camera property
  const updateProperty = async (key: keyof CanonCameraSettings, code: number) => {
    try {
      setCameraSettings(prev => ({ ...prev, [key]: code }))
      await api.updateExtendedCameraSettings({ [key]: code })
    } catch (err) {
      console.error('Failed to update camera property:', err)
    }
  }

  // Update local app settings (mirror, rotation)
  const updateLocalSetting = (key: 'mirror' | 'rotation', value: boolean | number) => {
    setCameraSettings(prev => ({ ...prev, [key]: value }))

    // Also update zustand store for UI state
    if (key === 'mirror') {
      updateSettings({
        cameraSettings: { ...settings.cameraSettings, mirror: value as boolean }
      })
    } else if (key === 'rotation') {
      updateSettings({
        cameraSettings: { ...settings.cameraSettings, rotation: value as 0 | 90 | 180 | 270 }
      })
    }

    // Send to backend
    api.updateExtendedCameraSettings({ [key]: value })
  }

  if (loading) {
    return (
      <div className="p-6 flex items-center justify-center">
        <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-primary"></div>
        <span className="ml-3 text-gray-400">Loading camera settings...</span>
      </div>
    )
  }

  if (error) {
    return (
      <div className="p-6">
        <div className="bg-red-900/20 border border-red-800 rounded-lg p-4 text-red-400">
          {error}
        </div>
      </div>
    )
  }

  return (
    <div className="space-y-4">
      {/* Exposure Control */}
      <GroupBox title="Exposure Control">
        <div className="grid grid-cols-2 gap-4">
          <SDKSelect
            label="ISO"
            options={supportedValues.iso}
            value={cameraSettings.isoCode}
            onChange={(code) => updateProperty('isoCode', code)}
          />
          <SDKSelect
            label="Aperture"
            options={supportedValues.aperture}
            value={cameraSettings.apertureCode}
            onChange={(code) => updateProperty('apertureCode', code)}
          />
          <SDKSelect
            label="Shutter Speed"
            options={supportedValues.shutterSpeed}
            value={cameraSettings.shutterSpeedCode}
            onChange={(code) => updateProperty('shutterSpeedCode', code)}
          />
          <SDKSelect
            label="Exposure Compensation"
            options={supportedValues.exposureComp}
            value={cameraSettings.exposureCompCode}
            onChange={(code) => updateProperty('exposureCompCode', code)}
          />
        </div>
        <SDKSelect
          label="Shooting Mode (AE Mode)"
          options={supportedValues.aeMode}
          value={cameraSettings.aeModeCode}
          onChange={(code) => updateProperty('aeModeCode', code)}
        />
      </GroupBox>

      {/* White Balance & Color */}
      <GroupBox title="White Balance & Color">
        <div className="grid grid-cols-2 gap-4">
          <SDKSelect
            label="White Balance"
            options={supportedValues.whiteBalance}
            value={cameraSettings.whiteBalanceCode}
            onChange={(code) => updateProperty('whiteBalanceCode', code)}
          />
          <SDKSelect
            label="Picture Style"
            options={supportedValues.pictureStyle}
            value={cameraSettings.pictureStyleCode}
            onChange={(code) => updateProperty('pictureStyleCode', code)}
          />
        </div>
      </GroupBox>

      {/* Focus & AF */}
      <GroupBox title="Focus & AF">
        <SDKSelect
          label="AF Mode"
          options={supportedValues.afMode}
          value={cameraSettings.afModeCode}
          onChange={(code) => updateProperty('afModeCode', code)}
        />
      </GroupBox>

      {/* Image Quality */}
      <GroupBox title="Image Quality">
        <div className="grid grid-cols-2 gap-4">
          <SDKSelect
            label="Image Quality"
            options={supportedValues.imageQuality}
            value={cameraSettings.imageQualityCode}
            onChange={(code) => updateProperty('imageQualityCode', code)}
          />
          <SDKSelect
            label="Drive Mode"
            options={supportedValues.driveMode}
            value={cameraSettings.driveModeCode}
            onChange={(code) => updateProperty('driveModeCode', code)}
          />
        </div>
      </GroupBox>

      {/* Live View Settings */}
      <GroupBox title="Live View Settings">
        <Toggle
          label="Mirror Image"
          checked={cameraSettings.mirror}
          onChange={(checked) => updateLocalSetting('mirror', checked)}
          description="Flip the live view horizontally"
        />

        <div>
          <label className="block text-sm font-medium mb-2 text-gray-300">Rotation</label>
          <div className="grid grid-cols-4 gap-2">
            {[0, 90, 180, 270].map((rotation) => (
              <button
                key={rotation}
                onClick={() => updateLocalSetting('rotation', rotation)}
                className={`px-3 py-2 rounded-lg transition ${
                  cameraSettings.rotation === rotation
                    ? 'bg-primary text-white'
                    : 'bg-dark-lighter hover:bg-gray-600'
                }`}
              >
                {rotation}°
              </button>
            ))}
          </div>
        </div>
      </GroupBox>

      {/* Auto Settings */}
      <GroupBox title="Auto Settings">
        <Toggle
          label="Auto Preview"
          checked={settings.autoPreview}
          onChange={(checked) => updateSettings({ autoPreview: checked })}
          description="Show preview after capture"
        />

        <Toggle
          label="Auto Print"
          checked={settings.autoPrint}
          onChange={(checked) => updateSettings({ autoPrint: checked })}
          description="Print automatically after capture"
        />

        {settings.autoPrint && (
          <div>
            <label className="block text-sm font-medium mb-1 text-gray-300">Print Copies</label>
            <input
              type="number"
              min="1"
              max="10"
              value={settings.printCopies}
              onChange={(e) => updateSettings({ printCopies: parseInt(e.target.value) })}
              className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
            />
          </div>
        )}
      </GroupBox>
    </div>
  )
}
