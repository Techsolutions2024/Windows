import { motion, AnimatePresence } from 'framer-motion';

interface PhotoSlot {
  id: number;
  imageUrl?: string;
  status: 'empty' | 'capturing' | 'filled';
}

interface PhotoStripTrayProps {
  photos: PhotoSlot[];
  totalSlots?: number;
}

export default function PhotoStripTray({ photos, totalSlots = 4 }: PhotoStripTrayProps) {
  const slots = Array.from({ length: totalSlots }, (_, i) => {
    const photo = photos.find(p => p.id === i + 1);
    return photo || { id: i + 1, status: 'empty' as const };
  });

  return (
    <div className="absolute bottom-0 w-full bg-gradient-to-t from-black via-black/80 to-transparent pb-10 pt-20 px-40">
      <div className="flex justify-center items-end gap-6 h-40">
        {slots.map((slot, index) => (
          <motion.div
            key={slot.id}
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ delay: index * 0.1 }}
            className="flex flex-col items-center gap-3"
          >
            <div
              className={`relative w-48 aspect-video rounded-lg overflow-hidden ${
                slot.status === 'capturing'
                  ? 'bg-primary/20 border-2 border-primary animate-pulse-glow'
                  : slot.status === 'filled'
                  ? 'bg-gray-800 border-2 border-gray-600'
                  : 'bg-[#232f48] border border-white/10'
              }`}
            >
              <AnimatePresence>
                {slot.imageUrl && (
                  <motion.div
                    initial={{ opacity: 0, scale: 0.8 }}
                    animate={{ opacity: 1, scale: 1 }}
                    exit={{ opacity: 0, scale: 0.8 }}
                    className="absolute inset-0 bg-cover bg-center"
                    style={{ backgroundImage: `url(${slot.imageUrl})` }}
                  />
                )}
              </AnimatePresence>

              {slot.status === 'empty' && (
                <div className="absolute inset-0 flex items-center justify-center">
                  <span className="material-symbols-outlined text-white/20 text-4xl">
                    add_a_photo
                  </span>
                </div>
              )}

              {slot.status === 'capturing' && (
                <div className="absolute inset-0 flex items-center justify-center">
                  <div className="bg-primary px-3 py-1 rounded text-[10px] font-bold uppercase tracking-tighter">
                    Saving...
                  </div>
                </div>
              )}
            </div>

            <span
              className={`text-xs font-bold uppercase tracking-widest ${
                slot.status === 'capturing'
                  ? 'text-primary'
                  : slot.status === 'filled'
                  ? 'text-white'
                  : 'text-white/40'
              }`}
            >
              Photo {slot.id}
            </span>
          </motion.div>
        ))}
      </div>
    </div>
  );
}
