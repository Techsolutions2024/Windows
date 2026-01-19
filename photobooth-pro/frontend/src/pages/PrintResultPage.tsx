import { useState } from 'react';
import { motion } from 'framer-motion';
import { useNavigate } from 'react-router-dom';
import QRCodeModal from '../components/QRCodeModal';

interface Photo {
  id: number;
  url: string;
}

export default function PrintResultPage() {
  const navigate = useNavigate();
  const [showQRModal, setShowQRModal] = useState(false);

  // Mock photos - replace with actual data
  const photos: Photo[] = [
    { id: 1, url: 'https://images.unsplash.com/photo-1529626455594-4ff0802cfb7e?w=400' },
    { id: 2, url: 'https://images.unsplash.com/photo-1511632765486-a01980e01a18?w=400' },
    { id: 3, url: 'https://images.unsplash.com/photo-1506794778202-cad84cf45f1d?w=400' },
    { id: 4, url: 'https://images.unsplash.com/photo-1507003211169-0a1dd7228f2d?w=400' },
  ];

  const collageUrl = 'https://photobooth.example.com/collage/abc123';

  return (
    <div className="min-h-screen bg-background-dark text-white flex flex-col">
      {/* Header */}
      <header className="flex items-center justify-between border-b border-white/10 px-10 py-5 bg-background-dark/50 backdrop-blur-md z-10">
        <div className="flex items-center gap-4">
          <div className="w-8 h-8 flex items-center justify-center bg-primary rounded-lg">
            <span className="material-symbols-outlined text-white text-2xl">auto_awesome</span>
          </div>
          <div>
            <h2 className="text-xl font-bold leading-tight tracking-tight">Event PhotoBooth</h2>
            <p className="text-slate-400 text-xs uppercase tracking-widest font-medium">
              Grand Gala 2024
            </p>
          </div>
        </div>
        <div className="flex gap-3">
          <button className="flex items-center justify-center rounded-lg h-12 w-12 bg-white/5 hover:bg-white/10 transition-colors">
            <span className="material-symbols-outlined">settings</span>
          </button>
          <button className="flex items-center justify-center rounded-lg h-12 w-12 bg-white/5 hover:bg-white/10 transition-colors">
            <span className="material-symbols-outlined">help</span>
          </button>
        </div>
      </header>

      {/* Main Content */}
      <main className="flex-1 flex overflow-hidden relative bg-gradient-to-br from-gray-900 to-black">
        {/* Left Side - Collage Preview */}
        <div className="flex-1 flex flex-col items-center justify-center p-8 gap-8">
          {/* 2x2 Photo Collage */}
          <motion.div
            initial={{ opacity: 0, rotate: -2, scale: 0.9 }}
            animate={{ opacity: 1, rotate: 1, scale: 1 }}
            transition={{ duration: 0.5 }}
            className="bg-white p-6 shadow-2xl rotate-1 transform-gpu"
          >
            <div className="grid grid-cols-2 gap-4 w-[600px]">
              {photos.map((photo, index) => (
                <motion.div
                  key={photo.id}
                  initial={{ opacity: 0, scale: 0.8 }}
                  animate={{ opacity: 1, scale: 1 }}
                  transition={{ delay: index * 0.1, duration: 0.3 }}
                  className="aspect-square bg-gray-200 overflow-hidden"
                >
                  <div
                    className="w-full h-full bg-center bg-cover"
                    style={{ backgroundImage: `url(${photo.url})` }}
                  />
                </motion.div>
              ))}
            </div>

            {/* Collage Footer */}
            <div className="mt-8 flex justify-between items-end">
              <div>
                <p className="text-gray-900 font-bold text-lg leading-tight">THE GRAND GALA</p>
                <p className="text-gray-500 text-sm font-medium italic">
                  {new Date().toLocaleDateString('en-US', {
                    month: 'long',
                    day: 'numeric',
                    year: 'numeric',
                  })}
                </p>
              </div>
              <div className="text-gray-400">
                <span className="material-symbols-outlined text-4xl">photo_library</span>
              </div>
            </div>
          </motion.div>

          {/* Action Buttons */}
          <div className="flex justify-center w-full max-w-[600px] gap-6 px-4">
            <motion.button
              whileHover={{ scale: 1.02 }}
              whileTap={{ scale: 0.98 }}
              onClick={() => navigate('/capture')}
              className="flex items-center justify-center gap-2 rounded-xl h-14 px-8 bg-white/10 hover:bg-white/20 border border-white/10 text-lg font-bold transition-all flex-1"
            >
              <span className="material-symbols-outlined">refresh</span>
              <span>Retake All</span>
            </motion.button>

            <motion.button
              whileHover={{ scale: 1.02 }}
              whileTap={{ scale: 0.98 }}
              onClick={() => navigate('/mode-selection')}
              className="flex items-center justify-center gap-2 rounded-xl h-14 px-8 bg-white/5 hover:bg-white/10 text-white/70 text-lg font-bold transition-all flex-1"
            >
              <span className="material-symbols-outlined">check_circle</span>
              <span>Done</span>
            </motion.button>
          </div>
        </div>

        {/* Right Sidebar - Share Options */}
        <aside className="w-[450px] bg-background-dark/80 backdrop-blur-xl border-l border-white/10 p-10 flex flex-col justify-between">
          <div className="flex flex-col gap-10">
            <div className="flex flex-col gap-2">
              <h1 className="text-3xl font-bold leading-tight">Share Your Memory</h1>
              <p className="text-slate-400 text-lg font-normal">
                Choose how you want to keep this moment forever.
              </p>
            </div>

            <div className="flex flex-col gap-4">
              {/* Print Button */}
              <motion.button
                whileHover={{ scale: 1.02 }}
                whileTap={{ scale: 0.98 }}
                className="group flex items-center gap-6 p-6 rounded-2xl bg-primary hover:bg-primary-600 transition-all shadow-lg shadow-primary/20"
              >
                <div className="w-16 h-16 rounded-xl bg-white/20 flex items-center justify-center">
                  <span className="material-symbols-outlined text-4xl text-white">print</span>
                </div>
                <div className="text-left">
                  <p className="text-white text-xl font-bold leading-none mb-1">Print Photo</p>
                  <p className="text-white/70 text-sm font-medium">Instant 2x2 physical strip</p>
                </div>
                <div className="ml-auto opacity-0 group-hover:opacity-100 transition-opacity">
                  <span className="material-symbols-outlined">chevron_right</span>
                </div>
              </motion.button>

              {/* Email Button */}
              <motion.button
                whileHover={{ scale: 1.02 }}
                whileTap={{ scale: 0.98 }}
                className="group flex items-center gap-6 p-6 rounded-2xl bg-white/5 hover:bg-white/10 border border-white/10 transition-all"
              >
                <div className="w-16 h-16 rounded-xl bg-white/10 flex items-center justify-center">
                  <span className="material-symbols-outlined text-4xl text-white">mail</span>
                </div>
                <div className="text-left">
                  <p className="text-white text-xl font-bold leading-none mb-1">Send to Email</p>
                  <p className="text-slate-400 text-sm font-medium">
                    Digital copy in full resolution
                  </p>
                </div>
                <div className="ml-auto opacity-0 group-hover:opacity-100 transition-opacity">
                  <span className="material-symbols-outlined">chevron_right</span>
                </div>
              </motion.button>

              {/* QR Code Button */}
              <motion.button
                whileHover={{ scale: 1.02 }}
                whileTap={{ scale: 0.98 }}
                onClick={() => setShowQRModal(true)}
                className="group flex items-center gap-6 p-6 rounded-2xl bg-white/5 hover:bg-white/10 border border-white/10 transition-all"
              >
                <div className="w-16 h-16 rounded-xl bg-white/10 flex items-center justify-center">
                  <span className="material-symbols-outlined text-4xl text-white">qr_code_2</span>
                </div>
                <div className="text-left">
                  <p className="text-white text-xl font-bold leading-none mb-1">Scan QR Code</p>
                  <p className="text-slate-400 text-sm font-medium">Download directly to phone</p>
                </div>
                <div className="ml-auto opacity-0 group-hover:opacity-100 transition-opacity">
                  <span className="material-symbols-outlined">chevron_right</span>
                </div>
              </motion.button>
            </div>
          </div>

          {/* Info Box */}
          <div className="p-6 rounded-2xl border border-primary/30 bg-primary/10 backdrop-blur-sm">
            <div className="flex items-center gap-4">
              <div className="w-10 h-10 rounded-full bg-primary flex items-center justify-center animate-pulse">
                <span className="material-symbols-outlined text-white text-xl">info</span>
              </div>
              <div>
                <p className="text-white text-sm font-bold">Ready to collect?</p>
                <p className="text-slate-400 text-xs">Collages are printed in under 30 seconds.</p>
              </div>
            </div>
          </div>
        </aside>
      </main>

      {/* Kiosk ID */}
      <div className="absolute bottom-6 left-6 text-white/30 text-xs font-medium uppercase tracking-[0.2em] pointer-events-none">
        Kiosk ID: BOOTH-04 // Session #8429
      </div>

      {/* QR Code Modal */}
      <QRCodeModal
        isOpen={showQRModal}
        onClose={() => setShowQRModal(false)}
        photoUrl={collageUrl}
      />
    </div>
  );
}
