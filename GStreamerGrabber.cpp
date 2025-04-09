#include "GStreamerGrabber.h"
#include <QDebug>

GStreamerGrabber::GStreamerGrabber(QObject *parent)
    : QObject(parent)
{
    gst_init(nullptr, nullptr);
}

GStreamerGrabber::~GStreamerGrabber()
{
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
}

bool GStreamerGrabber::init()
{
    QString pipelineStr = QStringLiteral(
        "imxv4l2src device=/dev/video0 ! "
        "imxvideoconvert ! "
        "video/x-raw, format=UYVY, width=1280, height=720 ! "
        "appsink name=appsink sync=false max-buffers=1 drop=true"
    );

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
    if (!pipeline) {
        qWarning() << "Failed to create pipeline:" << error->message;
        g_error_free(error);
        return false;
    }

    appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!appsink) {
        qWarning() << "appsink not found in pipeline";
        return false;
    }

    gst_app_sink_set_emit_signals((GstAppSink *)appsink, false);
    gst_app_sink_set_drop((GstAppSink *)appsink, true);
    gst_app_sink_set_max_buffers((GstAppSink *)appsink, 1);

    GstAppSinkCallbacks callbacks = {};
    callbacks.new_sample = &GStreamerGrabber::onNewSample;
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, this, nullptr);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return true;
}

bool GStreamerGrabber::start()
{
    return init();  // init() を内部で呼ぶだけ
}


QImage GStreamerGrabber::getCurrentFrame()
{
    QMutexLocker locker(&mutex);
    return lastFrame.copy(); // スレッドセーフに取得
}

GstFlowReturn GStreamerGrabber::onNewSample(GstAppSink *sink, gpointer user_data)
{
    GStreamerGrabber *self = static_cast<GStreamerGrabber *>(user_data);
    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample)
        return GST_FLOW_ERROR;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    if (!buffer || !caps) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        // RGBA に対応
        QImage img((const uchar *)map.data, width, height, QImage::Format_RGBA8888);
        if (!img.isNull()) {
            QMutexLocker locker(&self->mutex);
            self->lastFrame = img.copy();  // deep copy
        }
        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
	emit self->newFrame();  // ← これを追加！

    return GST_FLOW_OK;
}
