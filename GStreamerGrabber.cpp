#include "GStreamerGrabber.h"
#include <QDebug>

GStreamerGrabber::GStreamerGrabber(QObject *parent)
    : QObject(parent)
{
    qDebug() << "GStreamerGrabber constructor called";
    gst_init(nullptr, nullptr);
}

GStreamerGrabber::~GStreamerGrabber()
{
    if (pipeline) {
        qDebug() << "Cleaning up GStreamer pipeline";
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
}

bool GStreamerGrabber::init()
{
    qDebug() << "GStreamerGrabber::init() called";
    
    // 既にパイプラインが作成されていれば初期化済み
    if (pipeline) {
        qDebug() << "Pipeline already initialized";
        return true;
    }

    QString pipelineStr = QStringLiteral(
        "imxv4l2src device=/dev/video0 ! "
        "video/x-raw, format=I420, width=1280, height=720 ! "
        "imxvideoconvert_g2d  ! "
        "video/x-raw, format=RGBA, width=1280, height=720 ! "
        "appsink name=appsink sync=false max-buffers=1 drop=true"
    );

    qDebug() << "Creating pipeline:" << pipelineStr;

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
    if (!pipeline) {
        qWarning() << "Failed to create pipeline:" << error->message;
        g_error_free(error);
        return false;
    }

    qDebug() << "Pipeline created successfully";

    appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!appsink) {
        qWarning() << "appsink not found in pipeline";
        return false;
    }

    qDebug() << "Setting up appsink callbacks";
    
    gst_app_sink_set_emit_signals((GstAppSink *)appsink, false);
    gst_app_sink_set_drop((GstAppSink *)appsink, true);
    gst_app_sink_set_max_buffers((GstAppSink *)appsink, 1);

    // コールバック構造体の初期化を修正
    GstAppSinkCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));  // すべてのフィールドを0で初期化
    callbacks.new_sample = &GStreamerGrabber::onNewSample;
    
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, this, nullptr);
    
    qDebug() << "Callbacks set up";

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    qDebug() << "Pipeline state change result:" << ret;
    
    // 状態変更が成功したか確認
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set pipeline to PLAYING";
        return false;
    }
    
    // パイプラインの状態を取得
    GstState state;
    ret = gst_element_get_state(pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    qDebug() << "Pipeline current state:" << state << " (return code:" << ret << ")";
    
    return true;
}

bool GStreamerGrabber::start()
{
    // init()を呼ぶだけにし、二重初期化を防止
    return init();
}

QImage GStreamerGrabber::getCurrentFrame()
{
    QMutexLocker locker(&mutex);
    return lastFrame.copy(); // スレッドセーフに取得
}

GstFlowReturn GStreamerGrabber::onNewSample(GstAppSink *sink, gpointer user_data)
{
    static int frameCount = 0;
    frameCount++;
    
    GStreamerGrabber *self = static_cast<GStreamerGrabber *>(user_data);
    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample) {
        qWarning() << "Failed to pull sample";
        return GST_FLOW_ERROR;
    }

    // 最初のフレームとその後10フレームごとにデバッグ出力
    bool printDebug = (frameCount == 1 || frameCount % 10 == 0);
    
    if (printDebug) {
        qDebug() << "onNewSample called, frame:" << frameCount;
    }

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    if (!buffer || !caps) {
        if (printDebug) {
            qWarning() << "Invalid buffer or caps";
        }
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    
    if (printDebug) {
        qDebug() << "Frame info:" << width << "x" << height 
                 << "format:" << gst_structure_get_string(structure, "format");
    }

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        if (printDebug) {
            qDebug() << "Buffer mapped, size:" << map.size;
        }
        
        // QImageの作成 - stride（行の長さ）を正しく設定
        QImage img((const uchar *)map.data, width, height, width * 4, QImage::Format_RGBA8888);
        if (!img.isNull()) {
            QMutexLocker locker(&self->mutex);
            self->lastFrame = img.copy();  // deep copy
            if (printDebug) {
                qDebug() << "Frame copied to lastFrame:" << self->lastFrame.width() 
                         << "x" << self->lastFrame.height();
            }
        } else {
            if (printDebug) {
                qWarning() << "Failed to create QImage";
            }
        }
        gst_buffer_unmap(buffer, &map);
    } else {
        if (printDebug) {
            qWarning() << "Failed to map buffer";
        }
    }

    gst_sample_unref(sample);
    emit self->newFrame();  // フレーム更新シグナル

    return GST_FLOW_OK;
}