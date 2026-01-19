import { useState, useEffect } from 'react';
import { useParams, useNavigate, useSearchParams } from 'react-router-dom';
import { motion } from 'framer-motion';
// import { useAppStore } from '../store/useAppStore';
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
  // const { settings, updateSettings, countdown, isCapturing } = useAppStore();

  const eventId = paramEventId || searchParams.get('eventId');
  const modeParam = searchParams.get('mode');

  const [currentEvent, setCurrentEvent] = useState<Event | null>(null);
  const [captureMode, setCaptureMode] = useState<'photo' | 'gif' | 'boomerang' | 'video'>((modeParam as any) || 'photo');
  const [photos, setPhotos] = useState<Photo[]>([
    { id: 1, status: 'empty' },
    { id: 2, status: 'empty' },
    { id: 3, status: 'empty' },
    { id: 4, status: 'empty' },
  ]);
  const [currentPhotoIndex, setCurrentPhotoIndex] = useState(0);
  const [countdownValue, setCountdownValue] = useState(0);

  useEffect(() => {
    // Fetch event details if eventId is present
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

    // Start live view when component mounts
    const startLiveView = async () => {
      try {
        await api.startLiveView();
      } catch (error) {
        console.error("Failed to start live view", error);
      }
    };
    startLiveView();

    // Cleanup: Stop live view when unmounting
    return () => {
      api.stopLiveView();
    };
  }, [eventId]);

  const handleCapture = async () => {
    if (currentPhotoIndex >= 4) return;

    // Start countdown
    setCountdownValue(3);
    const countdownInterval = setInterval(() => {
      setCountdownValue((prev) => {
        if (prev <= 1) {
          clearInterval(countdownInterval);
          capturePhoto();
          return 0;
        }
        return prev - 1;
      });
    }, 1000);
  };

  const capturePhoto = async () => {
    // Mark current photo as capturing
    setPhotos((prev) =>
      prev.map((photo) =>
        photo.id === currentPhotoIndex + 1 ? { ...photo, status: 'capturing' } : photo
      )
    );

    try {
      // Call backend to capture
      // If eventId is available, pass it. If not, maybe pass 0 or handle error?
      // The API requires eventId.
      const idToUse = eventId ? parseInt(eventId) : 0;
      console.log("Capturing photo for event:", idToUse);

      // Simulating API call delay and result for now as LiveView might not be sending images back in this context yet
      // In a real scenario: const response = await api.capturePhoto(idToUse);

      await new Promise(resolve => setTimeout(resolve, 1000));
      const mockImageUrl = `https://picsum.photos/400/300?random=${currentPhotoIndex + 1}`;

      setPhotos((prev) =>
        prev.map((photo) =>
          photo.id === currentPhotoIndex + 1
            ? { ...photo, imageUrl: mockImageUrl, status: 'filled' }
            : photo
        )
      );

      // Move to next photo
      if (currentPhotoIndex < 3) {
        setCurrentPhotoIndex(currentPhotoIndex + 1);
      } else {
        // All photos captured, navigate to result
        setTimeout(() => {
          navigate(`/print-result${eventId ? `?eventId=${eventId}` : ''}`);
        }, 1500);
      }

    } catch (error) {
      console.error("Capture failed", error);
      setPhotos((prev) =>
        prev.map((photo) =>
          photo.id === currentPhotoIndex + 1 ? { ...photo, status: 'empty' } : photo
        )
      );
    }
  };

  const handleBack = () => {
    navigate(`/mode-selection${eventId ? `?eventId=${eventId}` : ''}`);
  };

  const handleRetake = () => {
    if (currentPhotoIndex > 0) {
      const prevIndex = currentPhotoIndex - 1;
      setPhotos((prev) =>
        prev.map((photo) =>
          photo.id === prevIndex + 1 ? { id: photo.id, status: 'empty' } : photo
        )
      );
      setCurrentPhotoIndex(prevIndex);
    }
  };

  return (
    <div className="w-full h-screen bg-black flex flex-col relative overflow-hidden">
      {/* Top Bar */}
      <div className="absolute top-0 left-0 right-0 z-20 bg-gradient-to-b from-black via-black/80 to-transparent px-10 py-6">
        <div className="flex items-center justify-between">
          {/* Left - Back & Event Info */}
          <div className="flex items-center gap-6">
            <button
              onClick={handleBack}
              className="flex items-center justify-center w-12 h-12 rounded-xl bg-white/10 hover:bg-white/20 backdrop-blur-sm transition-colors"
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
                  {currentPhotoIndex + 1} of 4 photos
                </p>
              </div>
            </div>
          </div>

          {/* Right - Camera Status & Mode */}
          <div className="flex items-center gap-4">
            {/* Capture Mode Selector */}
            <div className="flex gap-2 bg-white/10 backdrop-blur-sm rounded-xl p-1">
              <button
                onClick={() => setCaptureMode('photo')}
                className={`px-6 py-2 rounded-lg text-sm font-bold uppercase tracking-wider transition-all ${captureMode === 'photo'
                  ? 'bg-primary text-white shadow-lg'
                  : 'text-white/70 hover:text-white'
                  }`}
              >
                Photo
              </button>
              <button
                onClick={() => setCaptureMode('gif')}
                className={`px-6 py-2 rounded-lg text-sm font-bold uppercase tracking-wider transition-all ${captureMode === 'gif'
                  ? 'bg-primary text-white shadow-lg'
                  : 'text-white/70 hover:text-white'
                  }`}
              >
                GIF
              </button>
              <button
                onClick={() => setCaptureMode('boomerang')}
                className={`px-6 py-2 rounded-lg text-sm font-bold uppercase tracking-wider transition-all ${captureMode === 'boomerang'
                  ? 'bg-primary text-white shadow-lg'
                  : 'text-white/70 hover:text-white'
                  }`}
              >
                Boomerang
              </button>
              <button
                onClick={() => setCaptureMode('video')}
                className={`px-6 py-2 rounded-lg text-sm font-bold uppercase tracking-wider transition-all ${captureMode === 'video'
                  ? 'bg-primary text-white shadow-lg'
                  : 'text-white/70 hover:text-white'
                  }`}
              >
                Video
              </button>
            </div>

            {/* Camera Status */}
            <div className="flex items-center gap-3 px-4 py-2 bg-white/10 backdrop-blur-sm rounded-xl">
              <div className="w-2 h-2 bg-green-400 rounded-full animate-pulse"></div>
              <span className="text-white text-sm font-medium">Canon R100</span>
            </div>

            {/* Settings */}
            <button className="flex items-center justify-center w-12 h-12 rounded-xl bg-white/10 hover:bg-white/20 backdrop-blur-sm transition-colors">
              <span className="material-symbols-outlined text-white">settings</span>
            </button>
          </div>
        </div>
      </div>

      {/* Center - Live View */}
      <div className="flex-1 relative flex items-center justify-center">
        <LiveViewDisplay filter="none" />

        {/* Countdown Overlay */}
        {countdownValue > 0 && <CountdownOverlay count={countdownValue} />}

        {/* Center Capture Button */}
        <div className="absolute bottom-40 left-1/2 transform -translate-x-1/2 z-10">
          <motion.button
            whileHover={{ scale: 1.05 }}
            whileTap={{ scale: 0.95 }}
            onClick={handleCapture}
            disabled={currentPhotoIndex >= 4 || countdownValue > 0}
            className={`relative w-24 h-24 rounded-full flex items-center justify-center transition-all ${currentPhotoIndex >= 4 || countdownValue > 0
              ? 'bg-gray-600 cursor-not-allowed'
              : 'bg-primary hover:bg-primary-600 shadow-2xl shadow-primary/50'
              }`}
          >
            <div className="w-20 h-20 rounded-full border-4 border-white flex items-center justify-center">
              <span className="material-symbols-outlined text-white text-4xl">
                photo_camera
              </span>
            </div>
          </motion.button>
        </div>

        {/* Retake Button */}
        {currentPhotoIndex > 0 && currentPhotoIndex < 4 && (
          <motion.button
            initial={{ opacity: 0, x: 20 }}
            animate={{ opacity: 1, x: 0 }}
            whileHover={{ scale: 1.05 }}
            whileTap={{ scale: 0.95 }}
            onClick={handleRetake}
            className="absolute bottom-40 right-20 z-10 flex items-center gap-2 px-6 py-3 rounded-xl bg-white/10 hover:bg-white/20 backdrop-blur-sm transition-colors"
          >
            <span className="material-symbols-outlined text-white">undo</span>
            <span className="text-white font-bold">Retake</span>
          </motion.button>
        )}
      </div>

      {/* Bottom - Photo Strip Tray */}
      <PhotoStripTray photos={photos} totalSlots={4} />

      {/* Kiosk ID */}
      <div className="absolute bottom-6 left-6 text-white/30 text-xs font-medium uppercase tracking-[0.2em] pointer-events-none z-30">
        Kiosk ID: BOOTH-01 // Session #{eventId || 'QUICK'}
      </div>
    </div>
  );
}
