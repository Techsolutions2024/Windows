#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class LiveViewImageProvider : public QQuickImageProvider {
public:
    LiveViewImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override {
        Q_UNUSED(id);
        Q_UNUSED(requestedSize);

        QMutexLocker locker(&m_mutex);

        if (size) {
            *size = m_image.size();
        }

        return m_image;
    }

    void setImage(const QImage& image) {
        QMutexLocker locker(&m_mutex);
        m_image = image;
    }

private:
    QImage m_image;
    QMutex m_mutex;
};
