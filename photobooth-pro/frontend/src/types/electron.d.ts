export interface ElectronAPI {
    // Camera APIs
    listCameras: () => Promise<{ success: boolean; data: any[] }>;
    selectCamera: (config: { type: string; index?: number }) => Promise<{ success: boolean; error?: string }>;

    // Live View APIs
    startLiveView: () => Promise<{ success: boolean; error?: string }>;
    stopLiveView: () => Promise<{ success: boolean }>;
    onLiveViewFrame: (callback: (base64Data: string) => void) => () => void;

    // Capture APIs
    capture: () => Promise<{ success: boolean; data?: { path: string }; error?: string }>;

    // Backend Status
    onBackendError: (callback: (error: any) => void) => () => void;

    // Utility
    isElectron: () => boolean;
    platform: string;
    getBackendURL: () => string;
}

declare global {
    interface Window {
        electronAPI?: ElectronAPI;
    }
}

export { };
