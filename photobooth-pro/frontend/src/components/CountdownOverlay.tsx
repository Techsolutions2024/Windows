import { motion } from 'framer-motion'

interface CountdownOverlayProps {
  count: number
}

export default function CountdownOverlay({ count }: CountdownOverlayProps) {
  return (
    <div className="absolute inset-0 flex items-center justify-center bg-black/50 backdrop-blur-sm z-50">
      <motion.div
        key={count}
        initial={{ scale: 0.5, opacity: 0 }}
        animate={{ scale: 1, opacity: 1 }}
        exit={{ scale: 1.5, opacity: 0 }}
        transition={{ duration: 0.5 }}
        className="text-white"
      >
        <div className="text-[200px] font-bold leading-none text-primary animate-countdown">
          {count}
        </div>
      </motion.div>

      {/* Flash effect */}
      {count === 1 && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: [0, 1, 0] }}
          transition={{ duration: 0.3, times: [0, 0.5, 1] }}
          className="absolute inset-0 bg-white"
        />
      )}
    </div>
  )
}
