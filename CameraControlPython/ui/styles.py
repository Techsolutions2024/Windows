"""
Modern Dark Theme Stylesheet for PyQt6
"""

DARK_STYLESHEET = """
/* Main Window */
QMainWindow {
    background-color: #1e1e1e;
}

QWidget {
    background-color: #1e1e1e;
    color: #e0e0e0;
    font-family: 'Segoe UI', Arial, sans-serif;
    font-size: 12px;
}

/* Group Box */
QGroupBox {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    margin-top: 12px;
    padding: 10px;
    font-weight: bold;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0 8px;
    color: #4fc3f7;
}

/* Buttons */
QPushButton {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    border-radius: 6px;
    padding: 8px 16px;
    min-width: 80px;
    color: #e0e0e0;
}

QPushButton:hover {
    background-color: #4d4d4d;
    border-color: #5d5d5d;
}

QPushButton:pressed {
    background-color: #2d2d2d;
}

QPushButton:disabled {
    background-color: #2d2d2d;
    color: #666666;
    border-color: #3d3d3d;
}

/* Primary Button */
QPushButton#primaryButton {
    background-color: #1976d2;
    border: none;
    color: white;
    font-weight: bold;
}

QPushButton#primaryButton:hover {
    background-color: #1e88e5;
}

QPushButton#primaryButton:pressed {
    background-color: #1565c0;
}

/* Capture Button */
QPushButton#captureButton {
    background-color: #d32f2f;
    border: none;
    color: white;
    font-weight: bold;
    min-width: 120px;
    min-height: 50px;
    border-radius: 25px;
    font-size: 14px;
}

QPushButton#captureButton:hover {
    background-color: #e53935;
}

QPushButton#captureButton:pressed {
    background-color: #c62828;
}

/* Record Button */
QPushButton#recordButton {
    background-color: #d32f2f;
    border: 3px solid #f44336;
    border-radius: 30px;
    min-width: 60px;
    min-height: 60px;
}

QPushButton#recordButton:checked {
    background-color: #b71c1c;
    border-color: #d32f2f;
}

/* ComboBox */
QComboBox {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    border-radius: 6px;
    padding: 6px 12px;
    min-width: 100px;
    color: #e0e0e0;
}

QComboBox:hover {
    border-color: #5d5d5d;
}

QComboBox::drop-down {
    border: none;
    width: 30px;
}

QComboBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid #e0e0e0;
    margin-right: 10px;
}

QComboBox QAbstractItemView {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    selection-background-color: #1976d2;
    selection-color: white;
}

/* Slider */
QSlider::groove:horizontal {
    border: none;
    height: 6px;
    background: #3d3d3d;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    background: #1976d2;
    border: none;
    width: 18px;
    height: 18px;
    margin: -6px 0;
    border-radius: 9px;
}

QSlider::handle:horizontal:hover {
    background: #1e88e5;
}

/* Progress Bar */
QProgressBar {
    border: none;
    border-radius: 4px;
    background-color: #3d3d3d;
    height: 8px;
    text-align: center;
}

QProgressBar::chunk {
    background-color: #4caf50;
    border-radius: 4px;
}

/* Labels */
QLabel {
    color: #e0e0e0;
    background-color: transparent;
}

QLabel#titleLabel {
    font-size: 18px;
    font-weight: bold;
    color: #4fc3f7;
}

QLabel#valueLabel {
    font-size: 14px;
    font-weight: bold;
    color: #fff;
}

QLabel#statusLabel {
    font-size: 11px;
    color: #888888;
}

/* Tab Widget */
QTabWidget::pane {
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    background-color: #2d2d2d;
}

QTabBar::tab {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 8px 20px;
    margin-right: 2px;
    color: #888888;
}

QTabBar::tab:selected {
    background-color: #2d2d2d;
    color: #4fc3f7;
    border-bottom: 2px solid #4fc3f7;
}

QTabBar::tab:hover:!selected {
    background-color: #4d4d4d;
    color: #e0e0e0;
}

/* Scroll Area */
QScrollArea {
    border: none;
    background-color: transparent;
}

QScrollBar:vertical {
    background-color: #2d2d2d;
    width: 12px;
    border-radius: 6px;
}

QScrollBar::handle:vertical {
    background-color: #4d4d4d;
    border-radius: 6px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #5d5d5d;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

/* List Widget */
QListWidget {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
    padding: 4px;
}

QListWidget::item {
    padding: 8px;
    border-radius: 4px;
}

QListWidget::item:hover {
    background-color: #3d3d3d;
}

QListWidget::item:selected {
    background-color: #1976d2;
    color: white;
}

/* Tree Widget */
QTreeWidget {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
}

QTreeWidget::item {
    padding: 4px;
}

QTreeWidget::item:hover {
    background-color: #3d3d3d;
}

QTreeWidget::item:selected {
    background-color: #1976d2;
}

/* Status Bar */
QStatusBar {
    background-color: #2d2d2d;
    border-top: 1px solid #3d3d3d;
    color: #888888;
}

/* Menu Bar */
QMenuBar {
    background-color: #2d2d2d;
    border-bottom: 1px solid #3d3d3d;
}

QMenuBar::item {
    padding: 6px 12px;
    background-color: transparent;
}

QMenuBar::item:selected {
    background-color: #3d3d3d;
}

QMenu {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
}

QMenu::item {
    padding: 8px 30px;
}

QMenu::item:selected {
    background-color: #1976d2;
}

/* Spin Box */
QSpinBox, QDoubleSpinBox {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    border-radius: 6px;
    padding: 6px;
    color: #e0e0e0;
}

/* Check Box */
QCheckBox {
    spacing: 8px;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 2px solid #4d4d4d;
    background-color: #3d3d3d;
}

QCheckBox::indicator:checked {
    background-color: #1976d2;
    border-color: #1976d2;
}

QCheckBox::indicator:hover {
    border-color: #1976d2;
}

/* Tool Tip */
QToolTip {
    background-color: #3d3d3d;
    border: 1px solid #4d4d4d;
    border-radius: 4px;
    padding: 6px;
    color: #e0e0e0;
}

/* Splitter */
QSplitter::handle {
    background-color: #3d3d3d;
}

QSplitter::handle:horizontal {
    width: 4px;
}

QSplitter::handle:vertical {
    height: 4px;
}
"""

LIGHT_STYLESHEET = """
/* Light theme - similar structure with light colors */
QMainWindow {
    background-color: #f5f5f5;
}

QWidget {
    background-color: #f5f5f5;
    color: #333333;
    font-family: 'Segoe UI', Arial, sans-serif;
    font-size: 12px;
}

QGroupBox {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    margin-top: 12px;
    padding: 10px;
    font-weight: bold;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0 8px;
    color: #1976d2;
}

QPushButton {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 6px;
    padding: 8px 16px;
    min-width: 80px;
    color: #333333;
}

QPushButton:hover {
    background-color: #f0f0f0;
    border-color: #d0d0d0;
}

QPushButton:pressed {
    background-color: #e0e0e0;
}

QPushButton#captureButton {
    background-color: #d32f2f;
    border: none;
    color: white;
    font-weight: bold;
    min-width: 120px;
    min-height: 50px;
    border-radius: 25px;
    font-size: 14px;
}

QComboBox {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 6px;
    padding: 6px 12px;
}

QLabel {
    color: #333333;
    background-color: transparent;
}
"""
