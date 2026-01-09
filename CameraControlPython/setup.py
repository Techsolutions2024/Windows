"""
Setup script for creating standalone executable
"""
import sys
from cx_Freeze import setup, Executable

# Dependencies
build_exe_options = {
    "packages": ["PyQt6", "ctypes"],
    "excludes": ["tkinter", "unittest"],
    "include_files": [
        ("../EDSDK_64/Dll/", "EDSDK_64/Dll/"),
        ("../EDSDK/Dll/", "EDSDK/Dll/"),
    ],
}

# GUI application (no console)
base = "Win32GUI" if sys.platform == "win32" else None

setup(
    name="Canon Camera Control",
    version="1.0.0",
    description="Python GUI for Canon Camera Control",
    options={"build_exe": build_exe_options},
    executables=[
        Executable(
            "main.py",
            base=base,
            target_name="CanonCameraControl.exe",
            icon=None,  # Add icon path here if desired
        )
    ],
)
