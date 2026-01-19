import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { motion } from 'framer-motion';

interface Event {
  id: string;
  name: string;
  thumbnail: string;
  timestamp: string;
  selected?: boolean;
}

export default function EventsPage() {
  const navigate = useNavigate();
  const [events, setEvents] = useState<Event[]>([
    {
      id: '1',
      name: 'Grand Gala 2024',
      thumbnail: 'https://images.unsplash.com/photo-1511795409834-ef04bbd61622?w=400',
      timestamp: '35 phút trước',
      selected: true,
    },
    {
      id: '2',
      name: 'Wedding_Template_01',
      thumbnail: 'https://images.unsplash.com/photo-1519741497674-611481863552?w=400',
      timestamp: '2 giờ trước',
    },
    {
      id: '3',
      name: 'Birthday_Session',
      thumbnail: 'https://images.unsplash.com/photo-1530103862676-de8c9debad1d?w=400',
      timestamp: '1 ngày trước',
    },
    {
      id: '4',
      name: 'Corporate_Gala',
      thumbnail: 'https://images.unsplash.com/photo-1540575467063-178a50c2df87?w=400',
      timestamp: '3 ngày trước',
    },
  ]);

  const [sortBy, setSortBy] = useState('name');
  const [filterTime, setFilterTime] = useState('all');
  const [searchQuery, setSearchQuery] = useState('');

  const handleSelectAll = () => {
    setEvents(events.map((e) => ({ ...e, selected: true })));
  };

  const handleToggleSelect = (id: string) => {
    setEvents(events.map((e) => (e.id === id ? { ...e, selected: !e.selected } : e)));
  };

  const handleLaunchEvent = () => {
    const selectedEvent = events.find((e) => e.selected);
    if (selectedEvent) {
      navigate('/mode-selection');
    }
  };

  const handleSettingsEvent = () => {
    const selectedEvent = events.find((e) => e.selected);
    if (selectedEvent) {
      navigate('/settings');
    }
  };

  const selectedCount = events.filter((e) => e.selected).length;

  const buttonBaseClass = "px-4 py-2 border rounded-full text-xs font-medium transition-all flex items-center gap-1.5";
  const buttonDefaultClass = `${buttonBaseClass} border-white/40 hover:border-primary hover:bg-primary/10`;
  const buttonDisabledClass = "disabled:opacity-40 disabled:cursor-not-allowed disabled:hover:border-white/40 disabled:hover:bg-transparent";
  const buttonPrimaryClass = `${buttonBaseClass} border-primary bg-primary/10 text-primary hover:bg-primary/20`;

  return (
    <div className="min-h-screen bg-background-dark text-white flex flex-col">
      {/* Top Toolbar */}
      <header className="h-16 border-b border-white/10 flex items-center justify-between px-6 bg-background-dark">
        <div className="flex items-center gap-4">
          <span className="text-sm font-semibold mr-2">Sự kiện của bạn</span>
          <div className="flex items-center gap-2">
            <button
              onClick={handleSelectAll}
              className={`${buttonDefaultClass}`}
            >
              Chọn tất cả
            </button>
            <button
              disabled={selectedCount === 0}
              className={`${buttonDefaultClass} ${buttonDisabledClass}`}
            >
              Đổi tên
            </button>
            <button
              disabled={selectedCount === 0}
              className={`${buttonBaseClass} border-white/40 hover:border-red-500 hover:bg-red-500/10 ${buttonDisabledClass}`}
            >
              <span className="material-symbols-outlined text-sm">delete</span>
              Xóa
            </button>
            <button
              disabled={selectedCount === 0}
              className={`${buttonDefaultClass} ${buttonDisabledClass}`}
            >
              <span className="material-symbols-outlined text-sm">content_copy</span>
              Nhân bản
            </button>
            <button className={buttonPrimaryClass}>
              <span className="material-symbols-outlined text-sm">add</span>
              Tạo mới
            </button>
          </div>
        </div>
        <div className="flex items-center gap-3">
          {selectedCount > 0 && (
            <button
              onClick={handleSettingsEvent}
              className="bg-surface-dark hover:bg-white/10 text-white px-5 py-2.5 rounded-full flex items-center gap-2 text-sm font-bold transition-all border border-white/20 hover:border-white/40"
            >
              <span className="material-symbols-outlined text-lg">settings</span>
              Cài đặt
            </button>
          )}
          <button
            onClick={handleLaunchEvent}
            disabled={selectedCount === 0}
            className="bg-primary hover:bg-primary-600 text-white px-5 py-2.5 rounded-full flex items-center gap-2 text-sm font-bold transition-all disabled:opacity-40 disabled:cursor-not-allowed shadow-lg shadow-primary/20"
          >
            <span className="material-symbols-outlined text-lg">rocket_launch</span>
            Khởi chạy
          </button>
        </div>
      </header>

      {/* Filter Bar */}
      <div className="h-14 border-b border-white/10 flex items-center px-6 gap-4 bg-background-dark">
        <button
          onClick={() => setSortBy(sortBy === 'name' ? 'date' : 'name')}
          className={`${buttonDefaultClass}`}
        >
          <span className="material-symbols-outlined text-sm">sort_by_alpha</span>
          Sắp xếp theo {sortBy === 'name' ? 'tên' : 'ngày'}
        </button>
        <div className="relative">
          <select
            value={filterTime}
            onChange={(e) => setFilterTime(e.target.value)}
            className="bg-transparent border border-white/40 hover:border-primary rounded-full text-xs py-2 pl-4 pr-8 appearance-none focus:ring-1 focus:ring-primary outline-none cursor-pointer transition-colors"
          >
            <option value="all">Tất cả thời gian</option>
            <option value="7days">7 ngày qua</option>
            <option value="30days">30 ngày qua</option>
          </select>
          <span className="material-symbols-outlined absolute right-2 top-1/2 -translate-y-1/2 text-sm pointer-events-none">
            expand_more
          </span>
        </div>
        <div className="relative flex-1 max-w-xs ml-auto">
          <input
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="w-full bg-transparent border-b border-white/40 border-t-0 border-l-0 border-r-0 py-2 text-sm focus:ring-0 focus:border-primary outline-none placeholder:text-white/40 transition-colors"
            placeholder="Tìm kiếm sự kiện..."
            type="text"
          />
          <span className="material-symbols-outlined absolute right-0 top-1/2 -translate-y-1/2 text-lg text-white/60">
            search
          </span>
        </div>
      </div>

      {/* Main Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Events Grid - Scrollable */}
        <main className="flex-1 overflow-y-auto p-6 scrollbar-thin scrollbar-thumb-white/20 scrollbar-track-transparent hover:scrollbar-thumb-white/30">
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
            {events.map((event, index) => (
              <motion.div
                key={event.id}
                initial={{ opacity: 0, y: 20 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ delay: index * 0.05 }}
                onClick={() => handleToggleSelect(event.id)}
                className={`group relative rounded-xl border-2 overflow-hidden p-2 transition-all cursor-pointer ${
                  event.selected
                    ? 'border-primary bg-primary/5'
                    : 'border-white/10 bg-background-dark/50 hover:border-white/30'
                }`}
              >
                {/* Selection Checkbox */}
                <div className="absolute top-3 left-3 z-10">
                  <div
                    className={`w-6 h-6 rounded-full flex items-center justify-center transition-all ${
                      event.selected
                        ? 'bg-primary'
                        : 'bg-white/20 group-hover:bg-white/30'
                    }`}
                  >
                    {event.selected && (
                      <span className="material-symbols-outlined text-sm font-bold text-white">
                        check
                      </span>
                    )}
                  </div>
                </div>

                {/* Thumbnail */}
                <div className="aspect-video bg-white/10 mb-2 rounded-lg overflow-hidden">
                  <div
                    className="w-full h-full bg-cover bg-center transition-transform group-hover:scale-105"
                    style={{ backgroundImage: `url(${event.thumbnail})` }}
                  />
                </div>

                {/* Event Info */}
                <div className="text-center pb-2">
                  <p className="text-sm font-medium text-white truncate">{event.name}</p>
                  <p className="text-xs text-white/40 mt-1">{event.timestamp}</p>
                </div>
              </motion.div>
            ))}
          </div>
        </main>

        {/* Right Sidebar - Event Configuration - Scrollable */}
        <aside className="w-[400px] border-l border-white/10 flex flex-col bg-background-dark overflow-y-auto scrollbar-thin scrollbar-thumb-white/20 scrollbar-track-transparent hover:scrollbar-thumb-white/30">
          <div className="p-6 pb-2">
            <button className="w-full bg-surface-dark hover:bg-white/5 border border-white/10 rounded-xl py-3 px-4 flex items-center justify-between group transition-colors mb-6">
              <div className="flex items-center gap-3">
                <span className="material-symbols-outlined text-primary">tune</span>
                <span className="text-sm font-bold">Cấu hình sự kiện</span>
              </div>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white transition-colors">
                chevron_right
              </span>
            </button>

            <h3 className="text-sm font-semibold mb-4">Màn hình bắt đầu</h3>
            <div className="relative aspect-video bg-surface-dark rounded-xl border border-white/10 flex items-center justify-center overflow-hidden">
              {/* Background Icon */}
              <div className="absolute inset-0 flex items-center justify-center opacity-10 pointer-events-none">
                <span className="material-symbols-outlined text-8xl">image</span>
              </div>

              {/* Mode Icons */}
              <div className="flex gap-4 z-10">
                <div className="flex flex-col items-center gap-1">
                  <div className="w-8 h-8 rounded-full border border-white flex items-center justify-center">
                    <span className="material-symbols-outlined text-sm">print</span>
                  </div>
                  <span className="text-[8px] uppercase">In ảnh</span>
                </div>
                <div className="flex flex-col items-center gap-1">
                  <div className="w-8 h-8 rounded-full border border-white flex items-center justify-center">
                    <span className="material-symbols-outlined text-sm">all_inclusive</span>
                  </div>
                  <span className="text-[8px] uppercase">Boomerang</span>
                </div>
                <div className="flex flex-col items-center gap-1">
                  <div className="w-8 h-8 rounded-full border border-white flex items-center justify-center">
                    <span className="material-symbols-outlined text-sm">videocam</span>
                  </div>
                  <span className="text-[8px] uppercase">Video</span>
                </div>
              </div>

              {/* Corner Icons */}
              <span className="material-symbols-outlined absolute top-4 left-4 text-lg text-white/60">
                image_search
              </span>
              <span className="material-symbols-outlined absolute top-4 right-4 text-lg text-white/60">
                fullscreen
              </span>
            </div>
          </div>

          <hr className="border-white/10 mx-6" />

          {/* Capture Modes */}
          <div className="p-6">
            <h3 className="text-sm font-semibold mb-6">Chế độ chụp</h3>
            <div className="grid grid-cols-4 gap-4 text-center">
              <div className="flex flex-col items-center gap-3">
                <div className="w-14 h-14 rounded-xl border-2 border-primary flex items-center justify-center">
                  <span className="material-symbols-outlined text-3xl text-primary">
                    camera_alt
                  </span>
                </div>
                <span className="text-[11px] font-medium">Ảnh</span>
              </div>
              <div className="flex flex-col items-center gap-3">
                <div className="w-14 h-14 rounded-xl border-2 border-primary flex items-center justify-center">
                  <div className="relative w-8 h-8 border-2 border-primary rounded-sm before:absolute before:-top-1 before:-right-1 before:w-8 before:h-8 before:border-2 before:border-primary before:rounded-sm"></div>
                </div>
                <span className="text-[11px] font-medium text-white/80">GIF</span>
              </div>
              <div className="flex flex-col items-center gap-3">
                <div className="w-14 h-14 rounded-xl border border-white/20 flex items-center justify-center">
                  <span className="material-symbols-outlined text-3xl text-white/60">
                    all_inclusive
                  </span>
                </div>
                <span className="text-[11px] font-medium text-white/60">Boomerang</span>
              </div>
              <div className="flex flex-col items-center gap-3">
                <div className="w-14 h-14 rounded-xl border-2 border-primary flex items-center justify-center">
                  <span className="material-symbols-outlined text-3xl text-primary">
                    videocam
                  </span>
                </div>
                <span className="text-[11px] font-medium text-white/80">Video</span>
              </div>
            </div>
          </div>

          <hr className="border-white/10 mx-6" />

          {/* Configuration Options */}
          <div className="flex-1 px-6 py-4 space-y-2">
            <div className="flex items-center justify-between group cursor-pointer hover:bg-white/5 p-3 rounded-lg transition-colors">
              <span className="text-sm text-white/80 group-hover:text-white">Hiệu ứng</span>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white">
                chevron_right
              </span>
            </div>
            <div className="flex items-center justify-between group cursor-pointer hover:bg-white/5 p-3 rounded-lg transition-colors">
              <span className="text-sm text-white/80 group-hover:text-white">Đạo cụ kỹ thuật số</span>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white">
                chevron_right
              </span>
            </div>
            <div className="flex items-center justify-between group cursor-pointer hover:bg-white/5 p-3 rounded-lg transition-colors">
              <span className="text-sm text-white/80 group-hover:text-white">Bộ lọc làm đẹp</span>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white">
                chevron_right
              </span>
            </div>
            <div className="flex items-center justify-between group cursor-pointer hover:bg-white/5 p-3 rounded-lg transition-colors">
              <span className="text-sm text-white/80 group-hover:text-white">Watermark</span>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white">
                chevron_right
              </span>
            </div>
            <div className="flex items-center justify-between group cursor-pointer hover:bg-white/5 p-3 rounded-lg transition-colors">
              <span className="text-sm text-white/80 group-hover:text-white">Xử lý hậu kỳ</span>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white">
                chevron_right
              </span>
            </div>
          </div>
        </aside>
      </div>
    </div>
  );
}
