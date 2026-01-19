import { motion } from 'framer-motion';
import { useNavigate } from 'react-router-dom';

interface ModeButtonProps {
  icon: string;
  label: string;
  onClick: () => void;
}

const ModeButton = ({ icon, label, onClick }: ModeButtonProps) => (
  <motion.div
    className="flex flex-col items-center space-y-4 group"
    whileHover={{ scale: 1.05 }}
    whileTap={{ scale: 0.92 }}
  >
    <button
      onClick={onClick}
      className="w-24 h-24 md:w-32 md:h-32 rounded-full bg-white dark:bg-white text-slate-800 flex items-center justify-center shadow-xl hover:shadow-2xl transition-all"
    >
      <span className="material-symbols-outlined text-4xl md:text-5xl">{icon}</span>
    </button>
    <span className="text-slate-700 dark:text-white text-lg md:text-xl font-bold tracking-wide uppercase group-hover:text-primary transition-colors">
      {label}
    </span>
  </motion.div>
);

export default function ModeSelectionPage() {
  const navigate = useNavigate();

  const modes = [
    { icon: 'photo_camera', label: 'Print', path: '/capture?mode=photo' },
    { icon: 'collections', label: 'GIF', path: '/capture?mode=gif' },
    { icon: 'all_inclusive', label: 'Boomerang', path: '/capture?mode=boomerang' },
    { icon: 'videocam', label: 'Video', path: '/capture?mode=video' },
  ];

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-100 to-slate-200 dark:from-gray-900 dark:to-black text-slate-900 dark:text-white flex flex-col justify-between p-8 md:p-12 overflow-hidden transition-colors duration-300">
      {/* Header */}
      <header className="flex justify-between items-start w-full z-10">
        <div className="w-24 h-24 md:w-32 md:h-32 bg-white/10 dark:bg-white/5 rounded-xl border border-slate-200 dark:border-white/10 flex items-center justify-center backdrop-blur-sm">
          <span className="text-slate-400 dark:text-white/20 text-xs font-medium uppercase tracking-widest text-center px-2">
            Your Logo
          </span>
        </div>
        <div className="flex items-center space-x-4">
          <motion.button
            whileHover={{ scale: 1.1 }}
            whileTap={{ scale: 0.9 }}
            className="p-3 bg-red-500 hover:bg-red-600 text-white rounded-full shadow-lg shadow-red-500/30 transition-all flex items-center justify-center"
          >
            <span className="material-symbols-outlined text-2xl">qr_code_2</span>
          </motion.button>
          <motion.button
            whileHover={{ scale: 1.1 }}
            whileTap={{ scale: 0.9 }}
            onClick={() => navigate('/settings')}
            className="p-3 bg-primary hover:bg-primary-600 text-white rounded-full shadow-lg shadow-primary/30 transition-all flex items-center justify-center"
          >
            <span className="material-symbols-outlined text-2xl">settings</span>
          </motion.button>
        </div>
      </header>

      {/* Main Content */}
      <main className="flex-grow flex flex-col items-center justify-center z-10">
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5 }}
          className="grid grid-cols-2 md:grid-cols-4 gap-8 md:gap-16 items-center"
        >
          {modes.map((mode, index) => (
            <motion.div
              key={mode.label}
              initial={{ opacity: 0, scale: 0.8 }}
              animate={{ opacity: 1, scale: 1 }}
              transition={{ delay: index * 0.1, duration: 0.3 }}
            >
              <ModeButton
                icon={mode.icon}
                label={mode.label}
                onClick={() => navigate(mode.path)}
              />
            </motion.div>
          ))}
        </motion.div>
      </main>

      {/* Footer */}
      <footer className="w-full text-center z-10">
        <motion.p
          animate={{ opacity: [0.5, 1, 0.5] }}
          transition={{ duration: 2, repeat: Infinity }}
          className="text-slate-400 dark:text-slate-500 font-medium tracking-widest uppercase text-sm"
        >
          Touch a mode to start the fun
        </motion.p>
      </footer>

      {/* Theme Toggle */}
      <div className="fixed bottom-4 right-4 z-50">
        <motion.button
          whileHover={{ scale: 1.1 }}
          whileTap={{ scale: 0.9 }}
          onClick={() => document.documentElement.classList.toggle('dark')}
          className="p-3 rounded-full bg-white dark:bg-slate-800 shadow-md text-slate-600 dark:text-slate-300 hover:text-primary"
        >
          <span className="material-symbols-outlined block dark:hidden">dark_mode</span>
          <span className="material-symbols-outlined hidden dark:block">light_mode</span>
        </motion.button>
      </div>
    </div>
  );
}
