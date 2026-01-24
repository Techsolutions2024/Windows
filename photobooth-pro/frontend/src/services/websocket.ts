type MessageHandler = (data: any) => void;

interface WebSocketMessage {
  type: string;
  data: any;
}

class WebSocketService {
  private ws: WebSocket | null = null;
  private url: string;
  private handlers: Map<string, Set<MessageHandler>> = new Map();
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;
  private reconnectDelay = 1000;
  private isConnecting = false;
  private lastBlobUrl: string | null = null; // Track last blob URL for cleanup

  constructor(url?: string) {
    this.url = url || 'ws://localhost:8081';
  }

  connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      if (this.ws?.readyState === WebSocket.OPEN) {
        resolve();
        return;
      }

      if (this.isConnecting) {
        resolve();
        return;
      }

      this.isConnecting = true;

      try {
        this.ws = new WebSocket(this.url);
        this.ws.binaryType = 'arraybuffer'; // Enable binary mode for efficiently receiving video frames

        this.ws.onopen = () => {
          console.log('WebSocket connected');
          this.isConnecting = false;
          this.reconnectAttempts = 0;
          resolve();
        };

        this.ws.onmessage = (event) => {
          // Handle Binary Data (Live View Stream)
          if (event.data instanceof ArrayBuffer) {
            // Revoke previous blob URL to prevent memory leak
            if (this.lastBlobUrl) {
              URL.revokeObjectURL(this.lastBlobUrl);
            }

            // Create a Blob from the ArrayBuffer for display
            const blob = new Blob([event.data], { type: 'image/jpeg' });
            const imageUrl = URL.createObjectURL(blob);
            this.lastBlobUrl = imageUrl;

            // Notify subscribers of the binary frame
            this.handleMessage({
              type: WS_EVENTS.LIVEVIEW_FRAME,
              data: { imageData: imageUrl, isBinary: true }
            });
            return;
          }

          // Handle Text Data (JSON Commands)
          try {
            if (typeof event.data === 'string') {
              const message: WebSocketMessage = JSON.parse(event.data);
              this.handleMessage(message);
            }
          } catch (error) {
            console.error('Failed to parse WebSocket message:', error);
          }
        };

        this.ws.onclose = () => {
          console.log('WebSocket disconnected');
          this.isConnecting = false;
          this.attemptReconnect();
        };

        this.ws.onerror = (error) => {
          console.error('WebSocket error:', error);
          this.isConnecting = false;
          reject(error);
        };
      } catch (error) {
        this.isConnecting = false;
        reject(error);
      }
    });
  }

  disconnect(): void {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  private attemptReconnect(): void {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.log('Max reconnect attempts reached');
      return;
    }

    this.reconnectAttempts++;
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);

    console.log(`Attempting to reconnect in ${delay}ms (attempt ${this.reconnectAttempts})`);

    setTimeout(() => {
      this.connect().catch(() => {
        // Reconnect failed, will try again
      });
    }, delay);
  }

  private handleMessage(message: WebSocketMessage): void {
    const handlers = this.handlers.get(message.type);
    if (handlers) {
      handlers.forEach((handler) => {
        try {
          handler(message.data);
        } catch (error) {
          console.error(`Error in handler for ${message.type}:`, error);
        }
      });
    }

    // Also emit to wildcard handlers
    const wildcardHandlers = this.handlers.get('*');
    if (wildcardHandlers) {
      wildcardHandlers.forEach((handler) => {
        try {
          handler(message);
        } catch (error) {
          console.error('Error in wildcard handler:', error);
        }
      });
    }
  }

  on(eventType: string, handler: MessageHandler): () => void {
    if (!this.handlers.has(eventType)) {
      this.handlers.set(eventType, new Set());
    }
    this.handlers.get(eventType)!.add(handler);

    // Return unsubscribe function
    return () => {
      this.off(eventType, handler);
    };
  }

  off(eventType: string, handler: MessageHandler): void {
    const handlers = this.handlers.get(eventType);
    if (handlers) {
      handlers.delete(handler);
    }
  }

  send(type: string, data: any): void {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify({ type, data }));
    } else {
      console.warn('WebSocket is not connected');
    }
  }

  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN;
  }
}

// Event types
export const WS_EVENTS = {
  LIVEVIEW_FRAME: 'liveview:frame',
  CAPTURE_COUNTDOWN: 'capture:countdown',
  CAPTURE_COMPLETE: 'capture:complete',
  PRINT_STATUS: 'print:status',
  CAMERA_STATUS: 'camera:status',
  ERROR: 'error',
} as const;

// Singleton instance
export const wsService = new WebSocketService();

// React hook for WebSocket
import { useEffect, useState, useCallback } from 'react';

export function useWebSocket() {
  const [isConnected, setIsConnected] = useState(false);

  useEffect(() => {
    wsService.connect().then(() => {
      setIsConnected(true);
    }).catch(() => {
      setIsConnected(false);
    });

    const checkConnection = setInterval(() => {
      setIsConnected(wsService.isConnected());
    }, 1000);

    return () => {
      clearInterval(checkConnection);
    };
  }, []);

  const subscribe = useCallback((eventType: string, handler: MessageHandler) => {
    return wsService.on(eventType, handler);
  }, []);

  const send = useCallback((type: string, data: any) => {
    wsService.send(type, data);
  }, []);

  return {
    isConnected,
    subscribe,
    send,
  };
}

// Hook for live view
export function useLiveView() {
  const [frame, setFrame] = useState<string | null>(null);
  const [isActive, setIsActive] = useState(false);
  const { subscribe, isConnected } = useWebSocket();

  useEffect(() => {
    if (!isConnected) return;

    const unsubscribe = subscribe(WS_EVENTS.LIVEVIEW_FRAME, (data) => {
      setFrame(data.imageData);
      setIsActive(true);
    });

    return () => {
      unsubscribe();
    };
  }, [isConnected, subscribe]);

  return {
    frame,
    isActive,
    isConnected,
  };
}

// Hook for capture events
export function useCaptureEvents() {
  const [countdown, setCountdown] = useState<number | null>(null);
  const [lastCapture, setLastCapture] = useState<any>(null);
  const { subscribe, isConnected } = useWebSocket();

  useEffect(() => {
    if (!isConnected) return;

    const unsubCountdown = subscribe(WS_EVENTS.CAPTURE_COUNTDOWN, (data) => {
      setCountdown(data.seconds);
    });

    const unsubComplete = subscribe(WS_EVENTS.CAPTURE_COMPLETE, (data) => {
      setCountdown(null);
      setLastCapture(data);
    });

    return () => {
      unsubCountdown();
      unsubComplete();
    };
  }, [isConnected, subscribe]);

  return {
    countdown,
    lastCapture,
    isConnected,
  };
}

export default wsService;
