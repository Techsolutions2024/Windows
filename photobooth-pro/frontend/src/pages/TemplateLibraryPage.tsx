import { useState } from 'react';
import { motion } from 'framer-motion';
import { useNavigate } from 'react-router-dom';

interface Template {
  id: string;
  name: string;
  category: string;
  thumbnail: string;
  isPremium?: boolean;
  downloads?: number;
}

export default function TemplateLibraryPage() {
  const navigate = useNavigate();
  const [selectedCategory, setSelectedCategory] = useState('all');
  const [searchQuery, setSearchQuery] = useState('');

  const categories = [
    { id: 'all', name: 'All Templates', icon: 'grid_view' },
    { id: 'wedding', name: 'Wedding', icon: 'favorite' },
    { id: 'birthday', name: 'Birthday', icon: 'cake' },
    { id: 'corporate', name: 'Corporate', icon: 'business' },
    { id: 'holiday', name: 'Holiday', icon: 'celebration' },
    { id: 'custom', name: 'My Templates', icon: 'folder' },
  ];

  const templates: Template[] = [
    {
      id: '1',
      name: 'Elegant Wedding',
      category: 'wedding',
      thumbnail: 'https://images.unsplash.com/photo-1519741497674-611481863552?w=400',
      isPremium: true,
      downloads: 1250,
    },
    {
      id: '2',
      name: 'Birthday Celebration',
      category: 'birthday',
      thumbnail: 'https://images.unsplash.com/photo-1530103862676-de8c9debad1d?w=400',
      downloads: 890,
    },
    {
      id: '3',
      name: 'Corporate Event',
      category: 'corporate',
      thumbnail: 'https://images.unsplash.com/photo-1540575467063-178a50c2df87?w=400',
      isPremium: true,
      downloads: 650,
    },
    {
      id: '4',
      name: 'Christmas Party',
      category: 'holiday',
      thumbnail: 'https://images.unsplash.com/photo-1512389142860-9c449e58a543?w=400',
      downloads: 2100,
    },
    {
      id: '5',
      name: 'Modern Minimalist',
      category: 'wedding',
      thumbnail: 'https://images.unsplash.com/photo-1511795409834-ef04bbd61622?w=400',
      downloads: 780,
    },
    {
      id: '6',
      name: 'Kids Birthday',
      category: 'birthday',
      thumbnail: 'https://images.unsplash.com/photo-1464349095431-e9a21285b5f3?w=400',
      isPremium: true,
      downloads: 1450,
    },
  ];

  const filteredTemplates = templates.filter((template) => {
    const matchesCategory = selectedCategory === 'all' || template.category === selectedCategory;
    const matchesSearch = template.name.toLowerCase().includes(searchQuery.toLowerCase());
    return matchesCategory && matchesSearch;
  });

  return (
    <div className="min-h-screen bg-background-dark text-white flex">
      {/* Left Sidebar - Categories */}
      <aside className="w-64 border-r border-white/10 bg-background-dark flex flex-col">
        <div className="p-6 border-b border-white/10">
          <button
            onClick={() => navigate('/')}
            className="flex items-center gap-2 text-white/60 hover:text-white transition-colors mb-6"
          >
            <span className="material-symbols-outlined">arrow_back</span>
            <span className="text-sm font-medium">Back to Events</span>
          </button>
          <h1 className="text-2xl font-bold mb-2">Template Library</h1>
          <p className="text-sm text-white/60">Choose from professional designs</p>
        </div>

        <nav className="flex-1 p-4 space-y-1">
          {categories.map((category) => (
            <button
              key={category.id}
              onClick={() => setSelectedCategory(category.id)}
              className={`w-full flex items-center gap-3 px-4 py-3 rounded-lg transition-all ${
                selectedCategory === category.id
                  ? 'bg-primary text-white shadow-lg shadow-primary/20'
                  : 'text-white/70 hover:bg-white/5 hover:text-white'
              }`}
            >
              <span className="material-symbols-outlined text-xl">{category.icon}</span>
              <span className="text-sm font-medium">{category.name}</span>
            </button>
          ))}
        </nav>

        <div className="p-4 border-t border-white/10">
          <button className="w-full flex items-center justify-center gap-2 px-4 py-3 rounded-lg bg-primary hover:bg-primary-600 transition-colors shadow-lg shadow-primary/20">
            <span className="material-symbols-outlined">add</span>
            <span className="text-sm font-bold">Create Custom</span>
          </button>
        </div>
      </aside>

      {/* Main Content */}
      <main className="flex-1 flex flex-col">
        {/* Top Bar */}
        <header className="h-16 border-b border-white/10 flex items-center justify-between px-8 bg-background-dark">
          <div className="flex items-center gap-4">
            <h2 className="text-lg font-semibold">
              {categories.find((c) => c.id === selectedCategory)?.name || 'All Templates'}
            </h2>
            <span className="text-sm text-white/40">
              {filteredTemplates.length} templates
            </span>
          </div>

          <div className="flex items-center gap-4">
            {/* Search */}
            <div className="relative">
              <input
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="w-80 bg-white/5 border border-white/10 rounded-lg py-2 pl-10 pr-4 text-sm focus:ring-1 focus:ring-primary focus:border-primary outline-none placeholder:text-white/40 transition-colors"
                placeholder="Search templates..."
                type="text"
              />
              <span className="material-symbols-outlined absolute left-3 top-1/2 -translate-y-1/2 text-white/40 text-lg">
                search
              </span>
            </div>

            {/* View Toggle */}
            <div className="flex gap-1 bg-white/5 rounded-lg p-1">
              <button className="p-2 rounded bg-primary text-white">
                <span className="material-symbols-outlined text-lg">grid_view</span>
              </button>
              <button className="p-2 rounded text-white/60 hover:text-white hover:bg-white/10 transition-colors">
                <span className="material-symbols-outlined text-lg">view_list</span>
              </button>
            </div>
          </div>
        </header>

        {/* Templates Grid */}
        <div className="flex-1 overflow-y-auto p-8">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
            {filteredTemplates.map((template, index) => (
              <motion.div
                key={template.id}
                initial={{ opacity: 0, y: 20 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ delay: index * 0.05 }}
                className="group relative rounded-xl border border-white/10 bg-surface-dark overflow-hidden hover:border-primary transition-all cursor-pointer"
              >
                {/* Premium Badge */}
                {template.isPremium && (
                  <div className="absolute top-3 right-3 z-10 px-3 py-1 rounded-full bg-gradient-to-r from-yellow-500 to-orange-500 text-xs font-bold uppercase tracking-wider shadow-lg">
                    Premium
                  </div>
                )}

                {/* Thumbnail */}
                <div className="aspect-video bg-white/5 overflow-hidden">
                  <div
                    className="w-full h-full bg-cover bg-center transition-transform group-hover:scale-110"
                    style={{ backgroundImage: `url(${template.thumbnail})` }}
                  />
                </div>

                {/* Info */}
                <div className="p-4">
                  <h3 className="text-sm font-bold mb-1 truncate">{template.name}</h3>
                  <div className="flex items-center justify-between text-xs text-white/60">
                    <span className="flex items-center gap-1">
                      <span className="material-symbols-outlined text-sm">download</span>
                      {template.downloads?.toLocaleString()}
                    </span>
                    <span className="capitalize">{template.category}</span>
                  </div>
                </div>

                {/* Hover Actions */}
                <div className="absolute inset-0 bg-black/80 backdrop-blur-sm opacity-0 group-hover:opacity-100 transition-opacity flex items-center justify-center gap-3">
                  <button className="px-6 py-2 rounded-lg bg-primary hover:bg-primary-600 text-white font-bold transition-colors shadow-lg">
                    Use Template
                  </button>
                  <button className="p-2 rounded-lg bg-white/10 hover:bg-white/20 text-white transition-colors">
                    <span className="material-symbols-outlined">visibility</span>
                  </button>
                </div>
              </motion.div>
            ))}
          </div>

          {/* Empty State */}
          {filteredTemplates.length === 0 && (
            <div className="flex flex-col items-center justify-center h-96 text-center">
              <span className="material-symbols-outlined text-6xl text-white/20 mb-4">
                search_off
              </span>
              <h3 className="text-xl font-bold mb-2">No templates found</h3>
              <p className="text-white/60 mb-6">
                Try adjusting your search or browse different categories
              </p>
              <button
                onClick={() => {
                  setSearchQuery('');
                  setSelectedCategory('all');
                }}
                className="px-6 py-2 rounded-lg bg-primary hover:bg-primary-600 transition-colors"
              >
                Clear Filters
              </button>
            </div>
          )}
        </div>
      </main>
    </div>
  );
}
