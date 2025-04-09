#include "VideoItem.h"
#include "VideoRenderer.h"

GStreamerGrabber* VideoItem::grabber = nullptr;

VideoItem::VideoItem() {
    qDebug() << "VideoItem constructor called";
    
    if (!grabber) {
        qDebug() << "Creating new GStreamerGrabber";
        grabber = new GStreamerGrabber();

        if (!grabber->init()) {
            qCritical() << "Failed to initialize GStreamerGrabber";
        } else {
            qDebug() << "GStreamerGrabber initialized successfully";
        }

        connect(grabber, &GStreamerGrabber::newFrame, this, [this]() {
            // newFrameシグナルが発火した回数を確認
            static int updateCount = 0;
            if (updateCount % 30 == 0) {  // 30フレームごとに報告
                qDebug() << "VideoItem::update called via newFrame signal, count:" << updateCount;
            }
            updateCount++;
            update();
        });

        if (!grabber->start()) {
            qCritical() << "Failed to start GStreamerGrabber";
        } else {
            qDebug() << "GStreamerGrabber started successfully";
        }
    } else {
        qDebug() << "Using existing GStreamerGrabber instance";
    }
}

QQuickFramebufferObject::Renderer* VideoItem::createRenderer() const {
    qDebug() << "VideoItem::createRenderer called";
    return new VideoRenderer(grabber);
}