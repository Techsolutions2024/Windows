/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        primary: {
          DEFAULT: '#0ea5e9',
          50: '#f0f9ff',
          100: '#e0f2fe',
          200: '#bae6fd',
          300: '#7dd3fc',
          400: '#38bdf8',
          500: '#0ea5e9',
          600: '#0284c7',
          700: '#0369a1',
          800: '#075985',
          900: '#0c4a6e',
          dark: '#0284c7',
          light: '#38bdf8',
        },
        secondary: '#8b5cf6',
        accent: '#f59e0b',
        background: {
          light: '#f8fafc',
          dark: '#121212',
          surface: '#1e1e1e',
          'surface-light': '#2d2d2d',
        },
        border: {
          light: '#e2e8f0',
          dark: '#333333',
          'dark-light': '#404040',
        },
        dark: {
          DEFAULT: '#1F2937',
          lighter: '#374151',
          darker: '#111827',
        },
      },
      fontFamily: {
        display: ['Inter', 'Spline Sans', 'Plus Jakarta Sans', 'sans-serif'],
      },
      borderRadius: {
        DEFAULT: '0.5rem',
        lg: '0.75rem',
        xl: '1rem',
        '2xl': '1.5rem',
      },
      boxShadow: {
        'glow': '0 0 20px 2px rgba(14, 165, 233, 0.4)',
        'glow-lg': '0 0 30px 4px rgba(14, 165, 233, 0.5)',
      },
      animation: {
        'countdown': 'pulse 1s cubic-bezier(0.4, 0, 0.6, 1) infinite',
        'fly-down': 'flyDown 0.8s cubic-bezier(0.34, 1.56, 0.64, 1)',
        'pulse-glow': 'pulseGlow 2s cubic-bezier(0.4, 0, 0.6, 1) infinite',
      },
      keyframes: {
        flyDown: {
          '0%': { 
            transform: 'translateY(-100%) scale(0.5) rotate(-10deg)',
            opacity: '0',
            filter: 'blur(8px)',
          },
          '50%': {
            filter: 'blur(4px)',
          },
          '100%': { 
            transform: 'translateY(0) scale(1) rotate(0deg)',
            opacity: '1',
            filter: 'blur(0)',
          },
        },
        pulseGlow: {
          '0%, 100%': {
            boxShadow: '0 0 20px 2px rgba(14, 165, 233, 0.4)',
          },
          '50%': {
            boxShadow: '0 0 30px 4px rgba(14, 165, 233, 0.7)',
          },
        },
      },
    },
  },
  plugins: [],
}
