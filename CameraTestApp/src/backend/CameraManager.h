#pragma once

#include <QObject>
#include <QList>
#include <QTimer>
#include "CameraDevice.h"

class CameraManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> cameras READ cameras NOTIFY camerasChanged)
    Q_PROPERTY(CameraDevice* currentCamera READ currentCamera NOTIFY currentCameraChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(bool sdkInitialized READ isSdkInitialized NOTIFY sdkInitializedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit CameraManager(QObject* parent = nullptr);
    ~CameraManager();

    QList<QObject*> cameras() const;
    CameraDevice* currentCamera() const { return m_currentCamera; }
    int currentIndex() const { return m_currentIndex; }
    bool isSdkInitialized() const { return m_sdkInitialized; }
    QString lastError() const { return m_lastError; }

public slots:
    bool initializeSDK();
    void terminateSDK();
    void refreshCameraList();
    void setCurrentIndex(int index);

    // Convenience methods that operate on current camera
    bool connectCurrentCamera();
    void disconnectCurrentCamera();
    bool takePicture();
    bool startLiveView();
    bool stopLiveView();

signals:
    void camerasChanged();
    void currentCameraChanged();
    void currentIndexChanged();
    void sdkInitializedChanged();
    void lastErrorChanged();
    void error(const QString& message);

private:
    void clearCameras();
    void setLastError(const QString& error);

    QList<CameraDevice*> m_cameras;
    CameraDevice* m_currentCamera = nullptr;
    int m_currentIndex = -1;
    bool m_sdkInitialized = false;
    QString m_lastError;
};
