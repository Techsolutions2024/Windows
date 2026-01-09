"""
Canon Camera Class - High-level interface for camera operations
"""
import os
import time
import ctypes
from ctypes import c_void_p, c_uint32, byref
from typing import Optional, List, Callable, Tuple
from dataclasses import dataclass
from enum import Enum

from .edsdk_wrapper import EDSDK, EDSDKError
from .edsdk_types import (
    PropID, CameraCommand, CameraStatusCommand, SaveTo, ShutterButton,
    EvfOutputDevice, FileCreateDisposition, Access, EdsCapacity, EdsErrorCode,
    PropertyEvent, ObjectEvent, StateEvent,
    ISO_VALUES, AV_VALUES, TV_VALUES, WHITE_BALANCE_VALUES, AE_MODE_VALUES,
    METERING_MODE_VALUES, DRIVE_MODE_VALUES
)


@dataclass
class CameraInfo:
    """Camera device information"""
    name: str
    port: str
    ref: c_void_p


@dataclass
class VolumeInfo:
    """Memory card volume information"""
    label: str
    storage_type: int
    max_capacity: int
    free_space: int
    ref: c_void_p


@dataclass
class FileInfo:
    """File information"""
    name: str
    size: int
    is_folder: bool
    ref: c_void_p


class Camera:
    """
    High-level camera interface
    """
    def __init__(self, camera_ref: c_void_p):
        self._sdk = EDSDK()
        self._ref = camera_ref
        self._session_open = False
        self._evf_started = False
        self._save_to = SaveTo.Host
        self._download_path = os.path.join(os.path.expanduser("~"), "Pictures", "Canon")

        # Event callbacks
        self._on_property_changed: Optional[Callable] = None
        self._on_object_event: Optional[Callable] = None
        self._on_state_event: Optional[Callable] = None
        self._on_download_progress: Optional[Callable] = None
        self._on_image_downloaded: Optional[Callable] = None

        # Get device info
        info = self._sdk.get_device_info(camera_ref)
        self.name = info.szDeviceDescription.decode('utf-8', errors='ignore')
        self.port = info.szPortName.decode('utf-8', errors='ignore')

    @property
    def ref(self) -> c_void_p:
        return self._ref

    @property
    def is_connected(self) -> bool:
        return self._session_open

    # ========== Connection ==========

    def connect(self):
        """Open session with camera"""
        if self._session_open:
            return

        self._sdk.open_session(self._ref)
        self._session_open = True

        # Set up event handlers
        self._setup_event_handlers()

        # Set save destination to host
        self.set_save_to(self._save_to)

        # Set host capacity
        self._set_host_capacity()

    def disconnect(self):
        """Close session with camera"""
        if not self._session_open:
            return

        if self._evf_started:
            self.stop_live_view()

        self._sdk.close_session(self._ref)
        self._session_open = False

    def _setup_event_handlers(self):
        """Setup camera event handlers"""
        self._sdk.set_property_event_handler(
            self._ref, PropertyEvent.All, self._handle_property_event
        )
        self._sdk.set_object_event_handler(
            self._ref, ObjectEvent.All, self._handle_object_event
        )
        self._sdk.set_state_event_handler(
            self._ref, StateEvent.All, self._handle_state_event
        )

    def _set_host_capacity(self):
        """Set host computer capacity"""
        capacity = EdsCapacity()
        capacity.numberOfFreeClusters = 0x7FFFFFFF
        capacity.bytesPerSector = 0x1000
        capacity.reset = True
        self._sdk.set_capacity(self._ref, capacity)

    # ========== Event Handlers ==========

    def _handle_property_event(self, event, prop_id, param, context):
        """Handle property change events"""
        if self._on_property_changed:
            self._on_property_changed(event, prop_id, param)
        return EdsErrorCode.EDS_ERR_OK

    def _handle_object_event(self, event, obj_ref, context):
        """Handle object events (new files, etc.)"""
        if event == ObjectEvent.DirItemRequestTransfer:
            # Auto-download if save to host
            if self._save_to in (SaveTo.Host, SaveTo.Both):
                self._download_image(obj_ref)

        if self._on_object_event:
            self._on_object_event(event, obj_ref)
        return EdsErrorCode.EDS_ERR_OK

    def _handle_state_event(self, event, param, context):
        """Handle state events"""
        if self._on_state_event:
            self._on_state_event(event, param)
        return EdsErrorCode.EDS_ERR_OK

    def _download_image(self, dir_item: c_void_p):
        """Download image from camera"""
        try:
            # Get file info
            info = self._sdk.get_directory_item_info(dir_item)
            filename = info.szFileName.decode('utf-8', errors='ignore')
            size = info.size

            # Create download directory
            os.makedirs(self._download_path, exist_ok=True)
            filepath = os.path.join(self._download_path, filename)

            # Create file stream
            stream = self._sdk.create_file_stream(
                filepath, FileCreateDisposition.CreateAlways, Access.ReadWrite
            )

            # Download
            self._sdk.download(dir_item, size, stream)
            self._sdk.download_complete(dir_item)

            # Release stream
            self._sdk.release(stream)

            # Notify
            if self._on_image_downloaded:
                self._on_image_downloaded(filepath)

            print(f"Downloaded: {filepath}")

        except EDSDKError as e:
            print(f"Download error: {e}")

    # ========== Event Callbacks ==========

    def on_property_changed(self, callback: Callable):
        """Set property changed callback"""
        self._on_property_changed = callback

    def on_object_event(self, callback: Callable):
        """Set object event callback"""
        self._on_object_event = callback

    def on_state_event(self, callback: Callable):
        """Set state event callback"""
        self._on_state_event = callback

    def on_image_downloaded(self, callback: Callable):
        """Set image downloaded callback"""
        self._on_image_downloaded = callback

    # ========== Settings ==========

    def set_save_to(self, save_to: int):
        """Set where to save images"""
        self._save_to = save_to
        self._sdk.set_property_data(self._ref, PropID.SaveTo, save_to)
        if save_to in (SaveTo.Host, SaveTo.Both):
            self._set_host_capacity()

    def set_download_path(self, path: str):
        """Set download path"""
        self._download_path = path

    # ========== Properties - Getters ==========

    def get_product_name(self) -> str:
        """Get camera product name"""
        return self._sdk.get_property_string(self._ref, PropID.ProductName)

    def get_iso(self) -> int:
        """Get current ISO value"""
        return self._sdk.get_property_data(self._ref, PropID.ISOSpeed)

    def get_iso_text(self) -> str:
        """Get current ISO as text"""
        value = self.get_iso()
        return ISO_VALUES.get(value, str(value))

    def get_av(self) -> int:
        """Get current aperture value"""
        return self._sdk.get_property_data(self._ref, PropID.Av)

    def get_av_text(self) -> str:
        """Get current aperture as text"""
        value = self.get_av()
        return f"f/{AV_VALUES.get(value, str(value))}"

    def get_tv(self) -> int:
        """Get current shutter speed value"""
        return self._sdk.get_property_data(self._ref, PropID.Tv)

    def get_tv_text(self) -> str:
        """Get current shutter speed as text"""
        value = self.get_tv()
        return TV_VALUES.get(value, str(value))

    def get_exposure_compensation(self) -> int:
        """Get exposure compensation value"""
        return self._sdk.get_property_data(self._ref, PropID.ExposureCompensation)

    def get_white_balance(self) -> int:
        """Get white balance value"""
        return self._sdk.get_property_data(self._ref, PropID.WhiteBalance)

    def get_white_balance_text(self) -> str:
        """Get white balance as text"""
        value = self.get_white_balance()
        return WHITE_BALANCE_VALUES.get(value, str(value))

    def get_ae_mode(self) -> int:
        """Get AE mode value"""
        return self._sdk.get_property_data(self._ref, PropID.AEMode)

    def get_ae_mode_text(self) -> str:
        """Get AE mode as text"""
        value = self.get_ae_mode()
        return AE_MODE_VALUES.get(value, str(value))

    def get_drive_mode(self) -> int:
        """Get drive mode value"""
        return self._sdk.get_property_data(self._ref, PropID.DriveMode)

    def get_drive_mode_text(self) -> str:
        """Get drive mode as text"""
        value = self.get_drive_mode()
        return DRIVE_MODE_VALUES.get(value, str(value))

    def get_metering_mode(self) -> int:
        """Get metering mode value"""
        return self._sdk.get_property_data(self._ref, PropID.MeteringMode)

    def get_metering_mode_text(self) -> str:
        """Get metering mode as text"""
        value = self.get_metering_mode()
        return METERING_MODE_VALUES.get(value, str(value))

    def get_battery_level(self) -> int:
        """Get battery level (0-100 or 0xFFFFFFFF for AC)"""
        return self._sdk.get_property_data(self._ref, PropID.BatteryLevel)

    def get_available_shots(self) -> int:
        """Get number of available shots"""
        return self._sdk.get_property_data(self._ref, PropID.AvailableShots)

    def get_lens_name(self) -> str:
        """Get lens name"""
        return self._sdk.get_property_string(self._ref, PropID.LensName)

    # ========== Properties - Setters ==========

    def set_iso(self, value: int):
        """Set ISO value"""
        self._sdk.set_property_data(self._ref, PropID.ISOSpeed, value)

    def set_av(self, value: int):
        """Set aperture value"""
        self._sdk.set_property_data(self._ref, PropID.Av, value)

    def set_tv(self, value: int):
        """Set shutter speed value"""
        self._sdk.set_property_data(self._ref, PropID.Tv, value)

    def set_exposure_compensation(self, value: int):
        """Set exposure compensation value"""
        self._sdk.set_property_data(self._ref, PropID.ExposureCompensation, value)

    def set_white_balance(self, value: int):
        """Set white balance value"""
        self._sdk.set_property_data(self._ref, PropID.WhiteBalance, value)

    def set_ae_mode(self, value: int):
        """Set AE mode value"""
        self._sdk.set_property_data(self._ref, PropID.AEModeSelect, value)

    def set_drive_mode(self, value: int):
        """Set drive mode value"""
        self._sdk.set_property_data(self._ref, PropID.DriveMode, value)

    def set_metering_mode(self, value: int):
        """Set metering mode value"""
        self._sdk.set_property_data(self._ref, PropID.MeteringMode, value)

    # ========== Property Available Values ==========

    def get_available_iso(self) -> List[Tuple[int, str]]:
        """Get list of available ISO values"""
        desc = self._sdk.get_property_desc(self._ref, PropID.ISOSpeed)
        result = []
        for i in range(desc.numElements):
            value = desc.propDesc[i]
            text = ISO_VALUES.get(value, str(value))
            result.append((value, text))
        return result

    def get_available_av(self) -> List[Tuple[int, str]]:
        """Get list of available aperture values"""
        desc = self._sdk.get_property_desc(self._ref, PropID.Av)
        result = []
        for i in range(desc.numElements):
            value = desc.propDesc[i]
            text = f"f/{AV_VALUES.get(value, str(value))}"
            result.append((value, text))
        return result

    def get_available_tv(self) -> List[Tuple[int, str]]:
        """Get list of available shutter speed values"""
        desc = self._sdk.get_property_desc(self._ref, PropID.Tv)
        result = []
        for i in range(desc.numElements):
            value = desc.propDesc[i]
            text = TV_VALUES.get(value, str(value))
            result.append((value, text))
        return result

    def get_available_white_balance(self) -> List[Tuple[int, str]]:
        """Get list of available white balance values"""
        desc = self._sdk.get_property_desc(self._ref, PropID.WhiteBalance)
        result = []
        for i in range(desc.numElements):
            value = desc.propDesc[i]
            text = WHITE_BALANCE_VALUES.get(value, str(value))
            result.append((value, text))
        return result

    # ========== Capture Commands ==========

    def take_picture(self):
        """Take a single picture"""
        self._sdk.send_command(
            self._ref, CameraCommand.PressShutterButton,
            ShutterButton.Completely
        )
        self._sdk.send_command(
            self._ref, CameraCommand.PressShutterButton,
            ShutterButton.OFF
        )

    def press_shutter_halfway(self):
        """Press shutter button halfway (for AF)"""
        self._sdk.send_command(
            self._ref, CameraCommand.PressShutterButton,
            ShutterButton.Halfway
        )

    def press_shutter_completely(self):
        """Press shutter button completely"""
        self._sdk.send_command(
            self._ref, CameraCommand.PressShutterButton,
            ShutterButton.Completely
        )

    def release_shutter(self):
        """Release shutter button"""
        self._sdk.send_command(
            self._ref, CameraCommand.PressShutterButton,
            ShutterButton.OFF
        )

    def start_bulb(self):
        """Start bulb exposure"""
        self._sdk.send_command(self._ref, CameraCommand.BulbStart, 0)

    def end_bulb(self):
        """End bulb exposure"""
        self._sdk.send_command(self._ref, CameraCommand.BulbEnd, 0)

    def do_evf_af(self, on: bool = True):
        """Start/stop EVF autofocus"""
        self._sdk.send_command(self._ref, CameraCommand.DoEvfAf, 1 if on else 0)

    def drive_lens(self, direction: int):
        """Drive lens focus (use DriveLens constants)"""
        self._sdk.send_command(self._ref, CameraCommand.DriveLensEvf, direction)

    # ========== Live View ==========

    def start_live_view(self):
        """Start live view (EVF)"""
        if self._evf_started:
            return

        # Set EVF output to PC
        device = EvfOutputDevice.PC
        self._sdk.set_property_data(self._ref, PropID.Evf_OutputDevice, device)
        self._evf_started = True

    def stop_live_view(self):
        """Stop live view"""
        if not self._evf_started:
            return

        # Turn off EVF
        self._sdk.set_property_data(self._ref, PropID.Evf_OutputDevice, 0)
        self._evf_started = False

    def get_live_view_image(self) -> Optional[bytes]:
        """Get live view image as JPEG bytes"""
        if not self._evf_started:
            return None

        try:
            # Create memory stream
            stream = self._sdk.create_memory_stream(0)

            # Create EVF image ref
            evf_image = self._sdk.create_evf_image_ref(stream)

            # Download EVF image
            if not self._sdk.download_evf_image(self._ref, evf_image):
                self._sdk.release(evf_image)
                self._sdk.release(stream)
                return None

            # Get image data
            length = self._sdk.get_length(stream)
            pointer = self._sdk.get_pointer(stream)

            # Copy to bytes
            data = ctypes.string_at(pointer, length)

            # Release
            self._sdk.release(evf_image)
            self._sdk.release(stream)

            return data

        except EDSDKError:
            return None

    @property
    def is_live_view_active(self) -> bool:
        return self._evf_started

    # ========== Video Recording ==========

    def start_movie_mode(self):
        """Switch to movie mode"""
        self._sdk.send_command(self._ref, CameraCommand.MovieSelectSwON, 0)

    def stop_movie_mode(self):
        """Switch to still mode"""
        self._sdk.send_command(self._ref, CameraCommand.MovieSelectSwOFF, 0)

    def start_recording(self):
        """Start video recording"""
        self._sdk.set_property_data(self._ref, PropID.Record, 4)

    def stop_recording(self):
        """Stop video recording"""
        self._sdk.set_property_data(self._ref, PropID.Record, 0)

    # ========== Storage ==========

    def get_volumes(self) -> List[VolumeInfo]:
        """Get list of memory card volumes"""
        volumes = []
        count = self._sdk.get_child_count(self._ref)

        for i in range(count):
            vol_ref = self._sdk.get_child_at_index(self._ref, i)
            try:
                info = self._sdk.get_volume_info(vol_ref)
                if info.storageType != 0:  # Not empty
                    volumes.append(VolumeInfo(
                        label=info.szVolumeLabel.decode('utf-8', errors='ignore'),
                        storage_type=info.storageType,
                        max_capacity=info.maxCapacity,
                        free_space=info.freeSpaceInBytes,
                        ref=vol_ref
                    ))
            except EDSDKError:
                pass

        return volumes

    def get_files(self, volume: VolumeInfo, folder_path: str = "") -> List[FileInfo]:
        """Get list of files in a volume/folder"""
        files = []
        parent = volume.ref

        # Navigate to folder if specified
        if folder_path:
            parts = folder_path.strip("/").split("/")
            for part in parts:
                count = self._sdk.get_child_count(parent)
                for i in range(count):
                    child = self._sdk.get_child_at_index(parent, i)
                    info = self._sdk.get_directory_item_info(child)
                    name = info.szFileName.decode('utf-8', errors='ignore')
                    if name == part and info.isFolder:
                        parent = child
                        break

        # Get files
        count = self._sdk.get_child_count(parent)
        for i in range(count):
            item_ref = self._sdk.get_child_at_index(parent, i)
            info = self._sdk.get_directory_item_info(item_ref)
            files.append(FileInfo(
                name=info.szFileName.decode('utf-8', errors='ignore'),
                size=info.size,
                is_folder=info.isFolder,
                ref=item_ref
            ))

        return files

    def download_file(self, file: FileInfo, save_path: str):
        """Download a specific file"""
        stream = self._sdk.create_file_stream(
            save_path, FileCreateDisposition.CreateAlways, Access.ReadWrite
        )
        self._sdk.download(file.ref, file.size, stream)
        self._sdk.download_complete(file.ref)
        self._sdk.release(stream)

    def delete_file(self, file: FileInfo):
        """Delete a file from camera"""
        self._sdk.delete_directory_item(file.ref)

    # ========== UI Lock ==========

    def ui_lock(self):
        """Lock camera UI"""
        self._sdk.send_status_command(self._ref, CameraStatusCommand.UILock, 1)

    def ui_unlock(self):
        """Unlock camera UI"""
        self._sdk.send_status_command(self._ref, CameraStatusCommand.UIUnLock, 1)

    # ========== Event Processing ==========

    def process_events(self):
        """Process pending events (call in main loop)"""
        self._sdk.get_event()

    # ========== Cleanup ==========

    def __del__(self):
        """Cleanup on destruction"""
        if self._session_open:
            try:
                self.disconnect()
            except:
                pass


class CameraManager:
    """
    Manager for discovering and managing cameras
    """
    def __init__(self):
        self._sdk = EDSDK()
        self._initialized = False
        self._cameras: List[Camera] = []

    def initialize(self) -> bool:
        """Initialize SDK"""
        if self._initialized:
            return True
        self._initialized = self._sdk.initialize()
        return self._initialized

    def terminate(self):
        """Terminate SDK"""
        # Disconnect all cameras
        for camera in self._cameras:
            try:
                camera.disconnect()
            except:
                pass
        self._cameras.clear()

        if self._initialized:
            self._sdk.terminate()
            self._initialized = False

    def get_camera_list(self) -> List[CameraInfo]:
        """Get list of connected cameras"""
        if not self._initialized:
            return []

        cameras = []
        camera_list = self._sdk.get_camera_list()
        count = self._sdk.get_child_count(camera_list)

        for i in range(count):
            camera_ref = self._sdk.get_child_at_index(camera_list, i)
            info = self._sdk.get_device_info(camera_ref)
            cameras.append(CameraInfo(
                name=info.szDeviceDescription.decode('utf-8', errors='ignore'),
                port=info.szPortName.decode('utf-8', errors='ignore'),
                ref=camera_ref
            ))

        self._sdk.release(camera_list)
        return cameras

    def get_camera(self, index: int = 0) -> Optional[Camera]:
        """Get camera by index"""
        camera_list = self._sdk.get_camera_list()
        count = self._sdk.get_child_count(camera_list)

        if index >= count:
            self._sdk.release(camera_list)
            return None

        camera_ref = self._sdk.get_child_at_index(camera_list, index)
        self._sdk.release(camera_list)

        camera = Camera(camera_ref)
        self._cameras.append(camera)
        return camera

    def __enter__(self):
        self.initialize()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.terminate()
