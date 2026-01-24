import { useEffect, useState, useCallback } from 'react';

export function useElectronCamera() {
    const [cameras, setCameras] = useState<any[]>([]);
    const [liveViewFrame, setLiveViewFrame] = useState<string | null>(null);
    const [isConnected, setIsConnected] = useState(true);
    const [isElectron, setIsElectron] = useState(false);

    useEffect(() => {
        // Check if running in Electron
        if (typeof window !== 'undefined' && window.electronAPI) {
            setIsElectron(true);

            // Setup live view frame listener
            const cleanup = window.electronAPI.onLiveViewFrame((base64Data: string) => {
                // Convert Base64 to data URL
                setLiveViewFrame(`data:image/jpeg;base64,${base64Data}`);
            });

            // Setup backend error listener
            const errorCleanup = window.electronAPI.onBackendError((error) => {
                console.error('Backend error:', error);
                setIsConnected(false);
            });

            return () => {
                cleanup();
                errorCleanup();
            };
        } else {
            console.warn('Not running in Electron - electronAPI not available');
            setIsElectron(false);
        }
    }, []);

    const listCameras = useCallback(async () => {
        if (!window.electronAPI) {
            console.warn('electronAPI not available');
            return;
        }

        try {
            const response = await window.electronAPI.listCameras();
            if (response.success && response.data) {
                setCameras(response.data);
            }
        } catch (error) {
            console.error('Failed to list cameras:', error);
        }
    }, []);

    const selectCamera = useCallback(async (type: string, index?: number) => {
        if (!window.electronAPI) {
            return { success: false, error: 'electronAPI not available' };
        }

        try {
            return await window.electronAPI.selectCamera({ type, index });
        } catch (error) {
            console.error('Failed to select camera:', error);
            return { success: false, error: String(error) };
        }
    }, []);

    const startLiveView = useCallback(async () => {
        if (!window.electronAPI) {
            return { success: false, error: 'electronAPI not available' };
        }

        try {
            return await window.electronAPI.startLiveView();
        } catch (error) {
            console.error('Failed to start live view:', error);
            return { success: false, error: String(error) };
        }
    }, []);

    const stopLiveView = useCallback(async () => {
        if (!window.electronAPI) {
            return { success: false };
        }

        try {
            return await window.electronAPI.stopLiveView();
        } catch (error) {
            console.error('Failed to stop live view:', error);
            return { success: false };
        }
    }, []);

    const capture = useCallback(async () => {
        if (!window.electronAPI) {
            return { success: false, error: 'electronAPI not available' };
        }

        try {
            return await window.electronAPI.capture();
        } catch (error) {
            console.error('Failed to capture:', error);
            return { success: false, error: String(error) };
        }
    }, []);

    return {
        cameras,
        liveViewFrame,
        isConnected,
        isElectron,
        listCameras,
        selectCamera,
        startLiveView,
        stopLiveView,
        capture
    };
}
