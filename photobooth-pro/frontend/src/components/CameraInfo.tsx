import { useState, useEffect } from 'react'
import { Camera as CameraType } from '../types'
import api from '../services/api'

export default function CameraInfo() {
    const [cameras, setCameras] = useState<CameraType[]>([])
    const [loading, setLoading] = useState(true)
    const [refreshing, setRefreshing] = useState(false)

    useEffect(() => {
        fetchCameras()
    }, [])

    const fetchCameras = async () => {
        try {
            setLoading(true)
            const response = await api.getCameras()
            if (response.success && response.data) {
                setCameras(response.data)
            }
        } catch (error) {
            console.error('Failed to fetch cameras:', error)
        } finally {
            setLoading(false)
        }
    }

    const handleRefresh = async () => {
        setRefreshing(true)
        await fetchCameras()
        setRefreshing(false)
    }

    const canonCameras = cameras.filter((c) => c.type === 'canon')
    const connectedCamera = canonCameras.find((c) => c.connected)

    return (
        <div className="flex items-center justify-between py-2 px-4 bg-gray-800/50 rounded-lg border border-gray-700">
            {loading ? (
                <div className="flex items-center gap-3">
                    <div className="animate-spin w-4 h-4 border-2 border-blue-500 border-t-transparent rounded-full"></div>
                    <span className="text-gray-400 text-sm">Detecting cameras...</span>
                </div>
            ) : connectedCamera ? (
                <div className="flex items-center gap-3">
                    <div className="text-xl">📷</div>
                    <div>
                        <span className="text-white font-medium">{connectedCamera.name}</span>
                        <span className="ml-3 text-sm text-green-400 flex items-center gap-1.5">
                            <div className="w-1.5 h-1.5 bg-green-400 rounded-full animate-pulse"></div>
                            Connected
                        </span>
                    </div>
                </div>
            ) : canonCameras.length > 0 ? (
                <div className="flex items-center gap-3">
                    <div className="text-xl">📷</div>
                    <div>
                        <span className="text-white font-medium">{canonCameras[0].name}</span>
                        <span className="ml-3 text-sm text-yellow-500">Not Connected</span>
                    </div>
                </div>
            ) : (
                <div className="flex items-center gap-3">
                    <div className="text-xl">⚠️</div>
                    <span className="text-yellow-500 font-medium">No cameras detected</span>
                </div>
            )}

            <button
                onClick={handleRefresh}
                disabled={refreshing}
                className="px-3 py-1.5 text-sm bg-gray-700 hover:bg-gray-600 text-white rounded-lg transition-colors disabled:opacity-50"
            >
                {refreshing ? (
                    <span className="flex items-center gap-2">
                        <div className="animate-spin w-3 h-3 border-2 border-white border-t-transparent rounded-full"></div>
                        Refreshing...
                    </span>
                ) : (
                    'Refresh'
                )}
            </button>
        </div>
    )
}
