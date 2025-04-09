#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "VideoItem.h"

int main(int argc, char *argv[]) {
    qDebug() << "Application starting...";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    
    // Wayland環境変数の確認
    qDebug() << "QT_QPA_PLATFORM=" << qgetenv("QT_QPA_PLATFORM");
    qDebug() << "WAYLAND_DISPLAY=" << qgetenv("WAYLAND_DISPLAY");
    
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    qDebug() << "Set AA_ShareOpenGLContexts attribute";
    
    QGuiApplication app(argc, argv);
    qDebug() << "QGuiApplication created";

    // QML で VideoItem を使えるように登録
    qmlRegisterType<VideoItem>("CustomVideo", 1, 0, "VideoItem");
    qDebug() << "Registered VideoItem type for QML";

    QQmlApplicationEngine engine;
    qDebug() << "Loading QML engine";
    
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML - no root objects";
        return -1;
    }
    
    qDebug() << "QML loaded successfully, starting event loop";
    return app.exec();
}
