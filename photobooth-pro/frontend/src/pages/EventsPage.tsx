import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { motion } from 'framer-motion';
import api, { Event as ApiEvent } from '../services/api';
import CreateEventModal from '../components/CreateEventModal';

// Extend ApiEvent to include UI-specific state
interface Event extends ApiEvent {
  selected?: boolean;
}

export default function EventsPage() {
  const navigate = useNavigate();
  const [events, setEvents] = useState<Event[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const [sortBy, setSortBy] = useState('createdAt');
  const [sortOrder, setSortOrder] = useState<'ASC' | 'DESC'>('DESC');
  const [filterTime, setFilterTime] = useState('all');
  const [searchQuery, setSearchQuery] = useState('');

  const [showCreateModal, setShowCreateModal] = useState(false);

  // Fetch events
  const fetchEvents = async () => {
    setLoading(true);
    try {
      const response = searchQuery
        ? await api.searchEvents(searchQuery)
        : await api.getEvents({ sortBy, order: sortOrder });

      if (response.success && response.data) {
        const mappedEvents = response.data.map(e => ({
          ...e,
          selected: false
        }));
        setEvents(mappedEvents);
      } else {
        setError(response.error || 'Failed to fetch events');
      }
    } catch (err) {
      setError('An unexpected error occurred');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchEvents();
  }, [sortBy, sortOrder, searchQuery]); // Refetch when these change

  const handleSelectAll = () => {
    const allSelected = events.every(e => e.selected);
    setEvents(events.map((e) => ({ ...e, selected: !allSelected })));
  };

  const handleToggleSelect = (id: number) => {
    setEvents(events.map((e) => (e.id === id ? { ...e, selected: !e.selected } : e)));
  };

  const handleOpenCreateModal = () => {
    setShowCreateModal(true);
  };

  const handleCreateSubmit = async (name: string, location: string, date: string) => {
    try {
      const response = await api.createEvent(name, location, date);
      if (response.success) {
        fetchEvents();
      } else {
        alert(response.error || "Không thể tạo sự kiện");
      }
    } catch (error) {
      console.error(error);
      alert("Lỗi khi tạo sự kiện");
    }
  };

  const handleRenameEvent = async () => {
    const selectedEvent = events.find(e => e.selected);
    if (!selectedEvent) return;

    const newName = window.prompt("Nhập tên mới:", selectedEvent.name);
    if (!newName || newName === selectedEvent.name) return;

    try {
      const response = await api.updateEvent(selectedEvent.id, newName, selectedEvent.location, selectedEvent.eventDate);
      if (response.success) {
        fetchEvents();
      } else {
        alert(response.error || "Không thể đổi tên sự kiện");
      }
    } catch (error) {
      console.error(error);
      alert("Lỗi khi đổi tên sự kiện");
    }
  };

  const handleDeleteEvent = async () => {
    const selectedEvents = events.filter(e => e.selected);
    if (selectedEvents.length === 0) return;

    if (!window.confirm(`Bạn có chắc chắn muốn xóa ${selectedEvents.length} sự kiện đã chọn?`)) {
      return;
    }

    try {
      let successCount = 0;
      for (const event of selectedEvents) {
        const res = await api.deleteEvent(event.id);
        if (res.success) successCount++;
      }

      if (successCount > 0) {
        fetchEvents();
      }
    } catch (error) {
      console.error(error);
      alert("Lỗi khi xóa sự kiện");
    }
  };

  const handleDuplicateEvent = async () => {
    const selectedEvent = events.find(e => e.selected);
    if (!selectedEvent) return;

    try {
      const response = await api.duplicateEvent(selectedEvent.id);
      if (response.success) {
        fetchEvents();
      } else {
        alert(response.error || "Không thể nhân bản sự kiện");
      }
    } catch (error) {
      console.error(error);
      alert("Lỗi khi nhân bản sự kiện");
    }
  };

  const handleLaunchEvent = () => {
    const selectedEvent = events.find((e) => e.selected);
    if (selectedEvent) {
      navigate(`/mode-selection?eventId=${selectedEvent.id}`);
    }
  };

  const handleSettingsEvent = () => {
    const selectedEvent = events.find((e) => e.selected);
    if (selectedEvent) {
      navigate(`/settings?eventId=${selectedEvent.id}`);
    }
  };

  const selectedCount = events.filter((e) => e.selected).length;
  const singleSelection = selectedCount === 1;

  const buttonBaseClass = "px-4 py-2 border rounded-full text-xs font-medium transition-all flex items-center gap-1.5";
  const buttonDefaultClass = `${buttonBaseClass} border-white/40 hover:border-primary hover:bg-primary/10`;
  const buttonDisabledClass = "disabled:opacity-40 disabled:cursor-not-allowed disabled:hover:border-white/40 disabled:hover:bg-transparent";
  const buttonPrimaryClass = `${buttonBaseClass} border-primary bg-primary/10 text-primary hover:bg-primary/20`;

  return (
    <div className="min-h-screen bg-background-dark text-white flex flex-col">
      <CreateEventModal
        isOpen={showCreateModal}
        onClose={() => setShowCreateModal(false)}
        onSubmit={handleCreateSubmit}
      />

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
              onClick={handleRenameEvent}
              disabled={!singleSelection}
              className={`${buttonDefaultClass} ${buttonDisabledClass}`}
            >
              Đổi tên
            </button>
            <button
              onClick={handleDeleteEvent}
              disabled={selectedCount === 0}
              className={`${buttonBaseClass} border-white/40 hover:border-red-500 hover:bg-red-500/10 ${buttonDisabledClass}`}
            >
              <span className="material-symbols-outlined text-sm">delete</span>
              Xóa
            </button>
            <button
              onClick={handleDuplicateEvent}
              disabled={!singleSelection}
              className={`${buttonDefaultClass} ${buttonDisabledClass}`}
            >
              <span className="material-symbols-outlined text-sm">content_copy</span>
              Nhân bản
            </button>
            <button
              onClick={handleOpenCreateModal}
              className={buttonPrimaryClass}
            >
              <span className="material-symbols-outlined text-sm">add</span>
              Tạo mới
            </button>
          </div>
        </div>
        <div className="flex items-center gap-3">
          {selectedCount > 0 && (
            <button
              onClick={handleSettingsEvent}
              disabled={!singleSelection}
              className={`bg-surface-dark hover:bg-white/10 text-white px-5 py-2.5 rounded-full flex items-center gap-2 text-sm font-bold transition-all border border-white/20 hover:border-white/40 ${!singleSelection ? 'opacity-50 cursor-not-allowed' : ''}`}
            >
              <span className="material-symbols-outlined text-lg">settings</span>
              Cài đặt
            </button>
          )}
          <button
            onClick={handleLaunchEvent}
            disabled={!singleSelection}
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
          onClick={() => {
            if (sortBy === 'name') {
              setSortBy('createdAt');
              setSortOrder('DESC');
            } else {
              setSortBy('name');
              setSortOrder('ASC');
            }
          }}
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
            <option value="all" className="bg-background-dark">Tất cả thời gian</option>
            <option value="7days" className="bg-background-dark">7 ngày qua (Demo)</option>
            <option value="30days" className="bg-background-dark">30 ngày qua (Demo)</option>
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
          {loading ? (
            <div className="flex items-center justify-center h-full">
              <div className="animate-spin rounded-full h-8 w-8 border-t-2 border-b-2 border-primary"></div>
            </div>
          ) : error ? (
            <div className="flex items-center justify-center h-full text-red-400">
              {error}
            </div>
          ) : events.length === 0 ? (
            <div className="flex flex-col items-center justify-center h-full text-white/40">
              <span className="material-symbols-outlined text-4xl mb-2">event_busy</span>
              <p>Chưa có sự kiện nào</p>
            </div>
          ) : (
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
              {events.map((event, index) => (
                <motion.div
                  key={event.id}
                  initial={{ opacity: 0, y: 20 }}
                  animate={{ opacity: 1, y: 0 }}
                  transition={{ delay: index * 0.05 }}
                  onClick={() => handleToggleSelect(event.id)}
                  className={`group relative rounded-xl border-2 overflow-hidden p-2 transition-all cursor-pointer ${event.selected
                    ? 'border-primary bg-primary/5'
                    : 'border-white/10 bg-background-dark/50 hover:border-white/30'
                    }`}
                >
                  {/* Selection Checkbox */}
                  <div className="absolute top-3 left-3 z-10">
                    <div
                      className={`w-6 h-6 rounded-full flex items-center justify-center transition-all ${event.selected
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
                  <div className="aspect-video bg-white/10 mb-2 rounded-lg overflow-hidden flex items-center justify-center">
                    {event.thumbnailPath ? (
                      <div
                        className="w-full h-full bg-cover bg-center transition-transform group-hover:scale-105"
                        style={{ backgroundImage: `url(${event.thumbnailPath})` }}
                      />
                    ) : (
                      <span className="material-symbols-outlined text-4xl text-white/20">
                        image
                      </span>
                    )}
                  </div>

                  {/* Event Info */}
                  <div className="text-center pb-2 px-2">
                    <p className="text-sm font-medium text-white truncate mb-1" title={event.name}>
                      {event.name}
                    </p>
                    {event.location && (
                      <p className="text-xs text-white/60 truncate flex items-center justify-center gap-1 mb-0.5" title={event.location}>
                        <span className="material-symbols-outlined text-[10px]">location_on</span>
                        {event.location}
                      </p>
                    )}
                    {event.eventDate && (
                      <p className="text-xs text-white/60 truncate flex items-center justify-center gap-1 mb-0.5">
                        <span className="material-symbols-outlined text-[10px]">calendar_today</span>
                        {new Date(event.eventDate).toLocaleDateString('vi-VN')}
                      </p>
                    )}
                    <p className="text-[10px] text-white/30 mt-1">
                      Tạo: {new Date(event.createdAt).toLocaleDateString('vi-VN')}
                    </p>
                  </div>
                </motion.div>
              ))}
            </div>
          )}
        </main>

        {/* Right Sidebar - Event Configuration - Placeholder for now, can be hydryated with real event data if single selection */}
        <aside className="w-[400px] border-l border-white/10 flex flex-col bg-background-dark overflow-y-auto scrollbar-thin scrollbar-thumb-white/20 scrollbar-track-transparent hover:scrollbar-thumb-white/30">
          <div className="p-6 pb-2">
            <button
              onClick={handleSettingsEvent}
              disabled={!singleSelection}
              className={`w-full bg-surface-dark hover:bg-white/5 border border-white/10 rounded-xl py-3 px-4 flex items-center justify-between group transition-colors mb-6 ${!singleSelection ? 'opacity-50 cursor-not-allowed' : ''}`}
            >
              <div className="flex items-center gap-3">
                <span className="material-symbols-outlined text-primary">tune</span>
                <span className="text-sm font-bold">Cấu hình sự kiện</span>
              </div>
              <span className="material-symbols-outlined text-white/40 group-hover:text-white transition-colors">
                chevron_right
              </span>
            </button>

            {/* If a single event is selected, we could show its config here. For now static or based on selection. */}
            {singleSelection && (
              <>
                <h3 className="text-sm font-semibold mb-4">Màn hình bắt đầu</h3>
                <div className="relative aspect-video bg-surface-dark rounded-xl border border-white/10 flex items-center justify-center overflow-hidden">
                  <div className="absolute inset-0 flex items-center justify-center opacity-10 pointer-events-none">
                    <span className="material-symbols-outlined text-8xl">image</span>
                  </div>
                </div>
              </>
            )}
            {!singleSelection && (
              <div className="text-white/40 text-center text-sm py-10">
                Chọn một sự kiện để xem cấu hình
              </div>
            )}
          </div>
        </aside>
      </div>
    </div>
  );
}
