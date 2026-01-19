import { useAppStore } from '../store/useAppStore'

export default function CameraSettings() {
  const { settings, updateSettings } = useAppStore()

  const isoOptions = [100, 200, 400, 800, 1600, 3200, 6400]
  const apertureOptions = ['f/1.4', 'f/2.0', 'f/2.8', 'f/4.0', 'f/5.6', 'f/8.0', 'f/11', 'f/16']
  const shutterSpeedOptions = ['1/4000', '1/2000', '1/1000', '1/500', '1/250', '1/125', '1/60', '1/30']
  const whiteBalanceOptions = ['Auto', 'Daylight', 'Cloudy', 'Tungsten', 'Fluorescent', 'Flash']

  return (
    <div className="p-6 space-y-6">
      <h2 className="text-lg font-semibold mb-4">Camera Settings</h2>

      {/* ISO */}
      <div>
        <label className="block text-sm font-medium mb-2">ISO</label>
        <select
          value={settings.cameraSettings.iso}
          onChange={(e) =>
            updateSettings({
              cameraSettings: { ...settings.cameraSettings, iso: parseInt(e.target.value) },
            })
          }
          className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
        >
          {isoOptions.map((iso) => (
            <option key={iso} value={iso}>
              ISO {iso}
            </option>
          ))}
        </select>
      </div>

      {/* Aperture */}
      <div>
        <label className="block text-sm font-medium mb-2">Aperture</label>
        <select
          value={settings.cameraSettings.aperture}
          onChange={(e) =>
            updateSettings({
              cameraSettings: { ...settings.cameraSettings, aperture: e.target.value },
            })
          }
          className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
        >
          {apertureOptions.map((aperture) => (
            <option key={aperture} value={aperture}>
              {aperture}
            </option>
          ))}
        </select>
      </div>

      {/* Shutter Speed */}
      <div>
        <label className="block text-sm font-medium mb-2">Shutter Speed</label>
        <select
          value={settings.cameraSettings.shutterSpeed}
          onChange={(e) =>
            updateSettings({
              cameraSettings: { ...settings.cameraSettings, shutterSpeed: e.target.value },
            })
          }
          className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
        >
          {shutterSpeedOptions.map((speed) => (
            <option key={speed} value={speed}>
              {speed}
            </option>
          ))}
        </select>
      </div>

      {/* White Balance */}
      <div>
        <label className="block text-sm font-medium mb-2">White Balance</label>
        <select
          value={settings.cameraSettings.whiteBalance}
          onChange={(e) =>
            updateSettings({
              cameraSettings: { ...settings.cameraSettings, whiteBalance: e.target.value },
            })
          }
          className="w-full px-3 py-2 bg-dark-lighter rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
        >
          {whiteBalanceOptions.map((wb) => (
            <option key={wb} value={wb}>
              {wb}
            </option>
          ))}
        </select>
      </div>

      {/* Mirror */}
      <div className="flex items-center justify-between">
        <label className="text-sm font-medium">Mirror Image</label>
        <button
          onClick={() =>
            updateSettings({
              cameraSettings: { ...settings.cameraSettings, mirror: !settings.cameraSettings.mirror },
            })
          }
          className={`relative w-12 h-6 rounded-full transition ${
            settings.cameraSettings.mirror ? 'bg-primary' : 'bg-gray-600'
          }`}
        >
          <div
            className={`absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full transition-transform ${
              settings.cameraSettings.mirror ? 'translate-x-6' : 'translate-x-0'
            }`}
          />
        </button>
      </div>

      {/* Rotation */}
      <div>
        <label className="block text-sm font-medium mb-2">Rotation</label>
        <div className="grid grid-cols-4 gap-2">
          {[0, 90, 180, 270].map((rotation) => (
            <button
              key={rotation}
              onClick={() =>
                updateSettings({
                  cameraSettings: { ...settings.cameraSettings, rotation: rotation as 0 | 90 | 180 | 270 },
                })
              }
              className={`px-3 py-2 rounded-lg transition ${
                settings.cameraSettings.rotation === rotation
                  ? 'bg-primary text-white'
                  : 'bg-dark-lighter hover:bg-gray-600'
              }`}
            >
              {rotation}°
            </button>
          ))}
        </div>
      </div>

      {/* Auto Preview */}
      <div className="flex items-center justify-between">
        <label className="text-sm font-medium">Auto Preview</label>
        <button
          onClick={() => updateSettings({ autoPreview: !settings.autoPreview })}
          className={`relative w-12 h-6 rounded-full transition ${
            settings.autoPreview ? 'bg-primary' : 'bg-gray-600'
          }`}
        >
          <div
            className={`absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full transition-transform ${
              settings.autoPreview ? 'translate-x-6' : 'translate-x-0'
            }`}
          />
        </button>
      </div>

      {/* Auto Print */}
      <div className="flex items-center justify-between">
        <label className="text-sm font-medium">Auto Print</label>
        <button
          onClick={() => updateSettings({ autoPrint: !settings.autoPrint })}
          className={`relative w-12 h-6 rounded-full transition ${
            settings.autoPrint ? 'bg-primary' : 'bg-gray-600'
          }`}
        >
          <div
            className={`absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full transition-transform ${
              settings.autoPrint ? 'translate-x-6' : 'translate-x-0'
            }`}
          />
        </button>
      </div>

      {/* Print Copies */}
      {settings.autoPrint && (
        <div>
          <label className="block text-sm font-medium mb-2">Print Copies</label>
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
    </div>
  )
}
