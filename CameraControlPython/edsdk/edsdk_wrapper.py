"""
Canon EDSDK Python Wrapper
Provides Python interface to Canon EDSDK DLLs
"""
import os
import sys
import ctypes
from ctypes import c_uint32, c_int32, c_void_p, c_char_p, byref, POINTER, c_uint64
from typing import Optional, Tuple, List, Callable
import platform

from .edsdk_types import (
    EdsError, EdsUInt32, EdsInt32, EdsBaseRef, EdsCameraRef, EdsCameraListRef,
    EdsVolumeRef, EdsDirectoryItemRef, EdsStreamRef, EdsImageRef, EdsEvfImageRef,
    EdsDeviceInfo, EdsVolumeInfo, EdsDirectoryItemInfo, EdsPoint, EdsSize, EdsRect,
    EdsCapacity, EdsPropertyDesc, EdsErrorCode,
    EdsPropertyEventHandler, EdsObjectEventHandler, EdsStateEventHandler,
    PropID, CameraCommand, CameraStatusCommand, SaveTo, FileCreateDisposition,
    Access, SeekOrigin, EvfOutputDevice
)


class EDSDKError(Exception):
    """Exception raised for EDSDK errors"""
    def __init__(self, error_code: int, message: str = ""):
        self.error_code = error_code
        self.message = message or f"EDSDK Error: 0x{error_code:08X}"
        super().__init__(self.message)


class EDSDK:
    """
    Python wrapper for Canon EDSDK
    """
    _instance = None
    _initialized = False

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if hasattr(self, '_dll') and self._dll is not None:
            return

        self._dll = None
        self._callbacks = []  # Keep references to prevent garbage collection
        self._load_dll()
        self._setup_functions()

    def _load_dll(self):
        """Load the EDSDK DLL"""
        # Determine DLL path based on architecture
        is_64bit = platform.architecture()[0] == '64bit'

        # Get the directory of this script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        base_dir = os.path.dirname(script_dir)

        if is_64bit:
            dll_dir = os.path.join(base_dir, "..", "EDSDK_64", "Dll")
            dll_name = "EDSDK.dll"
        else:
            dll_dir = os.path.join(base_dir, "..", "EDSDK", "Dll")
            dll_name = "EDSDK.dll"

        dll_path = os.path.normpath(os.path.join(dll_dir, dll_name))

        # Add DLL directory to PATH
        if os.path.exists(dll_dir):
            os.environ['PATH'] = dll_dir + os.pathsep + os.environ.get('PATH', '')
            if hasattr(os, 'add_dll_directory'):
                os.add_dll_directory(dll_dir)

        # Try to load the DLL
        try:
            self._dll = ctypes.WinDLL(dll_path)
            print(f"Loaded EDSDK from: {dll_path}")
        except OSError as e:
            # Try loading from current directory
            try:
                self._dll = ctypes.WinDLL(dll_name)
                print(f"Loaded EDSDK from PATH")
            except OSError:
                raise RuntimeError(f"Failed to load EDSDK.dll: {e}\nSearched: {dll_path}")

    def _setup_functions(self):
        """Setup function signatures"""
        dll = self._dll

        # Basic functions
        dll.EdsInitializeSDK.restype = EdsError
        dll.EdsInitializeSDK.argtypes = []

        dll.EdsTerminateSDK.restype = EdsError
        dll.EdsTerminateSDK.argtypes = []

        # Reference counter functions
        dll.EdsRetain.restype = EdsUInt32
        dll.EdsRetain.argtypes = [EdsBaseRef]

        dll.EdsRelease.restype = EdsUInt32
        dll.EdsRelease.argtypes = [EdsBaseRef]

        # Item-tree functions
        dll.EdsGetChildCount.restype = EdsError
        dll.EdsGetChildCount.argtypes = [EdsBaseRef, POINTER(EdsUInt32)]

        dll.EdsGetChildAtIndex.restype = EdsError
        dll.EdsGetChildAtIndex.argtypes = [EdsBaseRef, EdsInt32, POINTER(EdsBaseRef)]

        dll.EdsGetParent.restype = EdsError
        dll.EdsGetParent.argtypes = [EdsBaseRef, POINTER(EdsBaseRef)]

        # Camera list functions
        dll.EdsGetCameraList.restype = EdsError
        dll.EdsGetCameraList.argtypes = [POINTER(EdsCameraListRef)]

        # Device functions
        dll.EdsGetDeviceInfo.restype = EdsError
        dll.EdsGetDeviceInfo.argtypes = [EdsCameraRef, POINTER(EdsDeviceInfo)]

        # Session functions
        dll.EdsOpenSession.restype = EdsError
        dll.EdsOpenSession.argtypes = [EdsCameraRef]

        dll.EdsCloseSession.restype = EdsError
        dll.EdsCloseSession.argtypes = [EdsCameraRef]

        # Command functions
        dll.EdsSendCommand.restype = EdsError
        dll.EdsSendCommand.argtypes = [EdsCameraRef, EdsUInt32, EdsInt32]

        dll.EdsSendStatusCommand.restype = EdsError
        dll.EdsSendStatusCommand.argtypes = [EdsCameraRef, EdsUInt32, EdsInt32]

        # Property functions
        dll.EdsGetPropertySize.restype = EdsError
        dll.EdsGetPropertySize.argtypes = [EdsBaseRef, EdsUInt32, EdsInt32, POINTER(EdsUInt32), POINTER(EdsUInt32)]

        dll.EdsGetPropertyData.restype = EdsError
        dll.EdsGetPropertyData.argtypes = [EdsBaseRef, EdsUInt32, EdsInt32, EdsUInt32, c_void_p]

        dll.EdsSetPropertyData.restype = EdsError
        dll.EdsSetPropertyData.argtypes = [EdsBaseRef, EdsUInt32, EdsInt32, EdsUInt32, c_void_p]

        dll.EdsGetPropertyDesc.restype = EdsError
        dll.EdsGetPropertyDesc.argtypes = [EdsBaseRef, EdsUInt32, POINTER(EdsPropertyDesc)]

        # Volume functions
        dll.EdsGetVolumeInfo.restype = EdsError
        dll.EdsGetVolumeInfo.argtypes = [EdsVolumeRef, POINTER(EdsVolumeInfo)]

        dll.EdsFormatVolume.restype = EdsError
        dll.EdsFormatVolume.argtypes = [EdsVolumeRef]

        # Directory item functions
        dll.EdsGetDirectoryItemInfo.restype = EdsError
        dll.EdsGetDirectoryItemInfo.argtypes = [EdsDirectoryItemRef, POINTER(EdsDirectoryItemInfo)]

        dll.EdsDeleteDirectoryItem.restype = EdsError
        dll.EdsDeleteDirectoryItem.argtypes = [EdsDirectoryItemRef]

        # Download functions
        dll.EdsDownload.restype = EdsError
        dll.EdsDownload.argtypes = [EdsDirectoryItemRef, EdsUInt64, EdsStreamRef]

        dll.EdsDownloadComplete.restype = EdsError
        dll.EdsDownloadComplete.argtypes = [EdsDirectoryItemRef]

        dll.EdsDownloadCancel.restype = EdsError
        dll.EdsDownloadCancel.argtypes = [EdsDirectoryItemRef]

        dll.EdsDownloadThumbnail.restype = EdsError
        dll.EdsDownloadThumbnail.argtypes = [EdsDirectoryItemRef, EdsStreamRef]

        # Stream functions
        dll.EdsCreateFileStream.restype = EdsError
        dll.EdsCreateFileStream.argtypes = [c_char_p, EdsUInt32, EdsUInt32, POINTER(EdsStreamRef)]

        dll.EdsCreateMemoryStream.restype = EdsError
        dll.EdsCreateMemoryStream.argtypes = [EdsUInt64, POINTER(EdsStreamRef)]

        dll.EdsGetPointer.restype = EdsError
        dll.EdsGetPointer.argtypes = [EdsStreamRef, POINTER(c_void_p)]

        dll.EdsGetLength.restype = EdsError
        dll.EdsGetLength.argtypes = [EdsStreamRef, POINTER(EdsUInt64)]

        dll.EdsRead.restype = EdsError
        dll.EdsRead.argtypes = [EdsStreamRef, EdsUInt64, c_void_p, POINTER(EdsUInt64)]

        # Capacity functions
        dll.EdsSetCapacity.restype = EdsError
        dll.EdsSetCapacity.argtypes = [EdsCameraRef, EdsCapacity]

        # EVF functions
        dll.EdsCreateEvfImageRef.restype = EdsError
        dll.EdsCreateEvfImageRef.argtypes = [EdsStreamRef, POINTER(EdsEvfImageRef)]

        dll.EdsDownloadEvfImage.restype = EdsError
        dll.EdsDownloadEvfImage.argtypes = [EdsCameraRef, EdsEvfImageRef]

        # Event handler functions
        dll.EdsSetCameraAddedHandler.restype = EdsError
        dll.EdsSetCameraAddedHandler.argtypes = [c_void_p, c_void_p]

        dll.EdsSetPropertyEventHandler.restype = EdsError
        dll.EdsSetPropertyEventHandler.argtypes = [EdsCameraRef, EdsUInt32, c_void_p, c_void_p]

        dll.EdsSetObjectEventHandler.restype = EdsError
        dll.EdsSetObjectEventHandler.argtypes = [EdsCameraRef, EdsUInt32, c_void_p, c_void_p]

        dll.EdsSetCameraStateEventHandler.restype = EdsError
        dll.EdsSetCameraStateEventHandler.argtypes = [EdsCameraRef, EdsUInt32, c_void_p, c_void_p]

        # Get event (for console apps)
        dll.EdsGetEvent.restype = EdsError
        dll.EdsGetEvent.argtypes = []

    def _check_error(self, err: int, operation: str = ""):
        """Check for EDSDK errors and raise exception if needed"""
        if err != EdsErrorCode.EDS_ERR_OK:
            raise EDSDKError(err, f"{operation} failed with error: 0x{err:08X}")

    # ========== Basic Functions ==========

    def initialize(self) -> bool:
        """Initialize the SDK"""
        if self._initialized:
            return True
        err = self._dll.EdsInitializeSDK()
        if err == EdsErrorCode.EDS_ERR_OK:
            self._initialized = True
            return True
        return False

    def terminate(self):
        """Terminate the SDK"""
        if self._initialized:
            self._dll.EdsTerminateSDK()
            self._initialized = False

    # ========== Reference Functions ==========

    def retain(self, ref: c_void_p) -> int:
        """Increment reference counter"""
        return self._dll.EdsRetain(ref)

    def release(self, ref: c_void_p) -> int:
        """Decrement reference counter"""
        if ref:
            return self._dll.EdsRelease(ref)
        return 0

    # ========== Camera List Functions ==========

    def get_camera_list(self) -> c_void_p:
        """Get list of connected cameras"""
        camera_list = c_void_p()
        err = self._dll.EdsGetCameraList(byref(camera_list))
        self._check_error(err, "GetCameraList")
        return camera_list

    def get_child_count(self, ref: c_void_p) -> int:
        """Get number of child objects"""
        count = c_uint32()
        err = self._dll.EdsGetChildCount(ref, byref(count))
        self._check_error(err, "GetChildCount")
        return count.value

    def get_child_at_index(self, ref: c_void_p, index: int) -> c_void_p:
        """Get child object at index"""
        child = c_void_p()
        err = self._dll.EdsGetChildAtIndex(ref, index, byref(child))
        self._check_error(err, "GetChildAtIndex")
        return child

    # ========== Device Functions ==========

    def get_device_info(self, camera: c_void_p) -> EdsDeviceInfo:
        """Get camera device information"""
        info = EdsDeviceInfo()
        err = self._dll.EdsGetDeviceInfo(camera, byref(info))
        self._check_error(err, "GetDeviceInfo")
        return info

    # ========== Session Functions ==========

    def open_session(self, camera: c_void_p):
        """Open session with camera"""
        err = self._dll.EdsOpenSession(camera)
        self._check_error(err, "OpenSession")

    def close_session(self, camera: c_void_p):
        """Close session with camera"""
        err = self._dll.EdsCloseSession(camera)
        self._check_error(err, "CloseSession")

    # ========== Command Functions ==========

    def send_command(self, camera: c_void_p, command: int, param: int = 0):
        """Send command to camera"""
        err = self._dll.EdsSendCommand(camera, command, param)
        self._check_error(err, "SendCommand")

    def send_status_command(self, camera: c_void_p, command: int, param: int = 0):
        """Send status command to camera"""
        err = self._dll.EdsSendStatusCommand(camera, command, param)
        self._check_error(err, "SendStatusCommand")

    # ========== Property Functions ==========

    def get_property_data(self, ref: c_void_p, prop_id: int, param: int = 0) -> int:
        """Get property data as integer"""
        data_type = c_uint32()
        size = c_uint32()
        err = self._dll.EdsGetPropertySize(ref, prop_id, param, byref(data_type), byref(size))
        self._check_error(err, "GetPropertySize")

        value = c_uint32()
        err = self._dll.EdsGetPropertyData(ref, prop_id, param, size, byref(value))
        self._check_error(err, "GetPropertyData")
        return value.value

    def get_property_string(self, ref: c_void_p, prop_id: int, param: int = 0) -> str:
        """Get property data as string"""
        data_type = c_uint32()
        size = c_uint32()
        err = self._dll.EdsGetPropertySize(ref, prop_id, param, byref(data_type), byref(size))
        self._check_error(err, "GetPropertySize")

        buffer = ctypes.create_string_buffer(size.value)
        err = self._dll.EdsGetPropertyData(ref, prop_id, param, size, buffer)
        self._check_error(err, "GetPropertyData")
        return buffer.value.decode('utf-8', errors='ignore')

    def set_property_data(self, ref: c_void_p, prop_id: int, value: int, param: int = 0):
        """Set property data"""
        data = c_uint32(value)
        err = self._dll.EdsSetPropertyData(ref, prop_id, param, ctypes.sizeof(data), byref(data))
        self._check_error(err, "SetPropertyData")

    def get_property_desc(self, ref: c_void_p, prop_id: int) -> EdsPropertyDesc:
        """Get property description (available values)"""
        desc = EdsPropertyDesc()
        err = self._dll.EdsGetPropertyDesc(ref, prop_id, byref(desc))
        self._check_error(err, "GetPropertyDesc")
        return desc

    # ========== Volume Functions ==========

    def get_volume_info(self, volume: c_void_p) -> EdsVolumeInfo:
        """Get volume information"""
        info = EdsVolumeInfo()
        err = self._dll.EdsGetVolumeInfo(volume, byref(info))
        self._check_error(err, "GetVolumeInfo")
        return info

    def format_volume(self, volume: c_void_p):
        """Format volume"""
        err = self._dll.EdsFormatVolume(volume)
        self._check_error(err, "FormatVolume")

    # ========== Directory Functions ==========

    def get_directory_item_info(self, item: c_void_p) -> EdsDirectoryItemInfo:
        """Get directory item information"""
        info = EdsDirectoryItemInfo()
        err = self._dll.EdsGetDirectoryItemInfo(item, byref(info))
        self._check_error(err, "GetDirectoryItemInfo")
        return info

    def delete_directory_item(self, item: c_void_p):
        """Delete directory item"""
        err = self._dll.EdsDeleteDirectoryItem(item)
        self._check_error(err, "DeleteDirectoryItem")

    # ========== Download Functions ==========

    def download(self, item: c_void_p, size: int, stream: c_void_p):
        """Download file from camera"""
        err = self._dll.EdsDownload(item, size, stream)
        self._check_error(err, "Download")

    def download_complete(self, item: c_void_p):
        """Signal download complete"""
        err = self._dll.EdsDownloadComplete(item)
        self._check_error(err, "DownloadComplete")

    def download_cancel(self, item: c_void_p):
        """Cancel download"""
        err = self._dll.EdsDownloadCancel(item)
        self._check_error(err, "DownloadCancel")

    def download_thumbnail(self, item: c_void_p, stream: c_void_p):
        """Download thumbnail"""
        err = self._dll.EdsDownloadThumbnail(item, stream)
        self._check_error(err, "DownloadThumbnail")

    # ========== Stream Functions ==========

    def create_file_stream(self, filename: str, disposition: int, access: int) -> c_void_p:
        """Create file stream"""
        stream = c_void_p()
        err = self._dll.EdsCreateFileStream(
            filename.encode('utf-8'), disposition, access, byref(stream)
        )
        self._check_error(err, "CreateFileStream")
        return stream

    def create_memory_stream(self, size: int) -> c_void_p:
        """Create memory stream"""
        stream = c_void_p()
        err = self._dll.EdsCreateMemoryStream(size, byref(stream))
        self._check_error(err, "CreateMemoryStream")
        return stream

    def get_pointer(self, stream: c_void_p) -> c_void_p:
        """Get pointer to stream data"""
        pointer = c_void_p()
        err = self._dll.EdsGetPointer(stream, byref(pointer))
        self._check_error(err, "GetPointer")
        return pointer

    def get_length(self, stream: c_void_p) -> int:
        """Get stream length"""
        length = c_uint64()
        err = self._dll.EdsGetLength(stream, byref(length))
        self._check_error(err, "GetLength")
        return length.value

    # ========== Capacity Functions ==========

    def set_capacity(self, camera: c_void_p, capacity: EdsCapacity):
        """Set host computer capacity"""
        err = self._dll.EdsSetCapacity(camera, capacity)
        self._check_error(err, "SetCapacity")

    # ========== EVF Functions ==========

    def create_evf_image_ref(self, stream: c_void_p) -> c_void_p:
        """Create EVF image reference"""
        evf_image = c_void_p()
        err = self._dll.EdsCreateEvfImageRef(stream, byref(evf_image))
        self._check_error(err, "CreateEvfImageRef")
        return evf_image

    def download_evf_image(self, camera: c_void_p, evf_image: c_void_p):
        """Download EVF image"""
        err = self._dll.EdsDownloadEvfImage(camera, evf_image)
        if err != EdsErrorCode.EDS_ERR_OK and err != EdsErrorCode.EDS_ERR_OBJECT_NOTREADY:
            self._check_error(err, "DownloadEvfImage")
        return err == EdsErrorCode.EDS_ERR_OK

    # ========== Event Handler Functions ==========

    def set_property_event_handler(self, camera: c_void_p, event: int, handler: Callable, context: c_void_p = None):
        """Set property event handler"""
        callback = EdsPropertyEventHandler(handler)
        self._callbacks.append(callback)  # Keep reference
        err = self._dll.EdsSetPropertyEventHandler(camera, event, callback, context)
        self._check_error(err, "SetPropertyEventHandler")

    def set_object_event_handler(self, camera: c_void_p, event: int, handler: Callable, context: c_void_p = None):
        """Set object event handler"""
        callback = EdsObjectEventHandler(handler)
        self._callbacks.append(callback)  # Keep reference
        err = self._dll.EdsSetObjectEventHandler(camera, event, callback, context)
        self._check_error(err, "SetObjectEventHandler")

    def set_state_event_handler(self, camera: c_void_p, event: int, handler: Callable, context: c_void_p = None):
        """Set state event handler"""
        callback = EdsStateEventHandler(handler)
        self._callbacks.append(callback)  # Keep reference
        err = self._dll.EdsSetCameraStateEventHandler(camera, event, callback, context)
        self._check_error(err, "SetCameraStateEventHandler")

    # ========== Event Processing ==========

    def get_event(self):
        """Process events (call periodically in console apps)"""
        return self._dll.EdsGetEvent()
