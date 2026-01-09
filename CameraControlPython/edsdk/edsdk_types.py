"""
Canon EDSDK Type Definitions for Python
"""
import ctypes
from ctypes import c_uint32, c_int32, c_uint64, c_int64, c_void_p, c_char, c_bool, Structure, POINTER
from enum import IntEnum

# Basic Types
EdsError = c_uint32
EdsUInt32 = c_uint32
EdsInt32 = c_int32
EdsUInt64 = c_uint64
EdsInt64 = c_int64
EdsBool = c_bool
EdsVoid = c_void_p
EdsBaseRef = c_void_p
EdsCameraRef = c_void_p
EdsCameraListRef = c_void_p
EdsVolumeRef = c_void_p
EdsDirectoryItemRef = c_void_p
EdsStreamRef = c_void_p
EdsImageRef = c_void_p
EdsEvfImageRef = c_void_p

# Error Codes
class EdsErrorCode:
    EDS_ERR_OK = 0x00000000
    EDS_ERR_UNIMPLEMENTED = 0x00000001
    EDS_ERR_INTERNAL_ERROR = 0x00000002
    EDS_ERR_MEM_ALLOC_FAILED = 0x00000003
    EDS_ERR_MEM_FREE_FAILED = 0x00000004
    EDS_ERR_OPERATION_CANCELLED = 0x00000005
    EDS_ERR_INCOMPATIBLE_VERSION = 0x00000006
    EDS_ERR_NOT_SUPPORTED = 0x00000007
    EDS_ERR_UNEXPECTED_EXCEPTION = 0x00000008
    EDS_ERR_PROTECTION_VIOLATION = 0x00000009
    EDS_ERR_MISSING_SUBCOMPONENT = 0x0000000A
    EDS_ERR_SELECTION_UNAVAILABLE = 0x0000000B
    EDS_ERR_DEVICE_NOT_FOUND = 0x00000080
    EDS_ERR_DEVICE_BUSY = 0x00000081
    EDS_ERR_DEVICE_INVALID = 0x00000082
    EDS_ERR_DEVICE_EMERGENCY = 0x00000083
    EDS_ERR_DEVICE_MEMORY_FULL = 0x00000084
    EDS_ERR_DEVICE_INTERNAL_ERROR = 0x00000085
    EDS_ERR_DEVICE_INVALID_PARAMETER = 0x00000086
    EDS_ERR_DEVICE_NO_DISK = 0x00000087
    EDS_ERR_DEVICE_DISK_ERROR = 0x00000088
    EDS_ERR_DEVICE_CF_GATE_CHANGED = 0x00000089
    EDS_ERR_DEVICE_DIAL_CHANGED = 0x0000008A
    EDS_ERR_DEVICE_NOT_INSTALLED = 0x0000008B
    EDS_ERR_DEVICE_STAY_AWAKE = 0x0000008C
    EDS_ERR_DEVICE_NOT_RELEASED = 0x0000008D
    EDS_ERR_SESSION_NOT_OPEN = 0x00002003
    EDS_ERR_INVALID_TRANSACTIONID = 0x00002004
    EDS_ERR_INCOMPLETE_TRANSFER = 0x00002007
    EDS_ERR_INVALID_STRAGEID = 0x00002008
    EDS_ERR_DEVICEPROP_NOT_SUPPORTED = 0x0000200A
    EDS_ERR_INVALID_OBJECTFORMATCODE = 0x0000200B
    EDS_ERR_SELF_TEST_FAILED = 0x00002011
    EDS_ERR_PARTIAL_DELETION = 0x00002012
    EDS_ERR_SPECIFICATION_BY_FORMAT_UNSUPPORTED = 0x00002014
    EDS_ERR_NO_VALID_OBJECTINFO = 0x00002015
    EDS_ERR_INVALID_CODE_FORMAT = 0x00002016
    EDS_ERR_UNKNOWN_COMMAND = 0x0000201A
    EDS_ERR_OPERATION_REFUSED = 0x0000201B
    EDS_ERR_LENS_COVER_CLOSE = 0x0000201C
    EDS_ERR_LOW_BATTERY = 0x0000201D
    EDS_ERR_OBJECT_NOTREADY = 0x0000201E

# Property IDs
class PropID:
    ProductName = 0x00000002
    BodyIDEx = 0x00000015
    OwnerName = 0x00000004
    MakerName = 0x00000005
    DateTime = 0x00000006
    FirmwareVersion = 0x00000007
    BatteryLevel = 0x00000008
    SaveTo = 0x0000000b
    CurrentStorage = 0x0000000c
    CurrentFolder = 0x0000000d

    # Image Properties
    ImageQuality = 0x00000100
    Orientation = 0x00000102
    WhiteBalance = 0x00000106
    ColorTemperature = 0x00000107
    ColorSpace = 0x0000010d
    PictureStyle = 0x00000114

    # Capture Properties
    AEMode = 0x00000400
    AEModeSelect = 0x00000436
    DriveMode = 0x00000401
    ISOSpeed = 0x00000402
    MeteringMode = 0x00000403
    AFMode = 0x00000404
    Av = 0x00000405
    Tv = 0x00000406
    ExposureCompensation = 0x00000407
    FocalLength = 0x00000409
    AvailableShots = 0x0000040a
    LensName = 0x0000040d
    FlashMode = 0x00000414

    # EVF Properties
    Evf_OutputDevice = 0x00000500
    Evf_Mode = 0x00000501
    Evf_WhiteBalance = 0x00000502
    Evf_ColorTemperature = 0x00000503
    Evf_DepthOfFieldPreview = 0x00000504
    Evf_Zoom = 0x00000507
    Evf_ZoomPosition = 0x00000508
    Evf_AFMode = 0x0000050E

    Record = 0x00000510

    # Temperature
    TempStatus = 0x01000415
    FixedMovie = 0x01000422
    MovieParam = 0x01000423
    Aspect = 0x01000431

# Camera Commands
class CameraCommand:
    TakePicture = 0x00000000
    ExtendShutDownTimer = 0x00000001
    BulbStart = 0x00000002
    BulbEnd = 0x00000003
    PressShutterButton = 0x00000004
    DoEvfAf = 0x00000102
    DriveLensEvf = 0x00000103
    DoClickWBEvf = 0x00000104
    MovieSelectSwON = 0x00000107
    MovieSelectSwOFF = 0x00000108
    SetRemoteShootingMode = 0x0000010f
    RequestRollPitchLevel = 0x00000109

# Camera Status Commands
class CameraStatusCommand:
    UILock = 0x00000000
    UIUnLock = 0x00000001
    EnterDirectTransfer = 0x00000002
    ExitDirectTransfer = 0x00000003

# Shutter Button
class ShutterButton:
    OFF = 0x00000000
    Halfway = 0x00000001
    Completely = 0x00000003
    Halfway_NonAF = 0x00010001
    Completely_NonAF = 0x00010003

# EVF Output Device
class EvfOutputDevice:
    TFT = 0x00000001
    PC = 0x00000002
    PC_Small = 0x00000008

# Save To
class SaveTo:
    Camera = 1
    Host = 2
    Both = 3

# File Create Disposition
class FileCreateDisposition:
    CreateNew = 0
    CreateAlways = 1
    OpenExisting = 2
    OpenAlways = 3
    TruncateExisting = 4

# Access
class Access:
    Read = 0
    Write = 1
    ReadWrite = 2

# Seek Origin
class SeekOrigin:
    Cur = 0
    Begin = 1
    End = 2

# Drive Lens
class DriveLens:
    Near1 = 0x00000001
    Near2 = 0x00000002
    Near3 = 0x00000003
    Far1 = 0x00008001
    Far2 = 0x00008002
    Far3 = 0x00008003

# EVF Zoom
class EvfZoom:
    Fit = 1
    x5 = 5
    x6 = 6
    x10 = 10

# Property Events
class PropertyEvent:
    All = 0x00000100
    PropertyChanged = 0x00000101
    PropertyDescChanged = 0x00000102

# Object Events
class ObjectEvent:
    All = 0x00000200
    VolumeInfoChanged = 0x00000201
    VolumeUpdateItems = 0x00000202
    FolderUpdateItems = 0x00000203
    DirItemCreated = 0x00000204
    DirItemRemoved = 0x00000205
    DirItemInfoChanged = 0x00000206
    DirItemContentChanged = 0x00000207
    DirItemRequestTransfer = 0x00000208
    DirItemRequestTransferDT = 0x00000209
    DirItemCancelTransferDT = 0x0000020a
    VolumeAdded = 0x0000020c
    VolumeRemoved = 0x0000020d

# State Events
class StateEvent:
    All = 0x00000300
    Shutdown = 0x00000301
    JobStatusChanged = 0x00000302
    WillSoonShutDown = 0x00000303
    ShutDownTimerUpdate = 0x00000304
    CaptureError = 0x00000305
    InternalError = 0x00000306
    AfResult = 0x00000309
    BulbExposureTime = 0x00000310

# Structures
class EdsDeviceInfo(Structure):
    _fields_ = [
        ("szPortName", c_char * 256),
        ("szDeviceDescription", c_char * 256),
        ("deviceSubType", c_uint32),
        ("reserved", c_uint32),
    ]

class EdsVolumeInfo(Structure):
    _fields_ = [
        ("storageType", c_uint32),
        ("access", c_uint32),
        ("maxCapacity", c_uint64),
        ("freeSpaceInBytes", c_uint64),
        ("szVolumeLabel", c_char * 256),
    ]

class EdsDirectoryItemInfo(Structure):
    _fields_ = [
        ("size", c_uint64),
        ("isFolder", c_bool),
        ("groupID", c_uint32),
        ("option", c_uint32),
        ("szFileName", c_char * 256),
        ("format", c_uint32),
        ("dateTime", c_uint32),
    ]

class EdsPoint(Structure):
    _fields_ = [
        ("x", c_int32),
        ("y", c_int32),
    ]

class EdsSize(Structure):
    _fields_ = [
        ("width", c_int32),
        ("height", c_int32),
    ]

class EdsRect(Structure):
    _fields_ = [
        ("point", EdsPoint),
        ("size", EdsSize),
    ]

class EdsCapacity(Structure):
    _fields_ = [
        ("numberOfFreeClusters", c_int32),
        ("bytesPerSector", c_int32),
        ("reset", c_bool),
    ]

class EdsPropertyDesc(Structure):
    _fields_ = [
        ("form", c_int32),
        ("access", c_int32),
        ("numElements", c_int32),
        ("propDesc", c_int32 * 128),
    ]

# Callback function types
EdsProgressCallback = ctypes.CFUNCTYPE(c_uint32, c_uint32, c_void_p, POINTER(c_bool))
EdsCameraAddedHandler = ctypes.CFUNCTYPE(c_uint32, c_void_p)
EdsPropertyEventHandler = ctypes.CFUNCTYPE(c_uint32, c_uint32, c_uint32, c_uint32, c_void_p)
EdsObjectEventHandler = ctypes.CFUNCTYPE(c_uint32, c_uint32, c_void_p, c_void_p)
EdsStateEventHandler = ctypes.CFUNCTYPE(c_uint32, c_uint32, c_uint32, c_void_p)

# Value Mappings
ISO_VALUES = {
    0x00000000: "Auto",
    0x00000028: "6",
    0x00000030: "12",
    0x00000038: "25",
    0x00000040: "50",
    0x00000048: "100",
    0x0000004b: "125",
    0x0000004d: "160",
    0x00000050: "200",
    0x00000053: "250",
    0x00000055: "320",
    0x00000058: "400",
    0x0000005b: "500",
    0x0000005d: "640",
    0x00000060: "800",
    0x00000063: "1000",
    0x00000065: "1250",
    0x00000068: "1600",
    0x0000006b: "2000",
    0x0000006d: "2500",
    0x00000070: "3200",
    0x00000073: "4000",
    0x00000075: "5000",
    0x00000078: "6400",
    0x0000007b: "8000",
    0x0000007d: "10000",
    0x00000080: "12800",
    0x00000083: "16000",
    0x00000085: "20000",
    0x00000088: "25600",
    0x0000008b: "32000",
    0x0000008d: "40000",
    0x00000090: "51200",
    0x00000098: "102400",
}

AV_VALUES = {
    0x00: "Auto",
    0x08: "1.0",
    0x0B: "1.1",
    0x0C: "1.2",
    0x0D: "1.2 (1/3)",
    0x10: "1.4",
    0x13: "1.6",
    0x14: "1.8",
    0x15: "1.8 (1/3)",
    0x18: "2.0",
    0x1B: "2.2",
    0x1C: "2.5",
    0x1D: "2.5 (1/3)",
    0x20: "2.8",
    0x23: "3.2",
    0x24: "3.5",
    0x25: "3.5 (1/3)",
    0x28: "4.0",
    0x2B: "4.5",
    0x2C: "4.5 (1/3)",
    0x2D: "5.0",
    0x30: "5.6",
    0x33: "6.3",
    0x34: "6.7",
    0x35: "7.1",
    0x38: "8.0",
    0x3B: "9.0",
    0x3C: "9.5",
    0x3D: "10",
    0x40: "11",
    0x43: "13",
    0x44: "13 (1/3)",
    0x45: "14",
    0x48: "16",
    0x4B: "18",
    0x4C: "19",
    0x4D: "20",
    0x50: "22",
    0x53: "25",
    0x54: "27",
    0x55: "29",
    0x58: "32",
    0x5B: "36",
    0x5C: "38",
    0x5D: "40",
    0x60: "45",
    0x63: "51",
    0x64: "54",
    0x65: "57",
    0x68: "64",
    0x6B: "72",
    0x6C: "76",
    0x6D: "80",
    0x70: "91",
}

TV_VALUES = {
    0x00: "Auto",
    0x0C: "Bulb",
    0x10: "30\"",
    0x13: "25\"",
    0x14: "20\"",
    0x15: "20\" (1/3)",
    0x18: "15\"",
    0x1B: "13\"",
    0x1C: "10\"",
    0x1D: "10\" (1/3)",
    0x20: "8\"",
    0x23: "6\" (1/3)",
    0x24: "6\"",
    0x25: "5\"",
    0x28: "4\"",
    0x2B: "3\"2",
    0x2C: "3\"",
    0x2D: "2\"5",
    0x30: "2\"",
    0x33: "1\"6",
    0x34: "1\"5",
    0x35: "1\"3",
    0x38: "1\"",
    0x3B: "0\"8",
    0x3C: "0\"7",
    0x3D: "0\"6",
    0x40: "0\"5",
    0x43: "0\"4",
    0x44: "0\"3",
    0x45: "0\"3 (1/3)",
    0x48: "1/4",
    0x4B: "1/5",
    0x4C: "1/6",
    0x4D: "1/6 (1/3)",
    0x50: "1/8",
    0x53: "1/10 (1/3)",
    0x54: "1/10",
    0x55: "1/13",
    0x58: "1/15",
    0x5B: "1/20 (1/3)",
    0x5C: "1/20",
    0x5D: "1/25",
    0x60: "1/30",
    0x63: "1/40",
    0x64: "1/45",
    0x65: "1/50",
    0x68: "1/60",
    0x6B: "1/80",
    0x6C: "1/90",
    0x6D: "1/100",
    0x70: "1/125",
    0x73: "1/160",
    0x74: "1/180",
    0x75: "1/200",
    0x78: "1/250",
    0x7B: "1/320",
    0x7C: "1/350",
    0x7D: "1/400",
    0x80: "1/500",
    0x83: "1/640",
    0x84: "1/750",
    0x85: "1/800",
    0x88: "1/1000",
    0x8B: "1/1250",
    0x8C: "1/1500",
    0x8D: "1/1600",
    0x90: "1/2000",
    0x93: "1/2500",
    0x94: "1/3000",
    0x95: "1/3200",
    0x98: "1/4000",
    0x9B: "1/5000",
    0x9C: "1/6000",
    0x9D: "1/6400",
    0xA0: "1/8000",
}

WHITE_BALANCE_VALUES = {
    0: "Auto",
    1: "Daylight",
    2: "Cloudy",
    3: "Tungsten",
    4: "Fluorescent",
    5: "Flash",
    6: "Manual",
    8: "Shade",
    9: "Color Temp",
    23: "AWB White",
}

AE_MODE_VALUES = {
    0: "Program AE",
    1: "Tv",
    2: "Av",
    3: "Manual",
    4: "Bulb",
    5: "A-DEP",
    6: "DEP",
    7: "Camera settings registered",
    8: "Lock",
    9: "Auto",
    10: "Night Scene Portrait",
    11: "Sports",
    12: "Portrait",
    13: "Landscape",
    14: "Close-Up",
    15: "Flash Off",
    19: "Creative Auto",
    20: "Movies",
    21: "Photo In Movie",
    22: "Scene Intelligent Auto",
    23: "Night Scenes",
    24: "Backlit Scenes",
    25: "SCN",
    26: "Children",
    27: "Food",
    28: "Candlelight",
    29: "Night Portrait",
    30: "Grainy B/W",
    31: "Soft focus",
    32: "Toy camera effect",
    33: "Fish-eye effect",
    34: "Water painting effect",
    35: "Miniature effect",
    36: "HDR art standard",
    37: "HDR art vivid",
    38: "HDR art bold",
    39: "HDR art embossed",
    40: "Panning",
    41: "HDR",
    42: "Self Portrait",
    43: "Hybrid Auto",
    44: "Smooth skin",
    45: "Fv",
}

METERING_MODE_VALUES = {
    1: "Spot",
    3: "Evaluative",
    4: "Partial",
    5: "Center-weighted",
}

DRIVE_MODE_VALUES = {
    0x00000000: "Single",
    0x00000001: "Continuous",
    0x00000002: "Video",
    0x00000004: "Continuous High",
    0x00000005: "Continuous Low",
    0x00000006: "Silent Single",
    0x00000007: "Timer 10s + Remote",
    0x00000010: "Timer 10s",
    0x00000011: "Timer 2s",
    0x00000012: "Timer 10s + Continuous",
    0x00000013: "Silent Continuous High",
    0x00000014: "Silent Continuous Low",
    0x00000015: "Silent Single High",
    0x00000016: "Silent Single Low",
}
