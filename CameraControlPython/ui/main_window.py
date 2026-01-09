"""
Main Window - Main application window
"""
from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QSplitter,
    QTabWidget, QPushButton, QLabel, QStatusBar, QMessageBox,
    QFrame, QMenuBar, QMenu, QFileDialog, QGroupBox
)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtGui import QAction, QKeySequence
from typing import Optional
import os
import sys

sys.path.append('..')
from .live_view_widget import LiveViewWidget
from .settings_panel import SettingsPanel
from .file_browser import FileBrowser
from .styles import DARK_STYLESHEET


class CapturePanel(QWidget):
    """Panel with capture controls"""
    capture_clicked = pyqtSignal()
    record_clicked = pyqtSignal(bool)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._is_recording = False
        self._setup_ui()

    def _setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(16)

        # Capture button
        self._btn_capture = QPushButton("CAPTURE")
        self._btn_capture.setObjectName("captureButton")
        self._btn_capture.setMinimumSize(150, 60)
        self._btn_capture.clicked.connect(self.capture_clicked.emit)
        layout.addWidget(self._btn_capture, alignment=Qt.AlignmentFlag.AlignCenter)

        # Shutter controls
        shutter_group = QGroupBox("Shutter Control")
        shutter_layout = QHBoxLayout(shutter_group)

        self._btn_half = QPushButton("Half Press")
        self._btn_half.setToolTip("Press shutter halfway (Focus)")
        shutter_layout.addWidget(self._btn_half)

        self._btn_full = QPushButton("Full Press")
        self._btn_full.setToolTip("Press shutter completely")
        shutter_layout.addWidget(self._btn_full)

        self._btn_release = QPushButton("Release")
        self._btn_release.setToolTip("Release shutter button")
        shutter_layout.addWidget(self._btn_release)

        layout.addWidget(shutter_group)

        # Video controls
        video_group = QGroupBox("Video")
        video_layout = QHBoxLayout(video_group)

        self._btn_movie_mode = QPushButton("Movie Mode")
        self._btn_movie_mode.setCheckable(True)
        self._btn_movie_mode.setToolTip("Switch to movie mode")
        video_layout.addWidget(self._btn_movie_mode)

        self._btn_record = QPushButton("REC")
        self._btn_record.setObjectName("recordButton")
        self._btn_record.setCheckable(True)
        self._btn_record.setMinimumSize(60, 60)
        self._btn_record.clicked.connect(self._on_record_clicked)
        video_layout.addWidget(self._btn_record)

        layout.addWidget(video_group)

        # Bulb controls
        bulb_group = QGroupBox("Bulb Mode")
        bulb_layout = QHBoxLayout(bulb_group)

        self._btn_bulb_start = QPushButton("Start Bulb")
        self._btn_bulb_start.setToolTip("Start bulb exposure")
        bulb_layout.addWidget(self._btn_bulb_start)

        self._btn_bulb_end = QPushButton("End Bulb")
        self._btn_bulb_end.setToolTip("End bulb exposure")
        bulb_layout.addWidget(self._btn_bulb_end)

        self._bulb_timer = QLabel("00:00")
        self._bulb_timer.setObjectName("valueLabel")
        bulb_layout.addWidget(self._bulb_timer)

        layout.addWidget(bulb_group)

        layout.addStretch()

    def _on_record_clicked(self, checked: bool):
        self._is_recording = checked
        if checked:
            self._btn_record.setStyleSheet("background-color: #b71c1c;")
        else:
            self._btn_record.setStyleSheet("")
        self.record_clicked.emit(checked)

    def get_shutter_buttons(self):
        return self._btn_half, self._btn_full, self._btn_release

    def get_bulb_buttons(self):
        return self._btn_bulb_start, self._btn_bulb_end

    def get_movie_button(self):
        return self._btn_movie_mode


class MainWindow(QMainWindow):
    """
    Main application window for Canon Camera Control
    """
    def __init__(self):
        super().__init__()
        self._camera = None
        self._camera_manager = None
        self._event_timer = QTimer(self)
        self._event_timer.timeout.connect(self._process_events)

        self._setup_ui()
        self._setup_menu()
        self._setup_shortcuts()
        self._connect_signals()

    def _setup_ui(self):
        """Setup the main UI"""
        self.setWindowTitle("Canon Camera Control")
        self.setMinimumSize(1200, 800)
        self.setStyleSheet(DARK_STYLESHEET)

        # Central widget
        central = QWidget()
        self.setCentralWidget(central)

        main_layout = QHBoxLayout(central)
        main_layout.setSpacing(12)
        main_layout.setContentsMargins(12, 12, 12, 12)

        # Create splitter for main content
        splitter = QSplitter(Qt.Orientation.Horizontal)

        # Left panel - Live View and Capture
        left_panel = QWidget()
        left_layout = QVBoxLayout(left_panel)
        left_layout.setSpacing(12)

        # Live View
        self._live_view = LiveViewWidget()
        left_layout.addWidget(self._live_view, stretch=3)

        # Capture Panel
        self._capture_panel = CapturePanel()
        left_layout.addWidget(self._capture_panel, stretch=1)

        splitter.addWidget(left_panel)

        # Right panel - Tabs for Settings and File Browser
        right_panel = QTabWidget()
        right_panel.setMinimumWidth(350)

        # Settings Tab
        self._settings_panel = SettingsPanel()
        right_panel.addTab(self._settings_panel, "Settings")

        # Files Tab
        self._file_browser = FileBrowser()
        right_panel.addTab(self._file_browser, "Files")

        splitter.addWidget(right_panel)

        # Set splitter sizes
        splitter.setSizes([800, 400])

        main_layout.addWidget(splitter)

        # Status bar
        self._status_bar = QStatusBar()
        self.setStatusBar(self._status_bar)
        self._status_bar.showMessage("Ready - Connect a camera to begin")

        # Connection status label
        self._conn_status = QLabel("Disconnected")
        self._conn_status.setStyleSheet("color: #f44336; font-weight: bold;")
        self._status_bar.addPermanentWidget(self._conn_status)

    def _setup_menu(self):
        """Setup menu bar"""
        menubar = self.menuBar()

        # File menu
        file_menu = menubar.addMenu("&File")

        connect_action = QAction("&Connect Camera", self)
        connect_action.setShortcut(QKeySequence("Ctrl+O"))
        connect_action.triggered.connect(self._connect_camera)
        file_menu.addAction(connect_action)

        disconnect_action = QAction("&Disconnect", self)
        disconnect_action.triggered.connect(self._disconnect_camera)
        file_menu.addAction(disconnect_action)

        file_menu.addSeparator()

        set_download_path = QAction("Set Download &Path...", self)
        set_download_path.triggered.connect(self._set_download_path)
        file_menu.addAction(set_download_path)

        file_menu.addSeparator()

        exit_action = QAction("E&xit", self)
        exit_action.setShortcut(QKeySequence("Alt+F4"))
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)

        # Camera menu
        camera_menu = menubar.addMenu("&Camera")

        capture_action = QAction("&Capture", self)
        capture_action.setShortcut(QKeySequence("Space"))
        capture_action.triggered.connect(self._capture)
        camera_menu.addAction(capture_action)

        camera_menu.addSeparator()

        start_lv_action = QAction("Start &Live View", self)
        start_lv_action.setShortcut(QKeySequence("Ctrl+L"))
        start_lv_action.triggered.connect(self._start_live_view)
        camera_menu.addAction(start_lv_action)

        stop_lv_action = QAction("Stop Li&ve View", self)
        stop_lv_action.triggered.connect(self._stop_live_view)
        camera_menu.addAction(stop_lv_action)

        # View menu
        view_menu = menubar.addMenu("&View")

        refresh_action = QAction("&Refresh Settings", self)
        refresh_action.setShortcut(QKeySequence("F5"))
        refresh_action.triggered.connect(self._refresh_settings)
        view_menu.addAction(refresh_action)

        # Help menu
        help_menu = menubar.addMenu("&Help")

        about_action = QAction("&About", self)
        about_action.triggered.connect(self._show_about)
        help_menu.addAction(about_action)

    def _setup_shortcuts(self):
        """Setup keyboard shortcuts"""
        pass  # Already set in menu actions

    def _connect_signals(self):
        """Connect widget signals"""
        # Capture panel
        self._capture_panel.capture_clicked.connect(self._capture)
        self._capture_panel.record_clicked.connect(self._toggle_recording)

        half_btn, full_btn, release_btn = self._capture_panel.get_shutter_buttons()
        half_btn.clicked.connect(self._press_shutter_halfway)
        full_btn.clicked.connect(self._press_shutter_completely)
        release_btn.clicked.connect(self._release_shutter)

        bulb_start, bulb_end = self._capture_panel.get_bulb_buttons()
        bulb_start.clicked.connect(self._start_bulb)
        bulb_end.clicked.connect(self._end_bulb)

        movie_btn = self._capture_panel.get_movie_button()
        movie_btn.clicked.connect(self._toggle_movie_mode)

        # Settings panel
        self._settings_panel.iso_changed.connect(self._set_iso)
        self._settings_panel.av_changed.connect(self._set_av)
        self._settings_panel.tv_changed.connect(self._set_tv)
        self._settings_panel.wb_changed.connect(self._set_wb)
        self._settings_panel.metering_changed.connect(self._set_metering)
        self._settings_panel.drive_changed.connect(self._set_drive)

        # Live view click
        self._live_view.clicked.connect(self._on_live_view_clicked)

    # ========== Camera Connection ==========

    def _connect_camera(self):
        """Connect to camera"""
        try:
            # Import here to avoid circular imports
            from edsdk import CameraManager

            if self._camera_manager is None:
                self._camera_manager = CameraManager()
                if not self._camera_manager.initialize():
                    QMessageBox.critical(self, "Error", "Failed to initialize SDK")
                    return

            # Get camera list
            cameras = self._camera_manager.get_camera_list()

            if not cameras:
                QMessageBox.information(
                    self, "No Camera",
                    "No camera detected. Please connect a Canon camera via USB."
                )
                return

            # Connect to first camera
            self._camera = self._camera_manager.get_camera(0)
            self._camera.connect()

            # Set up callbacks
            self._camera.on_image_downloaded(self._on_image_downloaded)

            # Update UI
            self._live_view.set_camera(self._camera)
            self._settings_panel.set_camera(self._camera)
            self._file_browser.set_camera(self._camera)

            # Update status
            self._conn_status.setText(f"Connected: {self._camera.name}")
            self._conn_status.setStyleSheet("color: #4caf50; font-weight: bold;")
            self._status_bar.showMessage(f"Connected to {self._camera.name}")

            # Start event processing
            self._event_timer.start(100)

        except Exception as e:
            QMessageBox.critical(self, "Connection Error", f"Failed to connect: {e}")

    def _disconnect_camera(self):
        """Disconnect from camera"""
        if self._live_view.is_running:
            self._live_view.stop()

        if self._camera:
            try:
                self._camera.disconnect()
            except:
                pass
            self._camera = None

        self._event_timer.stop()

        # Update status
        self._conn_status.setText("Disconnected")
        self._conn_status.setStyleSheet("color: #f44336; font-weight: bold;")
        self._status_bar.showMessage("Disconnected")

    def _process_events(self):
        """Process camera events"""
        if self._camera:
            try:
                self._camera.process_events()
            except:
                pass

    # ========== Camera Operations ==========

    def _capture(self):
        """Take a picture"""
        if not self._camera:
            self._status_bar.showMessage("No camera connected")
            return

        try:
            self._camera.take_picture()
            self._status_bar.showMessage("Capturing...")
        except Exception as e:
            self._status_bar.showMessage(f"Capture failed: {e}")

    def _press_shutter_halfway(self):
        """Press shutter halfway"""
        if self._camera:
            try:
                self._camera.press_shutter_halfway()
            except Exception as e:
                print(f"Error: {e}")

    def _press_shutter_completely(self):
        """Press shutter completely"""
        if self._camera:
            try:
                self._camera.press_shutter_completely()
            except Exception as e:
                print(f"Error: {e}")

    def _release_shutter(self):
        """Release shutter"""
        if self._camera:
            try:
                self._camera.release_shutter()
            except Exception as e:
                print(f"Error: {e}")

    def _start_bulb(self):
        """Start bulb exposure"""
        if self._camera:
            try:
                self._camera.start_bulb()
                self._status_bar.showMessage("Bulb exposure started")
            except Exception as e:
                print(f"Error: {e}")

    def _end_bulb(self):
        """End bulb exposure"""
        if self._camera:
            try:
                self._camera.end_bulb()
                self._status_bar.showMessage("Bulb exposure ended")
            except Exception as e:
                print(f"Error: {e}")

    def _toggle_movie_mode(self, checked: bool):
        """Toggle movie mode"""
        if self._camera:
            try:
                if checked:
                    self._camera.start_movie_mode()
                else:
                    self._camera.stop_movie_mode()
            except Exception as e:
                print(f"Error: {e}")

    def _toggle_recording(self, recording: bool):
        """Toggle video recording"""
        if self._camera:
            try:
                if recording:
                    self._camera.start_recording()
                    self._status_bar.showMessage("Recording...")
                else:
                    self._camera.stop_recording()
                    self._status_bar.showMessage("Recording stopped")
            except Exception as e:
                print(f"Error: {e}")

    def _start_live_view(self):
        """Start live view"""
        self._live_view.start()

    def _stop_live_view(self):
        """Stop live view"""
        self._live_view.stop()

    def _on_live_view_clicked(self, x: int, y: int):
        """Handle click on live view"""
        # Could be used for touch AF, etc.
        self._status_bar.showMessage(f"Clicked at ({x}, {y})")

    # ========== Settings ==========

    def _set_iso(self, value: int):
        if self._camera:
            try:
                self._camera.set_iso(value)
            except Exception as e:
                print(f"Error setting ISO: {e}")

    def _set_av(self, value: int):
        if self._camera:
            try:
                self._camera.set_av(value)
            except Exception as e:
                print(f"Error setting Av: {e}")

    def _set_tv(self, value: int):
        if self._camera:
            try:
                self._camera.set_tv(value)
            except Exception as e:
                print(f"Error setting Tv: {e}")

    def _set_wb(self, value: int):
        if self._camera:
            try:
                self._camera.set_white_balance(value)
            except Exception as e:
                print(f"Error setting WB: {e}")

    def _set_metering(self, value: int):
        if self._camera:
            try:
                self._camera.set_metering_mode(value)
            except Exception as e:
                print(f"Error setting metering: {e}")

    def _set_drive(self, value: int):
        if self._camera:
            try:
                self._camera.set_drive_mode(value)
            except Exception as e:
                print(f"Error setting drive mode: {e}")

    def _refresh_settings(self):
        """Refresh settings panel"""
        self._settings_panel.refresh()

    def _set_download_path(self):
        """Set download path"""
        path = QFileDialog.getExistingDirectory(
            self, "Select Download Path",
            os.path.expanduser("~/Pictures")
        )
        if path and self._camera:
            self._camera.set_download_path(path)
            self._status_bar.showMessage(f"Download path set to: {path}")

    # ========== Callbacks ==========

    def _on_image_downloaded(self, filepath: str):
        """Handle image downloaded"""
        self._status_bar.showMessage(f"Downloaded: {os.path.basename(filepath)}")

    # ========== Other ==========

    def _show_about(self):
        """Show about dialog"""
        QMessageBox.about(
            self, "About Canon Camera Control",
            "<h3>Canon Camera Control</h3>"
            "<p>Python application for controlling Canon cameras via EDSDK</p>"
            "<p>Features:</p>"
            "<ul>"
            "<li>Live View streaming</li>"
            "<li>Remote capture</li>"
            "<li>Full camera settings control</li>"
            "<li>File download and management</li>"
            "<li>Video recording</li>"
            "</ul>"
            "<p>Using Canon EDSDK 13.19.10</p>"
        )

    def closeEvent(self, event):
        """Handle window close"""
        # Stop live view
        if self._live_view.is_running:
            self._live_view.stop()

        # Disconnect camera
        self._disconnect_camera()

        # Terminate SDK
        if self._camera_manager:
            self._camera_manager.terminate()

        event.accept()
