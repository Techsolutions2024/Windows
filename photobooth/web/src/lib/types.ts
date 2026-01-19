export type SessionStatus = 'Created' | 'Capturing' | 'Ready' | 'Printed' | 'Error';

export type SessionPhoto = {
  fileName: string;
  contentType: string;
  sizeBytes: number;
  capturedAtUtc: string;
};

export type SessionRender = {
  fileName: string;
  contentType: string;
  sizeBytes: number;
  renderedAtUtc: string;
};

export type PrintJob = {
  fileName: string;
  printedAtUtc: string;
};

export type SessionDto = {
  id: string;
  createdAtUtc: string;
  status: SessionStatus;
  templateId?: string | null;
  photos: SessionPhoto[];
  render?: SessionRender | null;
  printJob?: PrintJob | null;
  errorMessage?: string | null;
};
