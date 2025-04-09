#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "VideoItem.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // QML で VideoItem を使えるように登録
    qmlRegisterType<VideoItem>("CustomVideo", 1, 0, "VideoItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

