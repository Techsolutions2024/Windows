import { useState, useEffect } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import { Mail, MessageSquare, QrCode, Printer, ArrowLeft, Download, Trash2 } from 'lucide-react'
import { useAppStore } from '../store/useAppStore'

export default function SharingPage() {
  const { eventId } = useParams()
  const navigate = useNavigate()
  const { currentPhoto } = useAppStore()
  const [qrCodeUrl, setQrCodeUrl] = useState<string | null>(null)
  const [email, setEmail] = useState('')
  const [phone, setPhone] = useState('')
  const [printCopies, setPrintCopies] = useState(1)

  useEffect(() => {
    // Generate QR code for photo download
    // TODO: Implement QR code generation
  }, [currentPhoto])

  const handleEmail = async () => {
    if (!email || !currentPhoto) return
    // TODO: Send email via backend API
    console.log('Sending email to:', email)
  }

  const handleSMS = async () => {
    if (!phone || !currentPhoto) return
    // TODO: Send SMS via backend API
    console.log('Sending SMS to:', phone)
  }

  const handlePrint = async () => {
    if (!currentPhoto) return
    // TODO: Send print job to backend
    console.log('Printing', printCopies, 'copies')
  }

  const handleDownload = () => {
    if (!currentPhoto) return
    // TODO: Download photo
    console.log('Downloading photo')
  }

  const handleRetake = () => {
    navigate(`/capture/${eventId}`)
  }

  const handleDone = () => {
    navigate(`/capture/${eventId}`)
  }

  return (
    <div className="w-full h-screen bg-dark-darker flex">
      {/* Left Side - Photo Preview */}
      <div className="flex-1 flex flex-col bg-black">
        {/* Top Bar */}
        <div className="bg-dark border-b border-gray-700 px-6 py-3 flex items-center justify-between">
          <button
            onClick={() => navigate(`/capture/${eventId}`)}
            className="p-2 hover:bg-dark-lighter rounded-lg transition"
          >
            <ArrowLeft className="w-5 h-5" />
          </button>
          <h1 className="text-lg font-semibold">Share & Print</h1>
          <button
            onClick={handleDone}
            className="px-4 py-2 bg-primary rounded-lg hover:bg-primary-dark transition"
          >
            Done
          </button>
        </div>

        {/* Photo Display */}
        <div className="flex-1 flex items-center justify-center p-8">
          {currentPhoto ? (
            <img
              src={currentPhoto.filePath}
              alt="Captured"
              className="max-w-full max-h-full object-contain rounded-lg shadow-2xl"
            />
          ) : (
            <div className="text-center text-gray-400">
              <p className="text-xl mb-2">No photo selected</p>
              <button
                onClick={handleRetake}
                className="px-6 py-3 bg-primary rounded-lg hover:bg-primary-dark transition"
              >
                Take Photo
              </button>
            </div>
          )}
        </div>

        {/* Bottom Actions */}
        <div className="bg-dark border-t border-gray-700 px-6 py-4 flex items-center justify-between">
          <div className="flex gap-3">
            <button
              onClick={handleDownload}
              className="px-4 py-2 bg-dark-lighter hover:bg-gray-600 rounded-lg transition flex items-center gap-2"
            >
              <Download className="w-4 h-4" />
              Download
            </button>
            <button
              onClick={handleRetake}
              className="px-4 py-2 bg-dark-lighter hover:bg-gray-600 rounded-lg transition"
            >
              Retake
            </button>
          </div>
          <button className="p-2 text-red-500 hover:bg-red-500/10 rounded-lg transition">
            <Trash2 className="w-5 h-5" />
          </button>
        </div>
      </div>

      {/* Right Side - Sharing Options */}
      <div className="w-96 bg-dark border-l border-gray-700 flex flex-col">
        <div className="p-6 flex-1 overflow-y-auto">
          <h2 className="text-lg font-semibold mb-6">Sharing Options</h2>

          {/* Email */}
          <div className="mb-6 p-4 bg-dark-lighter rounded-lg">
            <div className="flex items-center gap-3 mb-3">
              <div className="w-12 h-12 bg-yellow-500 rounded-full flex items-center justify-center">
                <Mail className="w-6 h-6 text-white" />
              </div>
              <div>
                <h3 className="font-medium">Email</h3>
                <p className="text-sm text-gray-400">Send via email</p>
              </div>
            </div>
            <input
              type="email"
              placeholder="Enter email address"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className="w-full px-3 py-2 bg-dark rounded-lg mb-2 focus:outline-none focus:ring-2 focus:ring-primary"
            />
            <button
              onClick={handleEmail}
              disabled={!email}
              className="w-full px-4 py-2 bg-yellow-500 hover:bg-yellow-600 disabled:bg-gray-600 disabled:cursor-not-allowed rounded-lg transition text-white font-medium"
            >
              Send Email
            </button>
          </div>

          {/* SMS */}
          <div className="mb-6 p-4 bg-dark-lighter rounded-lg">
            <div className="flex items-center gap-3 mb-3">
              <div className="w-12 h-12 bg-green-500 rounded-full flex items-center justify-center">
                <MessageSquare className="w-6 h-6 text-white" />
              </div>
              <div>
                <h3 className="font-medium">SMS</h3>
                <p className="text-sm text-gray-400">Send via text message</p>
              </div>
            </div>
            <input
              type="tel"
              placeholder="Enter phone number"
              value={phone}
              onChange={(e) => setPhone(e.target.value)}
              className="w-full px-3 py-2 bg-dark rounded-lg mb-2 focus:outline-none focus:ring-2 focus:ring-primary"
            />
            <button
              onClick={handleSMS}
              disabled={!phone}
              className="w-full px-4 py-2 bg-green-500 hover:bg-green-600 disabled:bg-gray-600 disabled:cursor-not-allowed rounded-lg transition text-white font-medium"
            >
              Send SMS
            </button>
          </div>

          {/* QR Code */}
          <div className="mb-6 p-4 bg-dark-lighter rounded-lg">
            <div className="flex items-center gap-3 mb-3">
              <div className="w-12 h-12 bg-purple-500 rounded-full flex items-center justify-center">
                <QrCode className="w-6 h-6 text-white" />
              </div>
              <div>
                <h3 className="font-medium">Scan QR</h3>
                <p className="text-sm text-gray-400">Download to phone</p>
              </div>
            </div>
            <div className="bg-white p-4 rounded-lg flex items-center justify-center">
              {qrCodeUrl ? (
                <img src={qrCodeUrl} alt="QR Code" className="w-48 h-48" />
              ) : (
                <div className="w-48 h-48 bg-gray-200 flex items-center justify-center text-gray-500">
                  QR Code
                </div>
              )}
            </div>
            <p className="text-xs text-gray-400 text-center mt-2">
              Scan with your camera app to download
            </p>
          </div>

          {/* Print */}
          <div className="p-4 bg-dark-lighter rounded-lg">
            <div className="flex items-center gap-3 mb-3">
              <div className="w-12 h-12 bg-red-500 rounded-full flex items-center justify-center">
                <Printer className="w-6 h-6 text-white" />
              </div>
              <div>
                <h3 className="font-medium">Print</h3>
                <p className="text-sm text-gray-400">Print physical copy</p>
              </div>
            </div>
            <div className="mb-3">
              <label className="block text-sm mb-2">Number of copies</label>
              <input
                type="number"
                min="1"
                max="10"
                value={printCopies}
                onChange={(e) => setPrintCopies(parseInt(e.target.value))}
                className="w-full px-3 py-2 bg-dark rounded-lg focus:outline-none focus:ring-2 focus:ring-primary"
              />
            </div>
            <button
              onClick={handlePrint}
              className="w-full px-4 py-2 bg-red-500 hover:bg-red-600 rounded-lg transition text-white font-medium"
            >
              Print Now
            </button>
          </div>
        </div>

        {/* Settings Toggle */}
        <div className="p-4 border-t border-gray-700">
          <div className="flex items-center justify-between mb-2">
            <span className="text-sm">Show original photos</span>
            <button className="relative w-12 h-6 bg-primary rounded-full">
              <div className="absolute top-0.5 right-0.5 w-5 h-5 bg-white rounded-full"></div>
            </button>
          </div>
          <div className="flex items-center justify-between">
            <span className="text-sm">Text labels</span>
            <button className="relative w-12 h-6 bg-primary rounded-full">
              <div className="absolute top-0.5 right-0.5 w-5 h-5 bg-white rounded-full"></div>
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}
