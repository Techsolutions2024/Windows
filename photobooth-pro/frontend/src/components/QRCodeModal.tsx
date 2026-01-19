import { motion, AnimatePresence } from 'framer-motion';
import { QRCodeSVG } from 'qrcode.react';

interface QRCodeModalProps {
  isOpen: boolean;
  onClose: () => void;
  photoUrl: string;
  shortUrl?: string;
  autoCloseSeconds?: number;
}

export default function QRCodeModal({
  isOpen,
  onClose,
  photoUrl,
  shortUrl = 'pix.io/ABCD-123',
  autoCloseSeconds = 45,
}: QRCodeModalProps) {
  return (
    <AnimatePresence>
      {isOpen && (
        <>
          {/* Backdrop */}
          <motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            onClick={onClose}
            className="fixed inset-0 z-50 backdrop-blur-xl bg-black/85"
          />

          {/* Modal */}
          <div className="fixed inset-0 z-50 flex items-center justify-center p-4">
            <motion.div
              initial={{ opacity: 0, scale: 0.9, y: 20 }}
              animate={{ opacity: 1, scale: 1, y: 0 }}
              exit={{ opacity: 0, scale: 0.9, y: 20 }}
              className="bg-white dark:bg-gray-900 w-full max-w-[640px] rounded-xl shadow-2xl flex flex-col items-center p-12 text-center relative"
            >
              {/* Close Button */}
              <button
                onClick={onClose}
                className="absolute top-6 right-6 text-slate-400 hover:text-slate-600 dark:text-slate-500 dark:hover:text-slate-300 transition-colors"
              >
                <span className="material-symbols-outlined text-3xl">close</span>
              </button>

              {/* Header */}
              <div className="mb-2">
                <h2 className="text-slate-900 dark:text-white text-3xl font-bold leading-tight tracking-tight">
                  Your Photo is Ready!
                </h2>
              </div>

              {/* Subtitle */}
              <div className="mb-10 px-8">
                <p className="text-slate-600 dark:text-slate-400 text-xl font-medium">
                  Scan to download your photo directly to your phone.
                </p>
              </div>

              {/* QR Code */}
              <div className="relative group p-6 bg-slate-50 dark:bg-slate-800 rounded-xl border-2 border-slate-100 dark:border-slate-700 mb-8">
                <div className="w-[320px] h-[320px] bg-white dark:bg-white flex items-center justify-center p-4 rounded-lg shadow-sm">
                  <QRCodeSVG
                    value={photoUrl}
                    size={288}
                    level="H"
                    includeMargin={false}
                    className="w-full h-full"
                  />
                </div>
              </div>

              {/* Fallback URL */}
              <div className="mb-10">
                <p className="text-slate-500 dark:text-slate-400 text-base font-normal mb-1">
                  Having trouble scanning?
                </p>
                <p className="text-primary text-lg font-semibold underline cursor-pointer hover:text-primary-600">
                  {shortUrl}
                </p>
              </div>

              {/* Done Button */}
              <button
                onClick={onClose}
                className="w-full h-16 bg-primary hover:bg-primary-600 text-white rounded-xl flex items-center justify-center gap-3 transition-colors"
              >
                <span className="text-xl font-bold uppercase tracking-wider">Done</span>
                <span className="material-symbols-outlined text-2xl">check_circle</span>
              </button>
            </motion.div>
          </div>

          {/* Station Info */}
          <div className="fixed bottom-10 left-1/2 -translate-x-1/2 z-50 flex gap-6 opacity-30 pointer-events-none">
            <div className="flex items-center gap-2 text-white">
              <span className="material-symbols-outlined">wifi</span>
              <span className="text-sm font-medium">Station: BOOTH-01</span>
            </div>
            <div className="flex items-center gap-2 text-white">
              <span className="material-symbols-outlined">update</span>
              <span className="text-sm font-medium">Auto-reset in {autoCloseSeconds}s</span>
            </div>
          </div>
        </>
      )}
    </AnimatePresence>
  );
}
