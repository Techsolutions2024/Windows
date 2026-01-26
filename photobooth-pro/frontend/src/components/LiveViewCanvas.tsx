import React, { useEffect, useRef, useState } from 'react';

interface LiveViewCanvasProps {
    width?: number;
    height?: number;
    className?: string;
    active?: boolean;
}

const LiveViewCanvas: React.FC<LiveViewCanvasProps> = ({
    width = 1280,
    height = 720,
    className = '',
    active = true
}) => {
    const canvasRef = useRef<HTMLCanvasElement>(null);
    const workerRef = useRef<Worker | null>(null);
    const [fps, setFps] = useState(0);
    const lastFrameTime = useRef(performance.now());
    const frameCount = useRef(0);
    const lastFpsUpdate = useRef(performance.now());

    useEffect(() => {
        if (!active) {
            if (workerRef.current) {
                workerRef.current.postMessage({ type: 'STOP' });
                workerRef.current.terminate();
                workerRef.current = null;
            }
            return;
        }

        // Initialize Web Worker
        // Using import.meta.url allows Vite/Webpack to resolve the worker file correctly
        try {
            workerRef.current = new Worker(new URL('../liveview.worker.ts', import.meta.url), { type: 'module' });
            const worker = workerRef.current;

            worker.onmessage = (e: MessageEvent) => {
                const { bitmap } = e.data;

                if (bitmap) {
                    const canvas = canvasRef.current;
                    if (canvas) {
                        // Get context optimized for frequent updates
                        const ctx = canvas.getContext('2d', { alpha: false, desynchronized: true });

                        if (ctx) {
                            // Draw the hardware-accelerated bitmap directly
                            ctx.drawImage(bitmap, 0, 0, canvas.width, canvas.height);
                        }
                    }

                    // Critical: Close bitmap to release GPU memory immediately
                    bitmap.close();

                    // FPS Calculation
                    frameCount.current++;
                    const now = performance.now();
                    if (now - lastFpsUpdate.current >= 1000) {
                        setFps(frameCount.current);
                        frameCount.current = 0;
                        lastFpsUpdate.current = now;
                    }

                    // Send 'READY' signal to worker to fetch next frame (Flow Control)
                    worker.postMessage({ type: 'FRAME_PROCESSED' });
                }
            };

            // Start the worker connection
            worker.postMessage({ type: 'START' });

        } catch (err) {
            console.error("Failed to initialize LiveView worker:", err);
        }

        return () => {
            if (workerRef.current) {
                console.log("Terminating LiveView worker");
                workerRef.current.postMessage({ type: 'STOP' });
                workerRef.current.terminate();
                workerRef.current = null;
            }
        };
    }, [active]);

    // Handle canvas scaling
    useEffect(() => {
        const canvas = canvasRef.current;
        if (canvas) {
            canvas.width = width;
            canvas.height = height;
        }
    }, [width, height]);

    return (
        <div className={`relative ${className}`}>
            <canvas
                ref={canvasRef}
                className="w-full h-full object-contain bg-black block" // 'block' prevents extra space
                style={{ imageRendering: 'high-quality' }}
            />
            {/* FPS Indicator */}
            <div className="absolute top-2 right-2 bg-black/60 text-white text-xs font-mono px-2 py-1 rounded backdrop-blur-sm pointer-events-none">
                FPS: {fps}
            </div>
        </div>
    );
};

export default LiveViewCanvas;
