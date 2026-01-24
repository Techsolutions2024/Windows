const { app, BrowserWindow, ipcMain } = require('electron');
const { spawn } = require('child_process');
const path = require('path');
const http = require('http');

let mainWindow;
let backendProcess;
const BACKEND_PORT = 8080;
const BACKEND_URL = `http://localhost:${BACKEND_PORT}`;

// ============================================
// Backend Server Management
// ============================================

function startBackendServer() {
    return new Promise((resolve, reject) => {
        const backendPath = path.join(__dirname, '../backend/build/bin/Release/photobooth-server.exe');

        console.log('🚀 Starting backend server:', backendPath);

        backendProcess = spawn(backendPath, [], {
            cwd: path.join(__dirname, '../backend/build/bin/Release'),
            stdio: ['ignore', 'pipe', 'pipe']
        });

        backendProcess.stdout.on('data', (data) => {
            const output = data.toString();
            console.log(`[Backend] ${output}`);

            // Check if server started successfully
            if (output.includes('Application initialized successfully')) {
                console.log('✅ Backend server ready');
                resolve();
            }
        });

        backendProcess.stderr.on('data', (data) => {
            console.error(`[Backend Error] ${data}`);
        });

        backendProcess.on('error', (error) => {
            console.error('❌ Failed to start backend:', error);
            reject(error);
        });

        backendProcess.on('close', (code) => {
            console.log(`Backend process exited with code ${code}`);
            if (code !== 0 && code !== null) {
                // Backend crashed, notify user
                if (mainWindow) {
                    mainWindow.webContents.send('backend:error', { code });
                }
            }
        });

        // Timeout after 10 seconds
        setTimeout(() => {
            if (backendProcess && !backendProcess.killed) {
                resolve(); // Assume it started
            }
        }, 10000);
    });
}

function stopBackendServer() {
    if (backendProcess && !backendProcess.killed) {
        console.log('🛑 Stopping backend server...');
        backendProcess.kill('SIGTERM');

        // Force kill after 5 seconds if not stopped
        setTimeout(() => {
            if (backendProcess && !backendProcess.killed) {
                backendProcess.kill('SIGKILL');
            }
        }, 5000);
    }
}

// ============================================
// HTTP Helper Functions
// ============================================

function makeRequest(method, endpoint, data = null) {
    return new Promise((resolve, reject) => {
        const options = {
            hostname: 'localhost',
            port: BACKEND_PORT,
            path: endpoint,
            method: method,
            headers: {
                'Content-Type': 'application/json'
            }
        };

        const req = http.request(options, (res) => {
            let body = '';

            res.on('data', (chunk) => {
                body += chunk;
            });

            res.on('end', () => {
                try {
                    const response = JSON.parse(body);
                    resolve(response);
                } catch (e) {
                    resolve({ success: false, error: 'Invalid JSON response' });
                }
            });
        });

        req.on('error', (error) => {
            reject(error);
        });

        if (data) {
            req.write(JSON.stringify(data));
        }

        req.end();
    });
}

// ============================================
// Electron Window Management
// ============================================

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1920,
        height: 1080,
        fullscreen: false, // Set to true for production
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
            devTools: process.env.NODE_ENV !== 'production'
        }
    });

    // Load React app
    if (process.env.NODE_ENV === 'development') {
        mainWindow.loadURL('http://localhost:5173');
        mainWindow.webContents.openDevTools();
    } else {
        mainWindow.loadFile(path.join(__dirname, '../frontend/dist/index.html'));
    }

    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

// ============================================
// IPC Handlers - Camera Management
// ============================================

ipcMain.handle('camera:list', async () => {
    console.log('📷 Listing cameras...');

    try {
        const response = await makeRequest('GET', '/api/cameras');
        console.log(`✅ Found ${response.data?.length || 0} camera(s)`);
        return response;
    } catch (error) {
        console.error('❌ Error listing cameras:', error);
        return {
            success: false,
            error: error.message,
            data: []
        };
    }
});

ipcMain.handle('camera:select', async (event, { type, index }) => {
    console.log(`📷 Selecting camera: ${type} at index ${index}`);

    try {
        const response = await makeRequest('POST', '/api/cameras/select', {
            type,
            webcamIndex: index
        });
        console.log(`✅ Camera selected: ${response.success}`);
        return response;
    } catch (error) {
        console.error('❌ Error selecting camera:', error);
        return { success: false, error: error.message };
    }
});

// ============================================
// IPC Handlers - Live View (SSE)
// ============================================

ipcMain.handle('liveview:start', async () => {
    console.log('🎥 Starting live view...');

    try {
        const response = await makeRequest('POST', '/api/cameras/liveview/start');

        if (response.success) {
            console.log('✅ Live view started');

            // Start SSE connection for frames
            startSSEConnection();
        }

        return response;
    } catch (error) {
        console.error('❌ Error starting live view:', error);
        return { success: false, error: error.message };
    }
});

ipcMain.handle('liveview:stop', async () => {
    console.log('🛑 Stopping live view...');

    try {
        const response = await makeRequest('POST', '/api/cameras/liveview/stop');
        console.log('✅ Live view stopped');
        return response;
    } catch (error) {
        console.error('❌ Error stopping live view:', error);
        return { success: false, error: error.message };
    }
});

// SSE Connection for Live View Frames
let sseRequest = null;

function startSSEConnection() {
    if (sseRequest) {
        sseRequest.destroy();
    }

    const options = {
        hostname: 'localhost',
        port: BACKEND_PORT,
        path: '/api/cameras/liveview/stream',
        method: 'GET',
        headers: {
            'Accept': 'text/event-stream',
            'Cache-Control': 'no-cache',
            'Connection': 'keep-alive'
        }
    };

    sseRequest = http.request(options, (res) => {
        console.log('📡 SSE connection established');

        let buffer = '';

        res.on('data', (chunk) => {
            buffer += chunk.toString();

            // Process complete events
            const events = buffer.split('\n\n');
            buffer = events.pop() || ''; // Keep incomplete event in buffer

            events.forEach(event => {
                if (event.startsWith('data: ')) {
                    const base64Data = event.substring(6).trim();
                    if (base64Data && mainWindow) {
                        // Send frame to renderer
                        mainWindow.webContents.send('liveview:frame', base64Data);
                    }
                }
            });
        });

        res.on('end', () => {
            console.log('📡 SSE connection closed');
            sseRequest = null;
        });
    });

    sseRequest.on('error', (error) => {
        console.error('❌ SSE connection error:', error);
        sseRequest = null;
    });

    sseRequest.end();
}

// ============================================
// IPC Handlers - Capture
// ============================================

ipcMain.handle('camera:capture', async () => {
    console.log('📸 Capturing photo...');

    try {
        const response = await makeRequest('POST', '/api/cameras/capture');
        console.log(`✅ Photo captured: ${response.data?.path}`);
        return response;
    } catch (error) {
        console.error('❌ Error capturing photo:', error);
        return { success: false, error: error.message };
    }
});

// ============================================
// App Lifecycle
// ============================================

app.whenReady().then(async () => {
    try {
        // Start backend server first
        await startBackendServer();

        // Wait a bit for server to be fully ready
        await new Promise(resolve => setTimeout(resolve, 2000));

        // Create Electron window
        createWindow();
    } catch (error) {
        console.error('❌ Failed to start application:', error);
        app.quit();
    }

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('before-quit', () => {
    // Stop SSE connection
    if (sseRequest) {
        sseRequest.destroy();
    }

    // Stop backend server
    stopBackendServer();
});

// Handle uncaught exceptions
process.on('uncaughtException', (error) => {
    console.error('💥 Uncaught Exception:', error);
});

console.log('🎯 Electron app initialized');
