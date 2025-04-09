#ifndef GSTREAMERGRABBER_H
#define GSTREAMERGRABBER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class GStreamerGrabber : public QObject
{
    Q_OBJECT
public:
    explicit GStreamerGrabber(QObject *parent = nullptr);
    ~GStreamerGrabber();

    QImage getCurrentFrame();

    bool init();

    bool start();

signals:
    void newFrame();

private:
    static GstFlowReturn onNewSample(GstAppSink *sink, gpointer user_data);

    GstElement *pipeline = nullptr;
    GstElement *appsink = nullptr;

    QImage lastFrame;
    QMutex mutex;
};

#endif // GSTREAMERGRABBER_H
