#include "CameraManager.h"
#include "EDSDKHelper.h"
#include <QDebug>

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
{
}

CameraManager::~CameraManager() {
    clearCameras();
    if (m_sdkInitialized) {
        terminateSDK();
    }
}

QList<QObject*> CameraManager::cameras() const {
    QList<QObject*> list;
    for (auto* cam : m_cameras) {
        list.append(cam);
    }
    return list;
}

bool CameraManager::initializeSDK() {
    if (m_sdkInitialized) return true;

    if (edsdk::initializeSDK()) {
        m_sdkInitialized = true;
        emit sdkInitializedChanged();
        refreshCameraList();
        return true;
    } else {
        setLastError("Failed to initialize Canon SDK");
        return false;
    }
}

void CameraManager::terminateSDK() {
    clearCameras();
    edsdk::terminateSDK();
    m_sdkInitialized = false;
    emit sdkInitializedChanged();
}

void CameraManager::refreshCameraList() {
    if (!m_sdkInitialized) {
        setLastError("SDK not initialized");
        return;
    }

    clearCameras();

    EdsCameraListRef cameraList = nullptr;
    EdsError err = EdsGetCameraList(&cameraList);

    if (err != EDS_ERR_OK) {
        setLastError(QString("Failed to get camera list: %1").arg(QString::fromStdString(edsdk::errorToString(err))));
        return;
    }

    EdsUInt32 count = 0;
    err = EdsGetChildCount(cameraList, &count);

    if (err == EDS_ERR_OK && count > 0) {
        for (EdsUInt32 i = 0; i < count; i++) {
            EdsCameraRef camera = nullptr;
            err = EdsGetChildAtIndex(cameraList, i, &camera);

            if (err == EDS_ERR_OK && camera) {
                EdsDeviceInfo deviceInfo;
                err = EdsGetDeviceInfo(camera, &deviceInfo);

                if (err == EDS_ERR_OK) {
                    CameraDevice* device = new CameraDevice(camera, deviceInfo, this);
                    connect(device, &CameraDevice::error, this, &CameraManager::error);
                    m_cameras.append(device);
                }

                EdsRelease(camera);
            }
        }
    }

    EdsRelease(cameraList);
    emit camerasChanged();

    // Auto-select first camera if available
    if (!m_cameras.isEmpty() && m_currentIndex < 0) {
        setCurrentIndex(0);
    }
}

void CameraManager::setCurrentIndex(int index) {
    if (index == m_currentIndex) return;

    if (index < 0 || index >= m_cameras.size()) {
        m_currentIndex = -1;
        m_currentCamera = nullptr;
    } else {
        m_currentIndex = index;
        m_currentCamera = m_cameras.at(index);
    }

    emit currentIndexChanged();
    emit currentCameraChanged();
}

bool CameraManager::connectCurrentCamera() {
    if (!m_currentCamera) {
        setLastError("No camera selected");
        return false;
    }
    return m_currentCamera->openSession();
}

void CameraManager::disconnectCurrentCamera() {
    if (m_currentCamera) {
        m_currentCamera->closeSession();
    }
}

bool CameraManager::takePicture() {
    if (!m_currentCamera) {
        setLastError("No camera selected");
        return false;
    }
    return m_currentCamera->takePicture();
}

bool CameraManager::startLiveView() {
    if (!m_currentCamera) {
        setLastError("No camera selected");
        return false;
    }
    return m_currentCamera->startLiveView();
}

bool CameraManager::stopLiveView() {
    if (!m_currentCamera) {
        setLastError("No camera selected");
        return false;
    }
    return m_currentCamera->stopLiveView();
}

void CameraManager::clearCameras() {
    if (m_currentCamera) {
        m_currentCamera = nullptr;
        emit currentCameraChanged();
    }
    m_currentIndex = -1;
    emit currentIndexChanged();

    qDeleteAll(m_cameras);
    m_cameras.clear();
    emit camerasChanged();
}

void CameraManager::setLastError(const QString& error) {
    m_lastError = error;
    emit lastErrorChanged();
    emit this->error(error);
}
