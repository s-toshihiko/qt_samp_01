#include "VideoItem.h"
#include "VideoRenderer.h"

GStreamerGrabber* VideoItem::grabber = nullptr;

VideoItem::VideoItem() {
    if (!grabber) {
        grabber = new GStreamerGrabber();

        grabber->init();     // ← 必須！

        connect(grabber, &GStreamerGrabber::newFrame, this, [this]() {
            update();        // ← これで描画更新されます！
        });

        grabber->start();
    }
}

QQuickFramebufferObject::Renderer* VideoItem::createRenderer() const {
    return new VideoRenderer(grabber);
}
