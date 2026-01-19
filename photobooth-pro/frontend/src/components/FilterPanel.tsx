interface FilterPanelProps {
  selectedFilter: string
  onFilterChange: (filter: string) => void
}

export default function FilterPanel({ selectedFilter, onFilterChange }: FilterPanelProps) {
  const filters = [
    { id: 'none', name: 'None', preview: '🎨' },
    { id: 'bw', name: 'Black & White', preview: '⚫' },
    { id: 'sepia', name: 'Sepia', preview: '🟤' },
    { id: 'vintage', name: 'Vintage', preview: '📷' },
    { id: 'pop', name: 'Pop', preview: '💥' },
    { id: 'retro', name: 'Retro', preview: '🕹️' },
    { id: 'cool', name: 'Cool', preview: '❄️' },
    { id: 'warm', name: 'Warm', preview: '🔥' },
  ]

  return (
    <div className="p-6">
      <h2 className="text-lg font-semibold mb-4">Filters</h2>
      
      <div className="grid grid-cols-2 gap-3">
        {filters.map((filter) => (
          <button
            key={filter.id}
            onClick={() => onFilterChange(filter.id)}
            className={`p-4 rounded-lg border-2 transition ${
              selectedFilter === filter.id
                ? 'border-primary bg-primary/10'
                : 'border-gray-700 bg-dark-lighter hover:border-gray-600'
            }`}
          >
            <div className="text-3xl mb-2">{filter.preview}</div>
            <div className="text-sm font-medium">{filter.name}</div>
          </button>
        ))}
      </div>

      {/* Adjustments */}
      <div className="mt-6 space-y-4">
        <h3 className="text-sm font-semibold">Adjustments</h3>
        
        {/* Brightness */}
        <div>
          <label className="block text-sm mb-2">Brightness</label>
          <input
            type="range"
            min="-100"
            max="100"
            defaultValue="0"
            className="w-full"
          />
        </div>

        {/* Contrast */}
        <div>
          <label className="block text-sm mb-2">Contrast</label>
          <input
            type="range"
            min="-100"
            max="100"
            defaultValue="0"
            className="w-full"
          />
        </div>

        {/* Saturation */}
        <div>
          <label className="block text-sm mb-2">Saturation</label>
          <input
            type="range"
            min="-100"
            max="100"
            defaultValue="0"
            className="w-full"
          />
        </div>
      </div>

      {/* Reset Button */}
      <button
        onClick={() => onFilterChange('none')}
        className="w-full mt-6 px-4 py-2 bg-dark-lighter hover:bg-gray-600 rounded-lg transition"
      >
        Reset All
      </button>
    </div>
  )
}
