#include "camera/CameraManager.h"
#include "camera/CanonSDKCamera.h"
#include "EDSDK.h"
#include <algorithm>
#include <iostream>
#include <string>

namespace photobooth {

CameraManager::CameraManager() : activeCamera_(nullptr), initialized_(false) {
    saveDirectory_ = "data/captures";
}

CameraManager::~CameraManager() {
    shutdown();
}

bool CameraManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_)
        return true;

    EdsError err = EdsInitializeSDK();
    if (err == EDS_ERR_OK) {
        initialized_ = true;
        std::cout << "[CameraManager] EDSDK initialized successfully." << std::endl;
        return true;
    } else {
        std::cerr << "[CameraManager] Failed to initialize EDSDK. Error: " << err << std::endl;
        return false;
    }
}

void CameraManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (activeCamera_) {
        activeCamera_->disconnect();
        activeCamera_.reset();
    }

    if (initialized_) {
        EdsTerminateSDK();
        initialized_ = false;
        std::cout << "[CameraManager] EDSDK terminated." << std::endl;
    }
}

std::vector<std::string> CameraManager::detectCameras() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_)
        return {};

    std::vector<std::string> cameras;
    EdsCameraListRef cameraList = nullptr;
    EdsUInt32 count = 0;

    EdsError err = EdsGetCameraList(&cameraList);
    if (err == EDS_ERR_OK) {
        err = EdsGetChildCount(cameraList, &count);
        if (err == EDS_ERR_OK) {
            for (EdsUInt32 i = 0; i < count; i++) {
                EdsCameraRef camRef = nullptr;
                err = EdsGetChildAtIndex(cameraList, i, &camRef);
                if (err == EDS_ERR_OK) {
                    EdsDeviceInfo deviceInfo;
                    err = EdsGetDeviceInfo(camRef, &deviceInfo);
                    if (err == EDS_ERR_OK) {
                        cameras.push_back(std::string(deviceInfo.szDeviceDescription));
                    }
                    EdsRelease(camRef);
                }
            }
        }
        EdsRelease(cameraList);
    }

    std::cout << "[CameraManager] Detected " << cameras.size() << " Canon camera(s)." << std::endl;
    return cameras;
}

std::vector<CameraInfo> CameraManager::getAvailableCameras() const {
    std::vector<CameraInfo> cameras;

    if (!initialized_)
        return cameras;

    EdsCameraListRef cameraList = nullptr;
    EdsUInt32 count = 0;

    EdsError err = EdsGetCameraList(&cameraList);
    if (err == EDS_ERR_OK) {
        err = EdsGetChildCount(cameraList, &count);
        if (err == EDS_ERR_OK) {
            for (EdsUInt32 i = 0; i < count; i++) {
                EdsCameraRef camRef = nullptr;
                err = EdsGetChildAtIndex(cameraList, i, &camRef);
                if (err == EDS_ERR_OK) {
                    EdsDeviceInfo deviceInfo;
                    err = EdsGetDeviceInfo(camRef, &deviceInfo);
                    if (err == EDS_ERR_OK) {
                        CameraInfo info;
                        info.name = std::string(deviceInfo.szDeviceDescription);
                        info.type = CameraType::Canon;
                        info.connected = (activeCamera_ && activeCamera_->getName() == info.name);
                        info.index = static_cast<int>(i);
                        cameras.push_back(info);
                    }
                    EdsRelease(camRef);
                }
            }
        }
        EdsRelease(cameraList);
    }

    return cameras;
}

bool CameraManager::selectCamera(const std::string &cameraName) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_)
        return false;

    // Disconnect existing camera
    if (activeCamera_) {
        activeCamera_->stopLiveView();
        activeCamera_->disconnect();
        activeCamera_.reset();
    }

    EdsCameraListRef cameraList = nullptr;
    EdsUInt32 count = 0;
    bool found = false;

    EdsError err = EdsGetCameraList(&cameraList);
    if (err == EDS_ERR_OK) {
        err = EdsGetChildCount(cameraList, &count);
        if (err == EDS_ERR_OK) {
            for (EdsUInt32 i = 0; i < count; i++) {
                EdsCameraRef camRef = nullptr;
                err = EdsGetChildAtIndex(cameraList, i, &camRef);
                if (err == EDS_ERR_OK) {
                    EdsDeviceInfo deviceInfo;
                    err = EdsGetDeviceInfo(camRef, &deviceInfo);
                    if (err == EDS_ERR_OK) {
                        std::string name(deviceInfo.szDeviceDescription);

                        // Match by name or select first if "Auto" or empty
                        if (cameraName.empty() || name == cameraName || cameraName == "Auto") {
                            // Create CanonSDKCamera (bodyID not available in this SDK version)
                            activeCamera_ = std::make_unique<CanonSDKCamera>(camRef, 0);
                            activeCamera_->setSaveDirectory(saveDirectory_);

                            if (activeCamera_->connect()) {
                                found = true;
                                std::cout << "[CameraManager] Connected to camera: " << name << std::endl;
                                // Don't release camRef - camera owns it now
                                break;
                            } else {
                                activeCamera_.reset();
                                EdsRelease(camRef);
                            }
                        } else {
                            EdsRelease(camRef);
                        }
                    } else {
                        EdsRelease(camRef);
                    }
                }
            }
        }
        EdsRelease(cameraList);
    }

    if (!found) {
        std::cerr << "[CameraManager] Camera not found: " << cameraName << std::endl;
    }

    return found;
}

ICamera* CameraManager::getActiveCamera() {
    return activeCamera_.get();
}

std::string CameraManager::getActiveCameraName() const {
    if (activeCamera_)
        return activeCamera_->getName();
    return "";
}

bool CameraManager::startLiveView(LiveViewCallback callback) {
    if (activeCamera_)
        return activeCamera_->startLiveView(callback);
    return false;
}

void CameraManager::stopLiveView() {
    if (activeCamera_)
        activeCamera_->stopLiveView();
}

void CameraManager::capture(CaptureMode mode, CaptureCallback callback) {
    if (activeCamera_) {
        activeCamera_->capture(mode, callback);
    } else {
        if (callback) {
            CaptureResult res;
            res.success = false;
            res.errorMessage = "No active camera";
            callback(res);
        }
    }
}

void CameraManager::captureWithCountdown(int seconds, CaptureMode mode,
                                         CaptureCallback callback) {
    if (activeCamera_) {
        activeCamera_->captureWithCountdown(seconds, mode, callback);
    } else {
        if (callback) {
            CaptureResult res;
            res.success = false;
            res.errorMessage = "No active camera";
            callback(res);
        }
    }
}

bool CameraManager::setSettings(const CameraSettings &settings) {
    if (activeCamera_)
        return activeCamera_->setSettings(settings);
    return false;
}

CameraSettings CameraManager::getSettings() const {
    if (activeCamera_)
        return activeCamera_->getSettings();
    return CameraSettings();
}

std::vector<int> CameraManager::getSupportedISO() const {
    if (activeCamera_)
        return activeCamera_->getSupportedISO();
    return {};
}

std::vector<std::string> CameraManager::getSupportedApertures() const {
    if (activeCamera_)
        return activeCamera_->getSupportedApertures();
    return {};
}

std::vector<std::string> CameraManager::getSupportedShutterSpeeds() const {
    if (activeCamera_)
        return activeCamera_->getSupportedShutterSpeeds();
    return {};
}

std::vector<std::string> CameraManager::getSupportedWhiteBalances() const {
    if (activeCamera_)
        return activeCamera_->getSupportedWhiteBalances();
    return {};
}

// Extended SDK methods

CanonSupportedValues CameraManager::getAllSupportedCameraValues() {
    if (activeCamera_)
        return activeCamera_->getAllSupportedValues();
    return CanonSupportedValues();
}

CanonCameraSettings CameraManager::getExtendedCameraSettings() const {
    if (activeCamera_)
        return activeCamera_->getExtendedSettings();
    return CanonCameraSettings();
}

bool CameraManager::setExtendedCameraSettings(const CanonCameraSettings& settings) {
    if (activeCamera_)
        return activeCamera_->setExtendedSettings(settings);
    return false;
}

bool CameraManager::setCameraPropertyByCode(EdsPropertyID propertyID, EdsUInt32 code) {
    if (activeCamera_)
        return activeCamera_->setPropertyByCode(propertyID, code);
    return false;
}

CanonSDKCamera* CameraManager::getCanonCamera() {
    return activeCamera_.get();
}

void CameraManager::setSaveDirectory(const std::string& dir) {
    saveDirectory_ = dir;
    if (activeCamera_) {
        activeCamera_->setSaveDirectory(dir);
    }
}

void CameraManager::detectCanonCameras() {
    // Already implemented in detectCameras()
}

} // namespace photobooth
