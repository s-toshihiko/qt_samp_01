#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "VideoItem.h"  // VideoItemクラスのヘッダーを適切にインクルード

int main(int argc, char *argv[]) {
    // Wayland用の設定
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell");
    
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    
    // 文字列引数を使用するようにsetSceneGraphBackendの呼び出しを修正
    QQuickWindow::setSceneGraphBackend("opengl");  // "openglrhi"ではなく"opengl"を使用
    
    QGuiApplication app(argc, argv);
    
    // QML で VideoItem を使えるように登録
    qmlRegisterType<VideoItem>("CustomVideo", 1, 0, "VideoItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}