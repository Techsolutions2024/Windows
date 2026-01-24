import { useState, useEffect, useRef } from 'react';
import { useParams, useNavigate, useSearchParams } from 'react-router-dom';
import { motion, AnimatePresence } from 'framer-motion';
import { useAppStore } from '../store/useAppStore';
import LiveViewDisplay from '../components/LiveViewDisplay';
import CountdownOverlay from '../components/CountdownOverlay';
import PhotoStripTray from '../components/PhotoStripTray';
import api, { Event } from '../services/api';

interface Photo {
  id: number;
  imageUrl?: string;
  status: 'empty' | 'capturing' | 'filled';
}

export default function CapturePage() {
  const { eventId: paramEventId } = useParams();
  const [searchParams] = useSearchParams();
  const navigate = useNavigate();
  const { settings } = useAppStore();

  const eventId = paramEventId || searchParams.get('eventId');
  const modeParam = searchParams.get('mode');

  const [currentEvent, setCurrentEvent] = useState<Event | null>(null);
  const [captureMode, setCaptureMode] = useState<'photo' | 'gif' | 'boomerang' | 'video'>((modeParam as any) || settings.captureMode || 'photo');

  // Determine configuration based on settings
  const selectedLayout = settings.layout.layouts.find(l => l.id === settings.layout.selectedLayoutId);
  const totalPhotos = selectedLayout ? selectedLayout.photoCount : 1; // Default to 1 if no layout

  const [photos, setPhotos] = useState<Photo[]>([]);
  const [currentPhotoIndex, setCurrentPhotoIndex] = useState(0);
  const [countdownValue, setCountdownValue] = useState(0);
  const [isSequenceRunning, setIsSequenceRunning] = useState(false);
  const [statusMessage, setStatusMessage] = useState('');

  // Initialize photos based on layout
  useEffect(() => {
    const initialPhotos: Photo[] = Array.from({ length: totalPhotos }, (_, i) => ({
      id: i + 1,
      status: 'empty'
    }));
    setPhotos(initialPhotos);
    setCurrentPhotoIndex(0);
  }, [totalPhotos]);

  useEffect(() => {
    // Fetch event details
    if (eventId) {
      const fetchEvent = async () => {
        try {
          const id = parseInt(eventId);
          if (!isNaN(id)) {
            const response = await api.getEvent(id);
            if (response.success && response.data) {
              setCurrentEvent(response.data);
            }
          }
        } catch (error) {
          console.error("Failed to fetch event", error);
        }
      };
      fetchEvent();
    }

    // Start Live View
    const startLiveView = async () => {
      try {
        await api.startLiveView();
      } catch (error) {
        console.error("Failed to start live view", error);
      }
    };
    startLiveView();

    return () => {
      api.stopLiveView();
    };
  }, [eventId]);

  // Sequence Runner
  const startCaptureSequence = async () => {
    if (isSequenceRunning) return;
    setIsSequenceRunning(true);

    for (let i = 0; i < totalPhotos; i++) {
      setCurrentPhotoIndex(i);

      // 1. Interval delay (except first photo, or maybe even first if configured?)
      // Usually first photo has a "Get Ready" countdown.
      // Subsequent photos have "Next photo in..." countdown/interval.

      const countDuration = i === 0 ? settings.countdownDuration : settings.photoInterval;

      await runCountdown(countDuration);

      // 2. Capture
      await capturePhoto(i);

      // 3. Short preview delay
      if (i < totalPhotos - 1) {
        setStatusMessage("Great shot! Next one coming up...");
        await new Promise(r => setTimeout(r, 2000)); // 2s preview of result
        setStatusMessage("");
      }
    }

    setIsSequenceRunning(false);

    // Complete
    setStatusMessage("All done! Processing...");
    setTimeout(() => {
      navigate(`/print-result${eventId ? `?eventId=${eventId}` : ''}`);
    }, 1500);
  };

  const runCountdown = (seconds: number): Promise<void> => {
    return new Promise((resolve) => {
      setCountdownValue(seconds);
      if (seconds === 0) {
        resolve();
        return;
      }

      let remaining = seconds;
      const interval = setInterval(() => {
        remaining -= 1;
        setCountdownValue(remaining);
        if (remaining <= 0) {
          clearInterval(interval);
          resolve();
        }
      }, 1000);
    });
  };

  const capturePhoto = async (index: number) => {
    // Mark as capturing
    setPhotos(prev => prev.map((p, idx) => idx === index ? { ...p, status: 'capturing' } : p));

    try {
      const idToUse = eventId ? parseInt(eventId) : 0;
      console.log(`Capturing photo ${index + 1}/${totalPhotos}`);

      // Actual API call
      // const response = await api.capturePhoto(idToUse);

      // Simulation
      await new Promise(resolve => setTimeout(resolve, 800)); // Shutter lag simulation
      const mockImageUrl = `https://picsum.photos/400/300?random=${Date.now()}`;

      setPhotos(prev => prev.map((p, idx) => idx === index ? { ...p, status: 'filled', imageUrl: mockImageUrl } : p));

    } catch (error) {
      console.error("Capture failed", error);
      setPhotos(prev => prev.map((p, idx) => idx === index ? { ...p, status: 'empty' } : p));
    }
  };

  const handleBack = () => {
    navigate(`/mode-selection${eventId ? `?eventId=${eventId}` : ''}`);
  };

  const handleRetake = () => {
    // Reset
    const initialPhotos: Photo[] = Array.from({ length: totalPhotos }, (_, i) => ({
      id: i + 1,
      status: 'empty'
    }));
    setPhotos(initialPhotos);
    setCurrentPhotoIndex(0);
    setIsSequenceRunning(false);
  };

  return (
    <div className="w-full h-screen bg-black flex flex-col relative overflow-hidden">
      {/* Top Bar */}
      <div className="absolute top-0 left-0 right-0 z-20 bg-gradient-to-b from-black via-black/80 to-transparent px-10 py-6">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-6">
            <button
              onClick={handleBack}
              disabled={isSequenceRunning}
              className="flex items-center justify-center w-12 h-12 rounded-xl bg-white/10 hover:bg-white/20 backdrop-blur-sm transition-colors disabled:opacity-50"
            >
              <span className="material-symbols-outlined text-white">arrow_back</span>
            </button>
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-lg bg-primary flex items-center justify-center">
                <span className="material-symbols-outlined text-white text-2xl">
                  photo_camera
                </span>
              </div>
              <div>
                <h2 className="text-white text-xl font-bold leading-tight">
                  {currentEvent?.name || 'Quick Session'}
                </h2>
                <p className="text-white/60 text-sm font-medium">
                  {selectedLayout?.name || 'Default Layout'} • {totalPhotos} Photos
                </p>
              </div>
            </div>
          </div>

          {/* Right Info */}
          <div className="flex items-center gap-4">
            <div className="px-4 py-2 bg-white/10 backdrop-blur-sm rounded-lg text-white font-medium">
              {captureMode.toUpperCase()}
            </div>
            {settings.selectedCamera && (
              <div className="flex items-center gap-2 px-4 py-2 bg-white/10 backdrop-blur-sm rounded-lg text-white">
                <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse" />
                <span className="text-sm">{settings.selectedCamera}</span>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Center - Live View */}
      <div className="flex-1 relative flex items-center justify-center bg-gray-900">
        <LiveViewDisplay filter="none" mirror={currentEvent?.config?.mirrorCamera} />

        <AnimatePresence>
          {countdownValue > 0 && <CountdownOverlay count={countdownValue} key="countdown" />}

          {statusMessage && (
            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0 }}
              className="absolute top-1/4 left-1/2 -translate-x-1/2 bg-black/60 backdrop-blur px-6 py-3 rounded-full text-white font-xl font-medium z-30"
            >
              {statusMessage}
            </motion.div>
          )}
        </AnimatePresence>

        {/* Start Button */}
        {!isSequenceRunning && currentPhotoIndex === 0 && (
          <div className="absolute bottom-40 left-1/2 transform -translate-x-1/2 z-10">
            <motion.button
              whileHover={{ scale: 1.05 }}
              whileTap={{ scale: 0.95 }}
              onClick={startCaptureSequence}
              className="relative w-24 h-24 rounded-full flex items-center justify-center bg-primary hover:bg-primary-600 shadow-2xl shadow-primary/50 transition-all"
            >
              <div className="w-20 h-20 rounded-full border-4 border-white flex items-center justify-center">
                <span className="material-symbols-outlined text-white text-4xl">
                  photo_camera
                </span>
              </div>
            </motion.button>
            <p className="absolute -bottom-10 left-1/2 -translate-x-1/2 text-white/80 font-medium whitespace-nowrap">
              Tap to Start
            </p>
          </div>
        )}

        {/* Retake Button (only if sequence finished but not navigated yet - though we auto navigate)
            Or if we want to allow stopping? For now let's keep it simple.
         */}
      </div>

      {/* Bottom - Photo Strip Tray */}
      <PhotoStripTray photos={photos} totalSlots={totalPhotos} />

    </div>
  );
}
