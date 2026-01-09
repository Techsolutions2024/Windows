"""
Settings Panel - Camera settings controls
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGridLayout, QLabel,
    QComboBox, QGroupBox, QPushButton, QSlider, QFrame,
    QScrollArea, QSizePolicy
)
from PyQt6.QtCore import Qt, pyqtSignal
from typing import Optional, List, Tuple
import sys
sys.path.append('..')
from edsdk.edsdk_types import (
    PropID, ISO_VALUES, AV_VALUES, TV_VALUES,
    WHITE_BALANCE_VALUES, AE_MODE_VALUES, METERING_MODE_VALUES, DRIVE_MODE_VALUES
)


class SettingComboBox(QWidget):
    """
    A labeled combo box for camera settings
    """
    value_changed = pyqtSignal(int)

    def __init__(self, label: str, parent=None):
        super().__init__(parent)
        self._values = []

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)

        # Label
        self._label = QLabel(label)
        self._label.setStyleSheet("font-weight: bold; color: #888;")
        layout.addWidget(self._label)

        # Combo box
        self._combo = QComboBox()
        self._combo.setMinimumWidth(120)
        self._combo.currentIndexChanged.connect(self._on_changed)
        layout.addWidget(self._combo)

        # Value display
        self._value_label = QLabel()
        self._value_label.setObjectName("valueLabel")
        self._value_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self._value_label)

    def set_items(self, items: List[Tuple[int, str]]):
        """Set combo box items [(value, text), ...]"""
        self._combo.blockSignals(True)
        self._combo.clear()
        self._values = []

        for value, text in items:
            self._combo.addItem(text)
            self._values.append(value)

        self._combo.blockSignals(False)

    def set_value(self, value: int):
        """Set current value"""
        if value in self._values:
            index = self._values.index(value)
            self._combo.setCurrentIndex(index)
            self._value_label.setText(self._combo.currentText())

    def get_value(self) -> int:
        """Get current value"""
        index = self._combo.currentIndex()
        if 0 <= index < len(self._values):
            return self._values[index]
        return 0

    def _on_changed(self, index: int):
        """Handle selection change"""
        if 0 <= index < len(self._values):
            self._value_label.setText(self._combo.currentText())
            self.value_changed.emit(self._values[index])

    def setEnabled(self, enabled: bool):
        """Enable/disable the control"""
        self._combo.setEnabled(enabled)


class SettingsPanel(QWidget):
    """
    Panel for camera settings controls
    """
    iso_changed = pyqtSignal(int)
    av_changed = pyqtSignal(int)
    tv_changed = pyqtSignal(int)
    wb_changed = pyqtSignal(int)
    exp_comp_changed = pyqtSignal(int)
    metering_changed = pyqtSignal(int)
    drive_changed = pyqtSignal(int)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._camera = None
        self._setup_ui()

    def _setup_ui(self):
        """Setup the UI"""
        # Create scroll area
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll.setStyleSheet("QScrollArea { border: none; }")

        # Main container
        container = QWidget()
        layout = QVBoxLayout(container)
        layout.setSpacing(16)

        # === Camera Info Section ===
        info_group = QGroupBox("Camera")
        info_layout = QVBoxLayout(info_group)

        self._camera_name = QLabel("Not Connected")
        self._camera_name.setObjectName("titleLabel")
        info_layout.addWidget(self._camera_name)

        self._lens_name = QLabel("No Lens")
        self._lens_name.setObjectName("statusLabel")
        info_layout.addWidget(self._lens_name)

        # Status row
        status_layout = QHBoxLayout()

        self._battery_label = QLabel("Battery: --")
        status_layout.addWidget(self._battery_label)

        self._shots_label = QLabel("Shots: --")
        status_layout.addWidget(self._shots_label)

        info_layout.addLayout(status_layout)
        layout.addWidget(info_group)

        # === Exposure Settings ===
        exposure_group = QGroupBox("Exposure")
        exposure_layout = QGridLayout(exposure_group)
        exposure_layout.setSpacing(12)

        # AE Mode
        self._ae_mode = SettingComboBox("Mode")
        self._ae_mode.value_changed.connect(lambda v: self._on_setting_changed('ae_mode', v))
        exposure_layout.addWidget(self._ae_mode, 0, 0)

        # Shutter Speed
        self._tv = SettingComboBox("Shutter")
        self._tv.value_changed.connect(self.tv_changed.emit)
        exposure_layout.addWidget(self._tv, 0, 1)

        # Aperture
        self._av = SettingComboBox("Aperture")
        self._av.value_changed.connect(self.av_changed.emit)
        exposure_layout.addWidget(self._av, 0, 2)

        # ISO
        self._iso = SettingComboBox("ISO")
        self._iso.value_changed.connect(self.iso_changed.emit)
        exposure_layout.addWidget(self._iso, 1, 0)

        # Exposure Compensation
        self._exp_comp = SettingComboBox("Exp Comp")
        self._exp_comp.value_changed.connect(self.exp_comp_changed.emit)
        exposure_layout.addWidget(self._exp_comp, 1, 1)

        # Metering
        self._metering = SettingComboBox("Metering")
        self._metering.value_changed.connect(self.metering_changed.emit)
        exposure_layout.addWidget(self._metering, 1, 2)

        layout.addWidget(exposure_group)

        # === Image Settings ===
        image_group = QGroupBox("Image")
        image_layout = QGridLayout(image_group)
        image_layout.setSpacing(12)

        # White Balance
        self._wb = SettingComboBox("White Balance")
        self._wb.value_changed.connect(self.wb_changed.emit)
        image_layout.addWidget(self._wb, 0, 0)

        # Drive Mode
        self._drive = SettingComboBox("Drive Mode")
        self._drive.value_changed.connect(self.drive_changed.emit)
        image_layout.addWidget(self._drive, 0, 1)

        layout.addWidget(image_group)

        # === Focus Controls ===
        focus_group = QGroupBox("Focus")
        focus_layout = QVBoxLayout(focus_group)

        # Focus buttons
        focus_btns = QHBoxLayout()

        self._btn_focus_near3 = QPushButton("<<<")
        self._btn_focus_near3.setToolTip("Focus Near (Large)")
        self._btn_focus_near3.clicked.connect(lambda: self._drive_lens('near3'))
        focus_btns.addWidget(self._btn_focus_near3)

        self._btn_focus_near2 = QPushButton("<<")
        self._btn_focus_near2.setToolTip("Focus Near (Medium)")
        self._btn_focus_near2.clicked.connect(lambda: self._drive_lens('near2'))
        focus_btns.addWidget(self._btn_focus_near2)

        self._btn_focus_near1 = QPushButton("<")
        self._btn_focus_near1.setToolTip("Focus Near (Small)")
        self._btn_focus_near1.clicked.connect(lambda: self._drive_lens('near1'))
        focus_btns.addWidget(self._btn_focus_near1)

        focus_btns.addStretch()

        self._btn_af = QPushButton("AF")
        self._btn_af.setObjectName("primaryButton")
        self._btn_af.setToolTip("Auto Focus")
        self._btn_af.clicked.connect(self._do_af)
        focus_btns.addWidget(self._btn_af)

        focus_btns.addStretch()

        self._btn_focus_far1 = QPushButton(">")
        self._btn_focus_far1.setToolTip("Focus Far (Small)")
        self._btn_focus_far1.clicked.connect(lambda: self._drive_lens('far1'))
        focus_btns.addWidget(self._btn_focus_far1)

        self._btn_focus_far2 = QPushButton(">>")
        self._btn_focus_far2.setToolTip("Focus Far (Medium)")
        self._btn_focus_far2.clicked.connect(lambda: self._drive_lens('far2'))
        focus_btns.addWidget(self._btn_focus_far2)

        self._btn_focus_far3 = QPushButton(">>>")
        self._btn_focus_far3.setToolTip("Focus Far (Large)")
        self._btn_focus_far3.clicked.connect(lambda: self._drive_lens('far3'))
        focus_btns.addWidget(self._btn_focus_far3)

        focus_layout.addLayout(focus_btns)
        layout.addWidget(focus_group)

        # Add stretch at bottom
        layout.addStretch()

        # Set scroll area content
        scroll.setWidget(container)

        # Main layout
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.addWidget(scroll)

    def set_camera(self, camera):
        """Set the camera and update controls"""
        self._camera = camera
        self._update_camera_info()
        self._update_available_values()
        self._update_current_values()

    def _update_camera_info(self):
        """Update camera info display"""
        if not self._camera:
            self._camera_name.setText("Not Connected")
            self._lens_name.setText("No Lens")
            self._battery_label.setText("Battery: --")
            self._shots_label.setText("Shots: --")
            return

        self._camera_name.setText(self._camera.name)

        try:
            self._lens_name.setText(self._camera.get_lens_name())
        except:
            self._lens_name.setText("Unknown Lens")

        try:
            battery = self._camera.get_battery_level()
            if battery == 0xFFFFFFFF:
                self._battery_label.setText("Battery: AC")
            else:
                self._battery_label.setText(f"Battery: {battery}%")
        except:
            self._battery_label.setText("Battery: --")

        try:
            shots = self._camera.get_available_shots()
            self._shots_label.setText(f"Shots: {shots}")
        except:
            self._shots_label.setText("Shots: --")

    def _update_available_values(self):
        """Update available values for all controls"""
        if not self._camera:
            return

        try:
            # AE Mode - usually not settable, just display
            mode = self._camera.get_ae_mode()
            mode_text = AE_MODE_VALUES.get(mode, str(mode))
            self._ae_mode.set_items([(mode, mode_text)])
            self._ae_mode.setEnabled(False)
        except:
            pass

        try:
            self._iso.set_items(self._camera.get_available_iso())
        except:
            pass

        try:
            self._av.set_items(self._camera.get_available_av())
        except:
            pass

        try:
            self._tv.set_items(self._camera.get_available_tv())
        except:
            pass

        try:
            self._wb.set_items(self._camera.get_available_white_balance())
        except:
            pass

        # Static values for metering and drive mode
        self._metering.set_items(list(METERING_MODE_VALUES.items()))
        self._drive.set_items(list(DRIVE_MODE_VALUES.items()))

        # Exposure compensation (static range)
        exp_comp_values = []
        for i in range(-24, 25, 3):  # -3 to +3 in 1/3 steps
            value = 0x10 + i if i >= 0 else 0x10 + i
            text = f"{i/8:+.1f}" if i != 0 else "0"
            exp_comp_values.append((value, text))
        self._exp_comp.set_items(exp_comp_values)

    def _update_current_values(self):
        """Update current values for all controls"""
        if not self._camera:
            return

        try:
            self._ae_mode.set_value(self._camera.get_ae_mode())
        except:
            pass

        try:
            self._iso.set_value(self._camera.get_iso())
        except:
            pass

        try:
            self._av.set_value(self._camera.get_av())
        except:
            pass

        try:
            self._tv.set_value(self._camera.get_tv())
        except:
            pass

        try:
            self._wb.set_value(self._camera.get_white_balance())
        except:
            pass

        try:
            self._metering.set_value(self._camera.get_metering_mode())
        except:
            pass

        try:
            self._drive.set_value(self._camera.get_drive_mode())
        except:
            pass

    def _on_setting_changed(self, setting: str, value: int):
        """Handle setting change"""
        if not self._camera:
            return

        try:
            if setting == 'ae_mode':
                self._camera.set_ae_mode(value)
        except Exception as e:
            print(f"Failed to set {setting}: {e}")

    def _drive_lens(self, direction: str):
        """Drive lens focus"""
        if not self._camera:
            return

        from edsdk.edsdk_types import DriveLens

        directions = {
            'near1': DriveLens.Near1,
            'near2': DriveLens.Near2,
            'near3': DriveLens.Near3,
            'far1': DriveLens.Far1,
            'far2': DriveLens.Far2,
            'far3': DriveLens.Far3,
        }

        try:
            self._camera.drive_lens(directions[direction])
        except Exception as e:
            print(f"Failed to drive lens: {e}")

    def _do_af(self):
        """Execute autofocus"""
        if not self._camera:
            return

        try:
            self._camera.do_evf_af(True)
        except Exception as e:
            print(f"Failed to autofocus: {e}")

    def refresh(self):
        """Refresh all values"""
        self._update_camera_info()
        self._update_current_values()
