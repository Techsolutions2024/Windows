#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include "backend/CameraManager.h"
#include "backend/CameraDevice.h"
#include "backend/PropertyHelper.h"
#include "backend/LiveViewImageProvider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("Canon Camera Test");
    app.setOrganizationName("CameraTest");
    app.setApplicationVersion("1.0.0");

    // Set Material style
    QQuickStyle::setStyle("Material");

    // Register QML types
    qmlRegisterType<CameraManager>("CameraTest", 1, 0, "CameraManager");
    qmlRegisterType<CameraDevice>("CameraTest", 1, 0, "CameraDevice");
    qmlRegisterType<PropertyHelper>("CameraTest", 1, 0, "PropertyHelper");

    // Also make CameraDevice available as uncreatable type for property access
    qmlRegisterUncreatableType<CameraDevice>("CameraTest", 1, 0, "CameraDeviceType",
        "CameraDevice cannot be created in QML");

    QQmlApplicationEngine engine;

    // Create image provider for live view
    LiveViewImageProvider* imageProvider = new LiveViewImageProvider();
    engine.addImageProvider("live", imageProvider);

    // Create and expose singleton instances
    CameraManager cameraManager;
    PropertyHelper propertyHelper;

    engine.rootContext()->setContextProperty("cameraManager", &cameraManager);
    engine.rootContext()->setContextProperty("propertyHelper", &propertyHelper);
    engine.rootContext()->setContextProperty("imageProvider", imageProvider);

    // Load QML
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
