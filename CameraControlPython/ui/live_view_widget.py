"""
Live View Widget - Displays camera live view stream
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QFrame, QSizePolicy
)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal, QSize
from PyQt6.QtGui import QImage, QPixmap, QPainter, QPen, QColor
from typing import Optional
import io


class LiveViewWidget(QWidget):
    """
    Widget for displaying camera live view
    """
    clicked = pyqtSignal(int, int)  # Emits x, y coordinates when clicked

    def __init__(self, parent=None):
        super().__init__(parent)
        self._camera = None
        self._timer = QTimer(self)
        self._timer.timeout.connect(self._update_frame)
        self._current_image: Optional[QImage] = None
        self._show_grid = False
        self._show_focus_point = False
        self._focus_point = (0.5, 0.5)  # Normalized coordinates
        self._aspect_ratio = 3/2  # Default 3:2 aspect ratio

        self._setup_ui()

    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)

        # Image display frame
        self._image_frame = QFrame()
        self._image_frame.setFrameStyle(QFrame.Shape.StyledPanel)
        self._image_frame.setStyleSheet("""
            QFrame {
                background-color: #000000;
                border: 2px solid #3d3d3d;
                border-radius: 8px;
            }
        """)
        self._image_frame.setMinimumSize(640, 427)
        self._image_frame.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        # Image label inside frame
        frame_layout = QVBoxLayout(self._image_frame)
        frame_layout.setContentsMargins(2, 2, 2, 2)

        self._image_label = QLabel()
        self._image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._image_label.setStyleSheet("background-color: #000000; border: none;")
        self._image_label.setScaledContents(False)
        self._image_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self._image_label.mousePressEvent = self._on_image_click
        frame_layout.addWidget(self._image_label)

        layout.addWidget(self._image_frame)

        # Control buttons
        controls_layout = QHBoxLayout()
        controls_layout.setSpacing(8)

        self._btn_start = QPushButton("Start Live View")
        self._btn_start.setObjectName("primaryButton")
        self._btn_start.clicked.connect(self._on_start_clicked)

        self._btn_stop = QPushButton("Stop Live View")
        self._btn_stop.setEnabled(False)
        self._btn_stop.clicked.connect(self._on_stop_clicked)

        self._btn_grid = QPushButton("Grid")
        self._btn_grid.setCheckable(True)
        self._btn_grid.clicked.connect(self._toggle_grid)

        self._btn_focus_point = QPushButton("Focus Point")
        self._btn_focus_point.setCheckable(True)
        self._btn_focus_point.clicked.connect(self._toggle_focus_point)

        controls_layout.addWidget(self._btn_start)
        controls_layout.addWidget(self._btn_stop)
        controls_layout.addStretch()
        controls_layout.addWidget(self._btn_grid)
        controls_layout.addWidget(self._btn_focus_point)

        layout.addLayout(controls_layout)

        # Status label
        self._status_label = QLabel("Live View: Stopped")
        self._status_label.setObjectName("statusLabel")
        self._status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self._status_label)

    def set_camera(self, camera):
        """Set the camera to use"""
        self._camera = camera

    def start(self):
        """Start live view"""
        if not self._camera:
            return

        try:
            self._camera.start_live_view()
            self._timer.start(50)  # ~20 fps
            self._btn_start.setEnabled(False)
            self._btn_stop.setEnabled(True)
            self._status_label.setText("Live View: Running")
        except Exception as e:
            self._status_label.setText(f"Error: {str(e)}")

    def stop(self):
        """Stop live view"""
        self._timer.stop()

        if self._camera:
            try:
                self._camera.stop_live_view()
            except:
                pass

        self._btn_start.setEnabled(True)
        self._btn_stop.setEnabled(False)
        self._status_label.setText("Live View: Stopped")

        # Clear image
        self._image_label.clear()
        self._current_image = None

    def _update_frame(self):
        """Update the live view frame"""
        if not self._camera:
            return

        try:
            # Get live view image
            image_data = self._camera.get_live_view_image()

            if image_data:
                # Load JPEG data
                image = QImage()
                image.loadFromData(image_data)

                if not image.isNull():
                    self._current_image = image
                    self._display_image()

            # Process events
            self._camera.process_events()

        except Exception as e:
            print(f"Live view error: {e}")

    def _display_image(self):
        """Display the current image with overlays"""
        if self._current_image is None:
            return

        # Create a copy for drawing
        display_image = self._current_image.copy()

        # Draw overlays
        painter = QPainter(display_image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Draw grid
        if self._show_grid:
            self._draw_grid(painter, display_image.width(), display_image.height())

        # Draw focus point
        if self._show_focus_point:
            self._draw_focus_point(painter, display_image.width(), display_image.height())

        painter.end()

        # Scale to fit label while maintaining aspect ratio
        label_size = self._image_label.size()
        scaled_pixmap = QPixmap.fromImage(display_image).scaled(
            label_size,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        )

        self._image_label.setPixmap(scaled_pixmap)

    def _draw_grid(self, painter: QPainter, width: int, height: int):
        """Draw rule of thirds grid"""
        pen = QPen(QColor(255, 255, 255, 128))
        pen.setWidth(1)
        painter.setPen(pen)

        # Vertical lines
        x1 = width // 3
        x2 = 2 * width // 3
        painter.drawLine(x1, 0, x1, height)
        painter.drawLine(x2, 0, x2, height)

        # Horizontal lines
        y1 = height // 3
        y2 = 2 * height // 3
        painter.drawLine(0, y1, width, y1)
        painter.drawLine(0, y2, width, y2)

    def _draw_focus_point(self, painter: QPainter, width: int, height: int):
        """Draw focus point indicator"""
        x = int(self._focus_point[0] * width)
        y = int(self._focus_point[1] * height)
        size = 40

        pen = QPen(QColor(255, 0, 0, 200))
        pen.setWidth(2)
        painter.setPen(pen)

        # Draw crosshair
        painter.drawLine(x - size, y, x + size, y)
        painter.drawLine(x, y - size, x, y + size)

        # Draw rectangle
        painter.drawRect(x - size//2, y - size//2, size, size)

    def _on_image_click(self, event):
        """Handle click on image"""
        if not self._current_image:
            return

        # Get click position relative to image
        label_size = self._image_label.size()
        pixmap = self._image_label.pixmap()

        if pixmap is None:
            return

        # Calculate image position within label
        img_width = pixmap.width()
        img_height = pixmap.height()
        x_offset = (label_size.width() - img_width) // 2
        y_offset = (label_size.height() - img_height) // 2

        # Get click position relative to image
        click_x = event.pos().x() - x_offset
        click_y = event.pos().y() - y_offset

        if 0 <= click_x <= img_width and 0 <= click_y <= img_height:
            # Normalize to 0-1
            norm_x = click_x / img_width
            norm_y = click_y / img_height

            # Update focus point
            self._focus_point = (norm_x, norm_y)

            # Emit signal with original image coordinates
            orig_x = int(norm_x * self._current_image.width())
            orig_y = int(norm_y * self._current_image.height())
            self.clicked.emit(orig_x, orig_y)

    def _on_start_clicked(self):
        """Handle start button click"""
        self.start()

    def _on_stop_clicked(self):
        """Handle stop button click"""
        self.stop()

    def _toggle_grid(self, checked: bool):
        """Toggle grid overlay"""
        self._show_grid = checked
        if self._current_image:
            self._display_image()

    def _toggle_focus_point(self, checked: bool):
        """Toggle focus point indicator"""
        self._show_focus_point = checked
        if self._current_image:
            self._display_image()

    def set_focus_point(self, x: float, y: float):
        """Set focus point position (normalized 0-1)"""
        self._focus_point = (x, y)
        if self._current_image:
            self._display_image()

    @property
    def is_running(self) -> bool:
        """Check if live view is running"""
        return self._timer.isActive()

    def resizeEvent(self, event):
        """Handle resize"""
        super().resizeEvent(event)
        if self._current_image:
            self._display_image()
