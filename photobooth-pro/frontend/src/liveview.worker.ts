
// Web Worker for handling LiveView stream
// Optimized for binary transfer and low latency

const WS_URL = 'ws://localhost:8081';
let ws: WebSocket | null = null;
let reconnectTimer: any = null;
let isProcessing = false;

// Connect to WebSocket server
function connect() {
    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
        return;
    }

    console.log('LiveView Worker: Connecting...');
    ws = new WebSocket(WS_URL);
    ws.binaryType = 'arraybuffer';

    ws.onopen = () => {
        console.log('LiveView Worker: Connected');
        // Signal ready for first frame
        ws?.send('READY');
    };

    ws.onmessage = async (event) => {
        try {
            if (event.data instanceof ArrayBuffer) {
                // Create blob from buffer
                const blob = new Blob([event.data], { type: 'image/jpeg' });

                // Create ImageBitmap (off-thread decoding)
                // This is much faster than Image() element on main thread
                const bitmap = await createImageBitmap(blob);

                // Transfer bitmap to main thread (zero-copy)
                self.postMessage({ type: 'FRAME', bitmap }, [bitmap]);
            }
        } catch (e) {
            console.error('LiveView Worker: Error processing frame', e);
            // If failed, still ask for next frame to avoid stall
            requestNextFrame();
        }
    };

    ws.onclose = () => {
        console.log('LiveView Worker: Disconnected');
        cleanup();
        scheduleReconnect();
    };

    ws.onerror = (e) => {
        console.error('LiveView Worker: Error', e);
        // ws.close() will trigger onclose
    };
}

function cleanup() {
    if (ws) {
        ws.onclose = null;
        ws.close();
        ws = null;
    }
}

function scheduleReconnect() {
    if (reconnectTimer) clearTimeout(reconnectTimer);
    reconnectTimer = setTimeout(() => {
        connect();
    }, 2000);
}

function requestNextFrame() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send('READY');
    }
}

self.onmessage = (e: MessageEvent) => {
    const { type } = e.data;

    switch (type) {
        case 'START':
            connect();
            break;

        case 'STOP':
            cleanup();
            if (reconnectTimer) clearTimeout(reconnectTimer);
            break;

        case 'FRAME_PROCESSED':
            // Main thread has drawn the frame, request next one
            requestNextFrame();
            break;
    }
};
