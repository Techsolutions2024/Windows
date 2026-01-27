import { useRef, useState } from 'react'
import { useAppStore } from '../store/useAppStore'
import { Layout } from '../types'
import { Layout as LayoutIcon, Upload, Trash2, CheckCircle } from 'lucide-react'

export default function LayoutSettings() {
    const { settings, updateSettings } = useAppStore()
    const fileInputRef = useRef<HTMLInputElement>(null)
    const canvasRef = useRef<HTMLCanvasElement>(null)
    const [analyzing, setAnalyzing] = useState(false)

    const handleFileUpload = (event: React.ChangeEvent<HTMLInputElement>) => {
        const file = event.target.files?.[0]
        if (!file) return

        setAnalyzing(true)
        const reader = new FileReader()
        reader.onload = (e) => {
            const img = new Image()
            img.onload = () => {
                analyzeLayout(img, file.name)
            }
            img.src = e.target?.result as string
        }
        reader.readAsDataURL(file)
    }

    const analyzeLayout = (img: HTMLImageElement, fileName: string) => {
        const canvas = canvasRef.current
        if (!canvas) return

        canvas.width = img.width
        canvas.height = img.height
        const ctx = canvas.getContext('2d')
        if (!ctx) return

        ctx.drawImage(img, 0, 0)
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height)
        const data = imageData.data

        // Detect separate transparent regions (simplified)
        // A robust algorithm would use connected components.
        // For now, let's just create a Layout object and let user specify or assume generic.
        // User requested: "hệ thống sẽ tính toán vị trí trống".
        // Let's implement a basic Connected Component Labeling for transparent pixels (alpha < 10)

        const photoCount = countTransparentRegions(data, canvas.width, canvas.height);

        const newLayout: Layout = {
            id: Date.now().toString(),
            name: fileName.replace('.png', ''),
            path: img.src, // In real app, upload to server and get URL. Here we use dataURI for demo.
            photoCount: photoCount
        }

        const currentLayouts = settings.layout?.layouts || []
        updateSettings({
            layout: {
                ...settings.layout,
                layouts: [...currentLayouts, newLayout]
            }
        })
        setAnalyzing(false)
    }

    const countTransparentRegions = (data: Uint8ClampedArray, width: number, height: number): number => {
        // Basic Connected Components Algorithm for Alpha channel
        // 0 = unvisited transparent, 1 = background/opaque, >1 = label
        const visited = new Int32Array(width * height).fill(0);
        let labels = 0;
        const threshold = 20; // Alpha threshold

        // Helper to get index
        const idx = (x: number, y: number) => (y * width + x);

        // Mark opaque pixels as visited (-1)
        for (let i = 0; i < width * height; i++) {
            if (data[i * 4 + 3] > threshold) {
                visited[i] = -1;
            }
        }

        const stack: [number, number][] = [];

        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
                const i = idx(x, y);
                if (visited[i] === 0) {
                    // Found new region
                    labels++;
                    stack.push([x, y]);
                    visited[i] = labels;

                    while (stack.length > 0) {
                        const [cx, cy] = stack.pop()!;

                        // Check neighbors (4-connectivity)
                        const neighbors = [
                            [cx + 1, cy], [cx - 1, cy], [cx, cy + 1], [cx, cy - 1]
                        ];

                        for (const [nx, ny] of neighbors) {
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                const ni = idx(nx, ny);
                                if (visited[ni] === 0) {
                                    visited[ni] = labels;
                                    stack.push([nx, ny]);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Filter out very small regions (noise)
        const labelCounts = new Array(labels + 1).fill(0);
        for (let i = 0; i < visited.length; i++) {
            if (visited[i] > 0) labelCounts[visited[i]]++;
        }

        // Assume a valid slot is at least 5% of the image area? Or just > 1000 pixels?
        // Let's say > 1% of total pixels
        const minPixels = (width * height) * 0.01;
        let validRegions = 0;
        for (let i = 1; i <= labels; i++) {
            if (labelCounts[i] > minPixels) validRegions++;
        }

        return validRegions > 0 ? validRegions : 1; // Default to 1 if analysis fails or 0 found (e.g. full transparency treated as 1 big block)
    }

    const handleDelete = (id: string) => {
        const currentLayouts = settings.layout?.layouts || []
        updateSettings({
            layout: {
                ...settings.layout,
                layouts: currentLayouts.filter(l => l.id !== id),
                selectedLayoutId: settings.layout.selectedLayoutId === id ? null : settings.layout.selectedLayoutId
            }
        })
    }

    return (
        <div className="max-w-4xl">
            <h2 className="text-2xl font-semibold mb-6">Layout Settings</h2>

            {/* Upload Section */}
            <div className="mb-8 p-6 bg-dark-lighter rounded-xl border-2 border-dashed border-gray-700 hover:border-primary transition cursor-pointer"
                onClick={() => fileInputRef.current?.click()}>
                <input
                    type="file"
                    ref={fileInputRef}
                    className="hidden"
                    accept="image/png"
                    onChange={handleFileUpload}
                />
                <div className="flex flex-col items-center text-center">
                    <Upload className="w-12 h-12 text-gray-400 mb-4" />
                    <h3 className="text-lg font-medium mb-1">Upload New Layout</h3>
                    <p className="text-gray-500 mb-4">Click to browse PNG files with transparency</p>
                    <p className="text-sm text-gray-600">
                        System will automatically detect transparent photo slots.
                    </p>
                </div>
            </div>

            <canvas ref={canvasRef} className="hidden" />

            {/* Layout Grid */}
            <h3 className="text-lg font-medium mb-4 flex items-center gap-2">
                <LayoutIcon className="w-5 h-5 text-primary" />
                Available Layouts
            </h3>

            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                {settings.layout?.layouts.map((layout) => (
                    <div
                        key={layout.id}
                        className={`relative group bg-dark-lighter rounded-xl overflow-hidden border-2 transition ${settings.layout.selectedLayoutId === layout.id
                            ? 'border-primary'
                            : 'border-transparent hover:border-gray-600'
                            }`}
                        onClick={() => updateSettings({
                            layout: { ...settings.layout, selectedLayoutId: layout.id }
                        })}
                    >
                        {/* Preview */}
                        <div className="aspect-[3/2] bg-gray-800 relative">
                            {/* Show simplified representation or actual image */}
                            <img src={layout.path} alt={layout.name} className="w-full h-full object-contain" />
                            <div className="absolute top-2 right-2 bg-black/70 px-2 py-1 rounded text-xs text-white">
                                {layout.photoCount} Photos
                            </div>
                        </div>

                        {/* Info */}
                        <div className="p-4 flex items-center justify-between">
                            <span className="font-medium truncate">{layout.name}</span>
                            {settings.layout.selectedLayoutId === layout.id && (
                                <CheckCircle className="w-5 h-5 text-primary" />
                            )}
                        </div>

                        {/* Delete Action */}
                        <button
                            onClick={(e) => {
                                e.stopPropagation();
                                handleDelete(layout.id);
                            }}
                            className="absolute top-2 left-2 p-2 bg-red-500/80 hover:bg-red-500 text-white rounded-lg opacity-0 group-hover:opacity-100 transition"
                        >
                            <Trash2 className="w-4 h-4" />
                        </button>
                    </div>
                ))}
            </div>
        </div>
    )
}
