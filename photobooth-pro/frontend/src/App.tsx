import { BrowserRouter as Router, Routes, Route } from 'react-router-dom'
import EventsPage from './pages/EventsPage'
import AttractPage from './pages/AttractPage'
import CapturePage from './pages/CapturePage'
import SharingPage from './pages/SharingPage'
import SettingsPage from './pages/SettingsPage'
import ModeSelectionPage from './pages/ModeSelectionPage'
import PrintResultPage from './pages/PrintResultPage'
import TemplateLibraryPage from './pages/TemplateLibraryPage'

function App() {
  return (
    <Router>
      <div className="w-full h-screen bg-dark-darker text-white">
        <Routes>
          <Route path="/" element={<EventsPage />} />
          <Route path="/attract" element={<AttractPage />} />
          <Route path="/templates" element={<TemplateLibraryPage />} />
          <Route path="/mode-selection" element={<ModeSelectionPage />} />
          <Route path="/capture" element={<CapturePage />} />
          <Route path="/capture/:eventId" element={<CapturePage />} />
          <Route path="/print-result" element={<PrintResultPage />} />
          <Route path="/sharing/:eventId" element={<SharingPage />} />
          <Route path="/settings" element={<SettingsPage />} />
        </Routes>
      </div>
    </Router>
  )
}

export default App
