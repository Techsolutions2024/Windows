#include "Camera.h"
#include <iostream>
#include <sstream>
#include <map>

namespace PhotoBooth {
namespace EDSDK {

// ISO value mappings
static const std::map<EdsUInt32, std::string> ISO_VALUES = {
    {0x00000000, "Auto"},
    {0x00000028, "6"},
    {0x00000030, "12"},
    {0x00000038, "25"},
    {0x00000040, "50"},
    {0x00000048, "100"},
    {0x0000004b, "125"},
    {0x0000004d, "160"},
    {0x00000050, "200"},
    {0x00000053, "250"},
    {0x00000055, "320"},
    {0x00000058, "400"},
    {0x0000005b, "500"},
    {0x0000005d, "640"},
    {0x00000060, "800"},
    {0x00000063, "1000"},
    {0x00000065, "1250"},
    {0x00000068, "1600"},
    {0x0000006b, "2000"},
    {0x0000006d, "2500"},
    {0x00000070, "3200"},
    {0x00000073, "4000"},
    {0x00000075, "5000"},
    {0x00000078, "6400"},
    {0x0000007b, "8000"},
    {0x0000007d, "10000"},
    {0x00000080, "12800"},
    {0x00000083, "16000"},
    {0x00000085, "20000"},
    {0x00000088, "25600"},
    {0x0000008b, "32000"},
    {0x0000008d, "40000"},
    {0x00000090, "51200"},
    {0x00000093, "64000"},
    {0x00000095, "80000"},
    {0x00000098, "102400"}
};

// Aperture value mappings
static const std::map<EdsUInt32, std::string> AV_VALUES = {
    {0x08, "f/1.0"},
    {0x0B, "f/1.1"},
    {0x0C, "f/1.2"},
    {0x0D, "f/1.2"},
    {0x10, "f/1.4"},
    {0x13, "f/1.6"},
    {0x14, "f/1.8"},
    {0x15, "f/1.8"},
    {0x18, "f/2.0"},
    {0x1B, "f/2.2"},
    {0x1C, "f/2.5"},
    {0x1D, "f/2.5"},
    {0x20, "f/2.8"},
    {0x23, "f/3.2"},
    {0x24, "f/3.5"},
    {0x25, "f/3.5"},
    {0x28, "f/4.0"},
    {0x2B, "f/4.5"},
    {0x2C, "f/4.5"},
    {0x2D, "f/5.0"},
    {0x30, "f/5.6"},
    {0x33, "f/6.3"},
    {0x34, "f/6.7"},
    {0x35, "f/7.1"},
    {0x38, "f/8.0"},
    {0x3B, "f/9.0"},
    {0x3C, "f/9.5"},
    {0x3D, "f/10"},
    {0x40, "f/11"},
    {0x43, "f/13"},
    {0x44, "f/13"},
    {0x45, "f/14"},
    {0x48, "f/16"},
    {0x4B, "f/18"},
    {0x4C, "f/19"},
    {0x4D, "f/20"},
    {0x50, "f/22"},
    {0x53, "f/25"},
    {0x54, "f/27"},
    {0x55, "f/29"},
    {0x58, "f/32"}
};

Camera::Camera()
    : cameraList_(nullptr)
    , camera_(nullptr)
    , evfImage_(nullptr)
    , initialized_(false)
    , connected_(false)
    , sessionOpen_(false)
    , liveViewActive_(false)
    , downloadPath_("./photos")
{
}

Camera::~Camera() {
    Shutdown();
}

bool Camera::Initialize() {
    if (initialized_) {
        return true;
    }

    EdsError err = EdsInitializeSDK();
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to initialize EDSDK");
        return false;
    }

    initialized_ = true;
    return true;
}

void Camera::Shutdown() {
    if (liveViewActive_) {
        StopLiveView();
    }

    if (sessionOpen_) {
        CloseSession();
    }

    if (connected_) {
        DisconnectCamera();
    }

    if (cameraList_) {
        EdsRelease(cameraList_);
        cameraList_ = nullptr;
    }

    if (initialized_) {
        EdsTerminateSDK();
        initialized_ = false;
    }
}

bool Camera::ConnectCamera(int cameraIndex) {
    if (!initialized_) {
        if (!Initialize()) {
            return false;
        }
    }

    // Get camera list
    EdsError err = EdsGetCameraList(&cameraList_);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to get camera list");
        return false;
    }

    // Get camera count
    EdsUInt32 count = 0;
    err = EdsGetChildCount(cameraList_, &count);
    if (err != EDS_ERR_OK || count == 0) {
        NotifyError(err, "No cameras found");
        return false;
    }

    // Get camera at index
    if (cameraIndex >= static_cast<int>(count)) {
        NotifyError(EDS_ERR_DEVICE_NOT_FOUND, "Camera index out of range");
        return false;
    }

    err = EdsGetChildAtIndex(cameraList_, cameraIndex, &camera_);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to get camera");
        return false;
    }

    // Set event handlers
    err = EdsSetObjectEventHandler(camera_, kEdsObjectEvent_All, HandleObjectEvent, this);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set object event handler");
        return false;
    }

    err = EdsSetPropertyEventHandler(camera_, kEdsPropertyEvent_All, HandlePropertyEvent, this);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set property event handler");
        return false;
    }

    err = EdsSetCameraStateEventHandler(camera_, kEdsStateEvent_All, HandleStateEvent, this);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set state event handler");
        return false;
    }

    connected_ = true;

    // Load camera info
    LoadCameraInfo();

    if (onCameraConnected_) {
        onCameraConnected_(camera_);
    }

    return true;
}

bool Camera::DisconnectCamera() {
    if (!connected_) {
        return true;
    }

    if (liveViewActive_) {
        StopLiveView();
    }

    if (sessionOpen_) {
        CloseSession();
    }

    if (camera_) {
        EdsRelease(camera_);
        camera_ = nullptr;
    }

    connected_ = false;

    if (onCameraDisconnected_) {
        onCameraDisconnected_();
    }

    return true;
}

bool Camera::IsConnected() const {
    return connected_;
}

std::vector<std::string> Camera::GetCameraList() {
    std::vector<std::string> cameras;

    if (!initialized_) {
        if (!const_cast<Camera*>(this)->Initialize()) {
            return cameras;
        }
    }

    EdsCameraListRef tempList = nullptr;
    EdsError err = EdsGetCameraList(&tempList);
    if (err != EDS_ERR_OK) {
        return cameras;
    }

    EdsUInt32 count = 0;
    err = EdsGetChildCount(tempList, &count);
    if (err != EDS_ERR_OK) {
        EdsRelease(tempList);
        return cameras;
    }

    for (EdsUInt32 i = 0; i < count; i++) {
        EdsCameraRef camera = nullptr;
        err = EdsGetChildAtIndex(tempList, i, &camera);
        if (err == EDS_ERR_OK) {
            EdsDeviceInfo deviceInfo;
            err = EdsGetDeviceInfo(camera, &deviceInfo);
            if (err == EDS_ERR_OK) {
                cameras.push_back(deviceInfo.szDeviceDescription);
            }
            EdsRelease(camera);
        }
    }

    EdsRelease(tempList);
    return cameras;
}

int Camera::GetCameraCount() {
    return static_cast<int>(GetCameraList().size());
}

CameraInfo Camera::GetCameraInfo() {
    return cameraInfo_;
}

bool Camera::OpenSession() {
    if (!connected_ || sessionOpen_) {
        return false;
    }

    EdsError err = EdsOpenSession(camera_);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to open session");
        return false;
    }

    sessionOpen_ = true;
    return true;
}

bool Camera::CloseSession() {
    if (!sessionOpen_) {
        return true;
    }

    EdsError err = EdsCloseSession(camera_);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to close session");
        return false;
    }

    sessionOpen_ = false;
    return true;
}

bool Camera::StartLiveView() {
    if (!connected_ || !sessionOpen_) {
        return false;
    }

    if (liveViewActive_) {
        return true;
    }

    // Set save destination to host
    EdsUInt32 saveTo = kEdsSaveTo_Host;
    EdsError err = EdsSetPropertyData(camera_, kEdsPropID_SaveTo, 0, sizeof(saveTo), &saveTo);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set save destination");
        return false;
    }

    // Set capacity
    EdsCapacity capacity = {0x7FFFFFFF, 0x1000, 1};
    err = EdsSetCapacity(camera_, capacity);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set capacity");
        return false;
    }

    // Start EVF
    EdsUInt32 evfMode = 1;
    err = EdsSetPropertyData(camera_, kEdsPropID_Evf_Mode, 0, sizeof(evfMode), &evfMode);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to start live view");
        return false;
    }

    // Set output device
    EdsUInt32 device = kEdsEvfOutputDevice_PC;
    err = EdsSetPropertyData(camera_, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to set output device");
        return false;
    }

    liveViewActive_ = true;
    return true;
}

bool Camera::StopLiveView() {
    if (!liveViewActive_) {
        return true;
    }

    // Stop EVF
    EdsUInt32 evfMode = 0;
    EdsError err = EdsSetPropertyData(camera_, kEdsPropID_Evf_Mode, 0, sizeof(evfMode), &evfMode);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to stop live view");
        return false;
    }

    // Set output device to camera
    EdsUInt32 device = kEdsEvfOutputDevice_TFT;
    err = EdsSetPropertyData(camera_, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device);

    if (evfImage_) {
        EdsRelease(evfImage_);
        evfImage_ = nullptr;
    }

    liveViewActive_ = false;
    return true;
}

bool Camera::IsLiveViewActive() const {
    return liveViewActive_;
}

bool Camera::DownloadLiveViewFrame() {
    if (!liveViewActive_) {
        return false;
    }

    // Create EVF image
    EdsError err = EdsCreateEvfImageRef(camera_, &evfImage_);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to create EVF image");
        return false;
    }

    // Download EVF data
    err = EdsDownloadEvfImage(camera_, evfImage_);
    if (err != EDS_ERR_OK) {
        EdsRelease(evfImage_);
        evfImage_ = nullptr;
        return false;
    }

    // Get image data
    EdsStreamRef stream = nullptr;
    err = EdsGetPropertyData(evfImage_, kEdsPropID_Evf_CoordinateSystem, 0, sizeof(stream), &stream);
    
    if (onLiveViewFrame_) {
        // Get image size
        EdsSize size;
        err = EdsGetPropertyData(evfImage_, kEdsPropID_Evf_CoordinateSystem, 0, sizeof(size), &size);
        
        // Get image pointer
        EdsStreamRef imageStream = nullptr;
        err = EdsCreateMemoryStream(0, &imageStream);
        if (err == EDS_ERR_OK) {
            err = EdsDownloadEvfImage(camera_, evfImage_);
            
            unsigned char* data = nullptr;
            EdsUInt64 length = 0;
            err = EdsGetPointer(imageStream, (EdsVoid**)&data);
            err = EdsGetLength(imageStream, &length);
            
            if (err == EDS_ERR_OK && data && length > 0) {
                onLiveViewFrame_(data, static_cast<size_t>(length), size.width, size.height);
            }
            
            EdsRelease(imageStream);
        }
    }

    EdsRelease(evfImage_);
    evfImage_ = nullptr;

    return true;
}

bool Camera::TakePicture() {
    if (!connected_ || !sessionOpen_) {
        return false;
    }

    EdsError err = EdsSendCommand(camera_, kEdsCameraCommand_TakePicture, 0);
    if (err != EDS_ERR_OK) {
        NotifyError(err, "Failed to take picture");
        return false;
    }

    return true;
}

bool Camera::LoadCameraInfo() {
    if (!connected_) {
        return false;
    }

    EdsDeviceInfo deviceInfo;
    EdsError err = EdsGetDeviceInfo(camera_, &deviceInfo);
    if (err != EDS_ERR_OK) {
        return false;
    }

    cameraInfo_.modelName = deviceInfo.szDeviceDescription;
    cameraInfo_.serialNumber = deviceInfo.szPortName;

    // Get firmware version
    EdsChar fwVersion[64];
    err = EdsGetPropertyData(camera_, kEdsPropID_FirmwareVersion, 0, sizeof(fwVersion), fwVersion);
    if (err == EDS_ERR_OK) {
        cameraInfo_.firmwareVersion = fwVersion;
    }

    // Get battery level
    EdsUInt32 battery = 0;
    err = EdsGetPropertyData(camera_, kEdsPropID_BatteryLevel, 0, sizeof(battery), &battery);
    if (err == EDS_ERR_OK) {
        cameraInfo_.batteryLevel = battery;
    }

    // Get lens name
    EdsChar lensName[256];
    err = EdsGetPropertyData(camera_, kEdsPropID_LensName, 0, sizeof(lensName), lensName);
    if (err == EDS_ERR_OK) {
        cameraInfo_.lensName = lensName;
    }

    return true;
}

void Camera::NotifyError(EdsError error, const std::string& message) {
    lastError_ = message + " (Error: " + ErrorToString(error) + ")";
    
    if (onError_) {
        onError_(error, lastError_);
    }
}

std::string Camera::ErrorToString(EdsError error) {
    std::stringstream ss;
    ss << "0x" << std::hex << error;
    return ss.str();
}

std::string Camera::GetLastError() const {
    return lastError_;
}

// Event handlers
EdsError EDSCALLBACK Camera::HandleObjectEvent(
    EdsObjectEvent event,
    EdsBaseRef object,
    EdsVoid* context)
{
    Camera* camera = static_cast<Camera*>(context);
    
    if (event == kEdsObjectEvent_DirItemRequestTransfer) {
        EdsDirectoryItemRef dirItem = (EdsDirectoryItemRef)object;
        
        // Download the image
        EdsDirectoryItemInfo dirItemInfo;
        EdsGetDirectoryItemInfo(dirItem, &dirItemInfo);
        
        // Create file stream
        std::string filePath = camera->downloadPath_ + "/" + dirItemInfo.szFileName;
        EdsStreamRef stream = nullptr;
        EdsCreateFileStream(filePath.c_str(), kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
        
        if (stream) {
            EdsDownload(dirItem, dirItemInfo.size, stream);
            EdsDownloadComplete(dirItem);
            EdsRelease(stream);
            
            if (camera->onPhotoCaptured_) {
                camera->onPhotoCaptured_(filePath);
            }
        }
    }
    
    return EDS_ERR_OK;
}

EdsError EDSCALLBACK Camera::HandlePropertyEvent(
    EdsPropertyEvent event,
    EdsPropertyID propertyID,
    EdsUInt32 param,
    EdsVoid* context)
{
    // Handle property changes
    return EDS_ERR_OK;
}

EdsError EDSCALLBACK Camera::HandleStateEvent(
    EdsStateEvent event,
    EdsUInt32 param,
    EdsVoid* context)
{
    Camera* camera = static_cast<Camera*>(context);
    
    if (event == kEdsStateEvent_Shutdown) {
        if (camera->onCameraDisconnected_) {
            camera->onCameraDisconnected_();
        }
    }
    
    return EDS_ERR_OK;
}

// Callback setters
void Camera::SetCameraConnectedCallback(CameraConnectedCallback callback) {
    onCameraConnected_ = callback;
}

void Camera::SetCameraDisconnectedCallback(CameraDisconnectedCallback callback) {
    onCameraDisconnected_ = callback;
}

void Camera::SetPhotoCapturedCallback(PhotoCapturedCallback callback) {
    onPhotoCaptured_ = callback;
}

void Camera::SetLiveViewFrameCallback(LiveViewFrameCallback callback) {
    onLiveViewFrame_ = callback;
}

void Camera::SetErrorCallback(ErrorCallback callback) {
    onError_ = callback;
}

} // namespace EDSDK
} // namespace PhotoBooth
