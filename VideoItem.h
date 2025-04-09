#pragma once
#include <QQuickFramebufferObject>
#include "GStreamerGrabber.h"
#include "VideoRenderer.h"

class VideoItem : public QQuickFramebufferObject {
    Q_OBJECT
public:
    VideoItem();

    Renderer *createRenderer() const override;
    static GStreamerGrabber *grabber;  // グローバル共有
};

