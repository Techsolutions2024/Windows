"""
File Browser - Browse and manage files on camera memory cards
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QPushButton, QLabel, QProgressBar, QFileDialog, QMessageBox,
    QGroupBox, QSplitter, QFrame
)
from PyQt6.QtCore import Qt, pyqtSignal, QThread
from PyQt6.QtGui import QIcon
from typing import Optional, List
import os


class DownloadThread(QThread):
    """Thread for downloading files"""
    progress = pyqtSignal(int, int)  # current, total
    file_downloaded = pyqtSignal(str)
    finished_all = pyqtSignal()
    error = pyqtSignal(str)

    def __init__(self, camera, files, save_path):
        super().__init__()
        self._camera = camera
        self._files = files
        self._save_path = save_path
        self._cancelled = False

    def run(self):
        total = len(self._files)
        for i, file in enumerate(self._files):
            if self._cancelled:
                break

            try:
                filepath = os.path.join(self._save_path, file.name)
                self._camera.download_file(file, filepath)
                self.file_downloaded.emit(filepath)
            except Exception as e:
                self.error.emit(f"Failed to download {file.name}: {e}")

            self.progress.emit(i + 1, total)

        self.finished_all.emit()

    def cancel(self):
        self._cancelled = True


class FileBrowser(QWidget):
    """
    Widget for browsing and managing camera files
    """
    download_started = pyqtSignal()
    download_finished = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self._camera = None
        self._current_volume = None
        self._current_files = []
        self._download_thread = None
        self._setup_ui()

    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setSpacing(12)

        # Volume selector
        volume_group = QGroupBox("Memory Cards")
        volume_layout = QHBoxLayout(volume_group)

        self._btn_card1 = QPushButton("Card 1")
        self._btn_card1.setCheckable(True)
        self._btn_card1.clicked.connect(lambda: self._select_volume(0))
        volume_layout.addWidget(self._btn_card1)

        self._btn_card2 = QPushButton("Card 2")
        self._btn_card2.setCheckable(True)
        self._btn_card2.clicked.connect(lambda: self._select_volume(1))
        volume_layout.addWidget(self._btn_card2)

        volume_layout.addStretch()

        self._btn_refresh = QPushButton("Refresh")
        self._btn_refresh.clicked.connect(self._refresh_files)
        volume_layout.addWidget(self._btn_refresh)

        layout.addWidget(volume_group)

        # Volume info
        self._volume_info = QLabel("No card selected")
        self._volume_info.setObjectName("statusLabel")
        layout.addWidget(self._volume_info)

        # File tree
        self._tree = QTreeWidget()
        self._tree.setHeaderLabels(["Name", "Size", "Type"])
        self._tree.setColumnWidth(0, 250)
        self._tree.setColumnWidth(1, 100)
        self._tree.setSelectionMode(QTreeWidget.SelectionMode.ExtendedSelection)
        self._tree.itemDoubleClicked.connect(self._on_item_double_clicked)
        layout.addWidget(self._tree)

        # Action buttons
        actions_layout = QHBoxLayout()

        self._btn_download = QPushButton("Download Selected")
        self._btn_download.setObjectName("primaryButton")
        self._btn_download.clicked.connect(self._download_selected)
        actions_layout.addWidget(self._btn_download)

        self._btn_download_all = QPushButton("Download All")
        self._btn_download_all.clicked.connect(self._download_all)
        actions_layout.addWidget(self._btn_download_all)

        actions_layout.addStretch()

        self._btn_delete = QPushButton("Delete Selected")
        self._btn_delete.setStyleSheet("color: #f44336;")
        self._btn_delete.clicked.connect(self._delete_selected)
        actions_layout.addWidget(self._btn_delete)

        layout.addLayout(actions_layout)

        # Progress bar
        self._progress_frame = QFrame()
        self._progress_frame.setVisible(False)
        progress_layout = QVBoxLayout(self._progress_frame)

        self._progress_label = QLabel("Downloading...")
        progress_layout.addWidget(self._progress_label)

        self._progress_bar = QProgressBar()
        progress_layout.addWidget(self._progress_bar)

        self._btn_cancel = QPushButton("Cancel")
        self._btn_cancel.clicked.connect(self._cancel_download)
        progress_layout.addWidget(self._btn_cancel)

        layout.addWidget(self._progress_frame)

        # Status
        self._status_label = QLabel("Ready")
        self._status_label.setObjectName("statusLabel")
        layout.addWidget(self._status_label)

    def set_camera(self, camera):
        """Set the camera"""
        self._camera = camera
        self._load_volumes()

    def _load_volumes(self):
        """Load available volumes"""
        if not self._camera:
            return

        self._btn_card1.setEnabled(False)
        self._btn_card2.setEnabled(False)

        try:
            volumes = self._camera.get_volumes()

            if len(volumes) >= 1:
                vol = volumes[0]
                self._btn_card1.setEnabled(True)
                self._btn_card1.setText(f"Card 1: {vol.label}")
                self._volumes = volumes

            if len(volumes) >= 2:
                vol = volumes[1]
                self._btn_card2.setEnabled(True)
                self._btn_card2.setText(f"Card 2: {vol.label}")

        except Exception as e:
            self._status_label.setText(f"Error loading volumes: {e}")

    def _select_volume(self, index: int):
        """Select a volume"""
        if not hasattr(self, '_volumes') or index >= len(self._volumes):
            return

        self._current_volume = self._volumes[index]

        # Update button states
        self._btn_card1.setChecked(index == 0)
        self._btn_card2.setChecked(index == 1)

        # Update volume info
        vol = self._current_volume
        free_gb = vol.free_space / (1024 ** 3)
        total_gb = vol.max_capacity / (1024 ** 3)
        self._volume_info.setText(
            f"{vol.label} - {free_gb:.1f} GB free of {total_gb:.1f} GB"
        )

        self._refresh_files()

    def _refresh_files(self):
        """Refresh file list"""
        if not self._camera or not self._current_volume:
            return

        self._tree.clear()
        self._current_files = []

        try:
            self._load_folder(self._current_volume, "", self._tree.invisibleRootItem())
            self._status_label.setText(f"Found {len(self._current_files)} files")
        except Exception as e:
            self._status_label.setText(f"Error: {e}")

    def _load_folder(self, volume, path: str, parent_item: QTreeWidgetItem):
        """Load folder contents recursively"""
        try:
            files = self._camera.get_files(volume, path)

            for file in files:
                item = QTreeWidgetItem(parent_item)
                item.setText(0, file.name)
                item.setData(0, Qt.ItemDataRole.UserRole, file)

                if file.is_folder:
                    item.setText(2, "Folder")
                    # Load subfolder
                    new_path = f"{path}/{file.name}" if path else file.name
                    self._load_folder(volume, new_path, item)
                else:
                    size_kb = file.size / 1024
                    if size_kb > 1024:
                        item.setText(1, f"{size_kb/1024:.1f} MB")
                    else:
                        item.setText(1, f"{size_kb:.0f} KB")

                    # Determine file type
                    ext = os.path.splitext(file.name)[1].upper()
                    item.setText(2, ext[1:] if ext else "Unknown")

                    self._current_files.append(file)

        except Exception as e:
            print(f"Error loading folder {path}: {e}")

    def _on_item_double_clicked(self, item: QTreeWidgetItem, column: int):
        """Handle double click on item"""
        file = item.data(0, Qt.ItemDataRole.UserRole)
        if file and not file.is_folder:
            # Download single file
            self._download_files([file])

    def _get_selected_files(self):
        """Get list of selected files"""
        files = []
        for item in self._tree.selectedItems():
            file = item.data(0, Qt.ItemDataRole.UserRole)
            if file and not file.is_folder:
                files.append(file)
        return files

    def _download_selected(self):
        """Download selected files"""
        files = self._get_selected_files()
        if not files:
            QMessageBox.information(self, "Info", "No files selected")
            return
        self._download_files(files)

    def _download_all(self):
        """Download all files"""
        if not self._current_files:
            QMessageBox.information(self, "Info", "No files to download")
            return
        self._download_files(self._current_files)

    def _download_files(self, files: List):
        """Download files"""
        # Ask for save location
        save_path = QFileDialog.getExistingDirectory(
            self, "Select Download Location",
            os.path.expanduser("~/Pictures")
        )

        if not save_path:
            return

        # Start download thread
        self._download_thread = DownloadThread(self._camera, files, save_path)
        self._download_thread.progress.connect(self._on_download_progress)
        self._download_thread.file_downloaded.connect(self._on_file_downloaded)
        self._download_thread.finished_all.connect(self._on_download_finished)
        self._download_thread.error.connect(self._on_download_error)

        self._progress_frame.setVisible(True)
        self._progress_bar.setValue(0)
        self._progress_bar.setMaximum(len(files))
        self._progress_label.setText(f"Downloading 0/{len(files)} files...")

        self.download_started.emit()
        self._download_thread.start()

    def _on_download_progress(self, current: int, total: int):
        """Handle download progress"""
        self._progress_bar.setValue(current)
        self._progress_label.setText(f"Downloading {current}/{total} files...")

    def _on_file_downloaded(self, filepath: str):
        """Handle file downloaded"""
        self._status_label.setText(f"Downloaded: {os.path.basename(filepath)}")

    def _on_download_finished(self):
        """Handle download finished"""
        self._progress_frame.setVisible(False)
        self._status_label.setText("Download complete!")
        self.download_finished.emit()
        QMessageBox.information(self, "Complete", "Download finished!")

    def _on_download_error(self, error: str):
        """Handle download error"""
        self._status_label.setText(f"Error: {error}")

    def _cancel_download(self):
        """Cancel download"""
        if self._download_thread:
            self._download_thread.cancel()
            self._progress_frame.setVisible(False)
            self._status_label.setText("Download cancelled")

    def _delete_selected(self):
        """Delete selected files"""
        files = self._get_selected_files()
        if not files:
            QMessageBox.information(self, "Info", "No files selected")
            return

        reply = QMessageBox.question(
            self, "Confirm Delete",
            f"Delete {len(files)} file(s)? This cannot be undone.",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )

        if reply != QMessageBox.StandardButton.Yes:
            return

        deleted = 0
        for file in files:
            try:
                self._camera.delete_file(file)
                deleted += 1
            except Exception as e:
                print(f"Failed to delete {file.name}: {e}")

        self._status_label.setText(f"Deleted {deleted} file(s)")
        self._refresh_files()
