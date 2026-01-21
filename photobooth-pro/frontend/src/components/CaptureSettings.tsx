import { useAppStore } from '../store/useAppStore'
import { Clock, Timer, Hourglass } from 'lucide-react'

export default function CaptureSettings() {
    const { settings, updateSettings } = useAppStore()

    return (
        <div className="max-w-2xl">
            <h2 className="text-2xl font-semibold mb-6">Capture Settings</h2>

            {/* Countdown Duration */}
            <div className="mb-6">
                <label className="block text-sm font-medium mb-2 flex items-center gap-2">
                    <Timer className="w-4 h-4 text-primary" />
                    Countdown Duration (seconds)
                </label>
                <div className="grid grid-cols-4 gap-3">
                    {[3, 5, 10].map((duration) => (
                        <button
                            key={duration}
                            onClick={() => updateSettings({ countdownDuration: duration })}
                            className={`px-4 py-3 rounded-lg border-2 transition ${settings.countdownDuration === duration
                                    ? 'border-primary bg-primary/10 text-white'
                                    : 'border-gray-700 hover:border-gray-600 text-gray-400'
                                }`}
                        >
                            <div className="text-lg font-bold">{duration}s</div>
                        </button>
                    ))}
                    <div className="relative">
                        <input
                            type="number"
                            value={settings.countdownDuration}
                            onChange={(e) => updateSettings({ countdownDuration: parseInt(e.target.value) || 0 })}
                            className="w-full h-full px-4 py-3 bg-dark-lighter rounded-lg text-center font-bold focus:outline-none focus:ring-2 focus:ring-primary border-2 border-transparent"
                        />
                        <span className="absolute right-3 top-1/2 -translate-y-1/2 text-sm text-gray-500">custom</span>
                    </div>
                </div>
                <p className="mt-2 text-sm text-gray-500">
                    Time before the first photo is taken.
                </p>
            </div>

            {/* Photo Interval */}
            <div className="mb-6">
                <label className="block text-sm font-medium mb-2 flex items-center gap-2">
                    <Hourglass className="w-4 h-4 text-primary" />
                    Photo Interval (seconds)
                </label>
                <div className="grid grid-cols-4 gap-3">
                    {[1, 3, 5].map((interval) => (
                        <button
                            key={interval}
                            onClick={() => updateSettings({ photoInterval: interval })}
                            className={`px-4 py-3 rounded-lg border-2 transition ${settings.photoInterval === interval
                                    ? 'border-primary bg-primary/10 text-white'
                                    : 'border-gray-700 hover:border-gray-600 text-gray-400'
                                }`}
                        >
                            <div className="text-lg font-bold">{interval}s</div>
                        </button>
                    ))}
                    <div className="relative">
                        <input
                            type="number"
                            value={settings.photoInterval}
                            onChange={(e) => updateSettings({ photoInterval: parseInt(e.target.value) || 0 })}
                            className="w-full h-full px-4 py-3 bg-dark-lighter rounded-lg text-center font-bold focus:outline-none focus:ring-2 focus:ring-primary border-2 border-transparent"
                        />
                        <span className="absolute right-3 top-1/2 -translate-y-1/2 text-sm text-gray-500">custom</span>
                    </div>
                </div>
                <p className="mt-2 text-sm text-gray-500">
                    Delay between consecutive photos in a multi-photo layout.
                </p>
            </div>

            {/* Capture Mode */}
            <div className="mb-6">
                <label className="block text-sm font-medium mb-2 flex items-center gap-2">
                    <Clock className="w-4 h-4 text-primary" />
                    Default Capture Mode
                </label>
                <div className="grid grid-cols-3 gap-3">
                    {['photo', 'gif', 'boomerang', 'video'].map((mode) => (
                        <button
                            key={mode}
                            onClick={() => updateSettings({ captureMode: mode as any })}
                            className={`px-4 py-3 rounded-lg border-2 capitalize transition ${settings.captureMode === mode
                                    ? 'border-primary bg-primary/10 text-white'
                                    : 'border-gray-700 hover:border-gray-600 text-gray-400'
                                }`}
                        >
                            {mode}
                        </button>
                    ))}
                </div>
            </div>
        </div>
    )
}
