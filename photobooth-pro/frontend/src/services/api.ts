const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:8080';

interface ApiResponse<T> {
  success: boolean;
  message?: string;
  data?: T;
  error?: string;
  total?: number;
}

// ==================== Events API ====================

export interface Event {
  id: number;
  name: string;
  location?: string;
  eventDate?: string;
  createdAt: string;
  updatedAt: string;
  photoCount: number;
  thumbnailPath: string;
  status: string;
  config?: EventConfig;
}

export interface EventConfig {
  eventId: number;
  startScreenImage: string;
  captureMode: string;
  photoEnabled: boolean;
  gifEnabled: boolean;
  boomerangEnabled: boolean;
  videoEnabled: boolean;
  effectsConfig: string;
  propsConfig: string;
  beautyFilterConfig: string;
  watermarkConfig: string;
  postProcessConfig: string;
  countdownSeconds: number;
  photoCount: number;
  layoutTemplate: string;
  cameraSource: 'canon' | 'webcam';
  webcamIndex: number;
}

export interface Camera {
  name: string;
  type: 'canon' | 'webcam';
  connected: boolean;
  webcamIndex?: number;
}

export interface Photo {
  id: number;
  eventId: number;
  filePath: string;
  thumbnailPath: string;
  captureMode: string;
  timestamp: string;
  width: number;
  height: number;
  printed: boolean;
  shared: boolean;
  filterApplied?: string;
  metadata?: string;
}

class ApiService {
  private async request<T>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<ApiResponse<T>> {
    const url = `${API_BASE_URL}${endpoint}`;

    const defaultHeaders: HeadersInit = {
      'Content-Type': 'application/json',
    };

    try {
      const response = await fetch(url, {
        ...options,
        headers: {
          ...defaultHeaders,
          ...options.headers,
        },
      });

      const data = await response.json();
      return data;
    } catch (error) {
      console.error('API Error:', error);
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error',
      };
    }
  }

  // ==================== Events ====================

  async getEvents(params?: {
    sortBy?: string;
    order?: 'ASC' | 'DESC';
    filter?: string;
  }): Promise<ApiResponse<Event[]>> {
    const queryParams = new URLSearchParams();
    if (params?.sortBy) queryParams.append('sortBy', params.sortBy);
    if (params?.order) queryParams.append('order', params.order);
    if (params?.filter) queryParams.append('filter', params.filter);

    const query = queryParams.toString();
    return this.request<Event[]>(`/api/events${query ? `?${query}` : ''}`);
  }

  async createEvent(name: string, location?: string, eventDate?: string): Promise<ApiResponse<Event>> {
    return this.request<Event>('/api/events', {
      method: 'POST',
      body: JSON.stringify({ name, location, eventDate }),
    });
  }

  async getEvent(id: number): Promise<ApiResponse<Event>> {
    return this.request<Event>(`/api/events/${id}`);
  }

  async updateEvent(id: number, name: string, location?: string, eventDate?: string): Promise<ApiResponse<Event>> {
    return this.request<Event>(`/api/events/${id}`, {
      method: 'PUT',
      body: JSON.stringify({ name, location, eventDate }),
    });
  }

  async deleteEvent(id: number): Promise<ApiResponse<void>> {
    return this.request<void>(`/api/events/${id}`, {
      method: 'DELETE',
    });
  }

  async duplicateEvent(id: number, newName?: string): Promise<ApiResponse<Event>> {
    return this.request<Event>(`/api/events/${id}/duplicate`, {
      method: 'POST',
      body: JSON.stringify({ name: newName }),
    });
  }

  async getEventConfig(id: number): Promise<ApiResponse<EventConfig>> {
    return this.request<EventConfig>(`/api/events/${id}/config`);
  }

  async updateEventConfig(
    id: number,
    config: Partial<EventConfig>
  ): Promise<ApiResponse<void>> {
    return this.request<void>(`/api/events/${id}/config`, {
      method: 'PUT',
      body: JSON.stringify(config),
    });
  }

  async searchEvents(query: string): Promise<ApiResponse<Event[]>> {
    return this.request<Event[]>(`/api/events/search?q=${encodeURIComponent(query)}`);
  }

  // ==================== Gallery ====================

  async getGallery(
    eventId: number,
    params?: { limit?: number; offset?: number }
  ): Promise<ApiResponse<Photo[]>> {
    const queryParams = new URLSearchParams();
    if (params?.limit) queryParams.append('limit', params.limit.toString());
    if (params?.offset) queryParams.append('offset', params.offset.toString());

    const query = queryParams.toString();
    return this.request<Photo[]>(`/api/gallery/${eventId}${query ? `?${query}` : ''}`);
  }

  async getPhoto(id: number): Promise<ApiResponse<Photo>> {
    return this.request<Photo>(`/api/gallery/photo/${id}`);
  }

  async deletePhoto(id: number): Promise<ApiResponse<void>> {
    return this.request<void>(`/api/gallery/photo/${id}`, {
      method: 'DELETE',
    });
  }

  async createCollage(photoIds: number[]): Promise<ApiResponse<{ collagePath: string }>> {
    return this.request<{ collagePath: string }>('/api/gallery/collage', {
      method: 'POST',
      body: JSON.stringify({ photoIds }),
    });
  }

  // ==================== Camera ====================

  async getCameras(): Promise<ApiResponse<Camera[]>> {
    return this.request<Camera[]>('/api/cameras');
  }

  async selectCamera(params: {
    type: 'canon' | 'webcam';
    name?: string;
    webcamIndex?: number;
  }): Promise<ApiResponse<void>> {
    return this.request<void>('/api/cameras/select', {
      method: 'POST',
      body: JSON.stringify(params),
    });
  }

  async getCameraSettings(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/cameras/settings');
  }

  async updateCameraSettings(settings: any): Promise<ApiResponse<void>> {
    return this.request<void>('/api/cameras/settings', {
      method: 'PUT',
      body: JSON.stringify(settings),
    });
  }

  async startLiveView(): Promise<ApiResponse<void>> {
    return this.request<void>('/api/cameras/liveview/start', {
      method: 'POST',
    });
  }

  async stopLiveView(): Promise<ApiResponse<void>> {
    return this.request<void>('/api/cameras/liveview/stop', {
      method: 'POST',
    });
  }

  // ==================== Capture ====================

  async capturePhoto(eventId: number): Promise<ApiResponse<Photo>> {
    return this.request<Photo>('/api/capture/photo', {
      method: 'POST',
      body: JSON.stringify({ eventId }),
    });
  }

  async captureGif(eventId: number, frameCount?: number): Promise<ApiResponse<Photo>> {
    return this.request<Photo>('/api/capture/gif', {
      method: 'POST',
      body: JSON.stringify({ eventId, frameCount }),
    });
  }

  async captureBoomerang(eventId: number): Promise<ApiResponse<Photo>> {
    return this.request<Photo>('/api/capture/boomerang', {
      method: 'POST',
      body: JSON.stringify({ eventId }),
    });
  }

  async startVideoRecording(eventId: number): Promise<ApiResponse<void>> {
    return this.request<void>('/api/capture/video/start', {
      method: 'POST',
      body: JSON.stringify({ eventId }),
    });
  }

  async stopVideoRecording(): Promise<ApiResponse<Photo>> {
    return this.request<Photo>('/api/capture/video/stop', {
      method: 'POST',
    });
  }

  // ==================== Print ====================

  async getPrinters(): Promise<ApiResponse<any[]>> {
    return this.request<any[]>('/api/printers');
  }

  async print(photoId: number, copies?: number): Promise<ApiResponse<void>> {
    return this.request<void>('/api/print', {
      method: 'POST',
      body: JSON.stringify({ photoId, copies }),
    });
  }

  async getPrintStatus(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/print/status');
  }

  async getPrintSettings(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/print/settings');
  }

  async updatePrintSettings(settings: any): Promise<ApiResponse<void>> {
    return this.request<void>('/api/print/settings', {
      method: 'PUT',
      body: JSON.stringify(settings),
    });
  }

  // ==================== Share ====================

  async shareEmail(photoId: number, email: string): Promise<ApiResponse<void>> {
    return this.request<void>('/api/share/email', {
      method: 'POST',
      body: JSON.stringify({ photoId, email }),
    });
  }

  async shareSMS(photoId: number, phone: string): Promise<ApiResponse<void>> {
    return this.request<void>('/api/share/sms', {
      method: 'POST',
      body: JSON.stringify({ photoId, phone }),
    });
  }

  async generateQRCode(url: string): Promise<ApiResponse<{ qrCodeUrl: string }>> {
    return this.request<{ qrCodeUrl: string }>('/api/share/qrcode', {
      method: 'POST',
      body: JSON.stringify({ url }),
    });
  }

  // ==================== Settings ====================

  async getSettings(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/settings');
  }

  async updateSettings(settings: any): Promise<ApiResponse<void>> {
    return this.request<void>('/api/settings', {
      method: 'PUT',
      body: JSON.stringify(settings),
    });
  }

  async getStorageInfo(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/settings/storage');
  }

  async getNetworkStatus(): Promise<ApiResponse<any>> {
    return this.request<any>('/api/settings/network');
  }
}

export const api = new ApiService();
export default api;
