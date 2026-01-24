const { contextBridge, ipcRenderer } = require('electron');

// Expose protected methods that allow the renderer process to use
// ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('electronAPI', {
    // ============================================
    // Camera APIs
    // ============================================

    /**
     * Get list of available cameras
     * @returns Promise<{ success: boolean, data: Array }>
     */
    listCameras: () => ipcRenderer.invoke('camera:list'),

    /**
     * Select a camera to use
     * @param config - { type: 'canon' | 'webcam', index?: number }
     * @returns Promise<{ success: boolean, error?: string }>
     */
    selectCamera: (config) => ipcRenderer.invoke('camera:select', config),

    // ============================================
    // Live View APIs (SSE-based)
    // ============================================

    /**
     * Start live view stream
     * @returns Promise<{ success: boolean, error?: string }>
     */
    startLiveView: () => ipcRenderer.invoke('liveview:start'),

    /**
     * Stop live view stream
     * @returns Promise<{ success: boolean }>
     */
    stopLiveView: () => ipcRenderer.invoke('liveview:stop'),

    /**
     * Listen for live view frames (Base64 JPEG data)
     * @param callback - Function to call when a new frame arrives
     * @returns Function to remove the listener
     */
    onLiveViewFrame: (callback) => {
        const subscription = (event, base64Data) => {
            // base64Data is a Base64-encoded JPEG string
            callback(base64Data);
        };

        ipcRenderer.on('liveview:frame', subscription);

        // Return cleanup function
        return () => {
            ipcRenderer.removeListener('liveview:frame', subscription);
        };
    },

    // ============================================
    // Capture APIs
    // ============================================

    /**
     * Capture a photo
     * @returns Promise<{ success: boolean, data?: { path: string }, error?: string }>
     */
    capture: () => ipcRenderer.invoke('camera:capture'),

    // ============================================
    // Backend Status
    // ============================================

    /**
     * Listen for backend errors
     * @param callback - Function to call when backend crashes
     * @returns Function to remove the listener
     */
    onBackendError: (callback) => {
        const subscription = (event, error) => {
            callback(error);
        };

        ipcRenderer.on('backend:error', subscription);

        return () => {
            ipcRenderer.removeListener('backend:error', subscription);
        };
    },

    // ============================================
    // Utility APIs
    // ============================================

    /**
     * Check if running in Electron
     * @returns boolean
     */
    isElectron: () => true,

    /**
     * Get platform info
     * @returns string
     */
    platform: process.platform,

    /**
     * Get backend URL (for direct HTTP calls if needed)
     * @returns string
     */
    getBackendURL: () => 'http://localhost:8080',
});

// Log that preload script has loaded
console.log('✅ Preload script loaded - electronAPI exposed to window');
console.log('📡 Using HTTP/SSE communication with backend server');
