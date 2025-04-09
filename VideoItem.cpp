#include "VideoItem.h"
#include "VideoRenderer.h"

GStreamerGrabber* VideoItem::grabber = nullptr;

VideoItem::VideoItem() {
    if (!grabber) {
        grabber = new GStreamerGrabber();

        // init() のみ呼び出し（start() は呼び出さない）
        if (!grabber->init()) {
            qCritical() << "Failed to initialize GStreamerGrabber";
        }

        connect(grabber, &GStreamerGrabber::newFrame, this, [this]() {
            static int updateCount = 0;
            updateCount++;
            if (updateCount % 30 == 0) {
                qDebug() << "newFrame signal received, count:" << updateCount;
            }
            update();
        });
    }
}

QQuickFramebufferObject::Renderer* VideoItem::createRenderer() const {
    qDebug() << "VideoItem::createRenderer called";
    return new VideoRenderer(grabber);
}