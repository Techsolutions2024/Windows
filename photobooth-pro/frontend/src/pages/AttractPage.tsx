
import React from 'react';
import { useNavigate, useSearchParams } from 'react-router-dom';
import { motion } from 'framer-motion';

export default function AttractPage() {
    const navigate = useNavigate();
    const [searchParams] = useSearchParams();
    const eventId = searchParams.get('eventId');

    const handleStart = () => {
        // Navigate to mode selection, passing the eventId if present
        navigate(`/mode-selection${eventId ? `?eventId=${eventId}` : ''}`);
    };

    return (
        <div
            onClick={handleStart}
            className="relative w-full h-screen bg-black overflow-hidden cursor-pointer"
        >
            {/* Background - In a real app, this could be a video loop */}
            <div
                className="absolute inset-0 bg-cover bg-center opacity-60 transition-transform duration-[20s] hover:scale-110"
                style={{
                    backgroundImage: 'url("https://images.unsplash.com/photo-1516035069371-29a1b244cc32?ixlib=rb-4.0.3&auto=format&fit=crop&w=1920&q=80")', // Example vibrant party image
                    animation: 'zoomIn 30s infinite alternate'
                }}
            />

            {/* Gradient Overlay for sophisticated look */}
            <div className="absolute inset-0 bg-gradient-to-t from-black/80 via-transparent to-black/40" />

            {/* Content */}
            <div className="absolute inset-0 flex flex-col items-center justify-center z-10 text-white p-8">

                {/* Animated Logo/Title Area */}
                <motion.div
                    initial={{ opacity: 0, y: -50 }}
                    animate={{ opacity: 1, y: 0 }}
                    transition={{ duration: 1 }}
                    className="mb-12 text-center"
                >
                    <div className="w-32 h-32 mx-auto mb-6 bg-white/10 backdrop-blur-md rounded-full border border-white/20 flex items-center justify-center shadow-[0_0_50px_rgba(255,255,255,0.2)]">
                        <span className="material-symbols-outlined text-6xl">photo_camera</span>
                    </div>
                    <h1 className="text-5xl md:text-7xl font-bold tracking-tight bg-clip-text text-transparent bg-gradient-to-r from-white via-primary-200 to-white/80 drop-shadow-lg">
                        PHOTOBOOTH
                    </h1>
                    <p className="text-xl md:text-2xl mt-4 font-light tracking-[0.3em] opacity-80">
                        CAPTURE THE MOMENT
                    </p>
                </motion.div>

                {/* CTA "Touch to Start" */}
                <motion.div
                    animate={{
                        scale: [1, 1.1, 1],
                        opacity: [0.7, 1, 0.7]
                    }}
                    transition={{
                        duration: 2,
                        repeat: Infinity,
                        ease: "easeInOut"
                    }}
                    className="mt-16 text-center"
                >
                    <div className="border border-white/30 bg-white/5 backdrop-blur-sm px-12 py-6 rounded-full">
                        <h2 className="text-3xl md:text-4xl font-black tracking-wider uppercase">
                            Chạm để bắt đầu
                        </h2>
                    </div>
                    <p className="mt-4 text-sm opacity-50 uppercase tracking-widest">Touch screen to start</p>
                </motion.div>

            </div>

            {/* CSS Animation for Background */}
            <style>{`
        @keyframes zoomIn {
          0% { transform: scale(1); }
          100% { transform: scale(1.15); }
        }
      `}</style>
        </div>
    );
}
