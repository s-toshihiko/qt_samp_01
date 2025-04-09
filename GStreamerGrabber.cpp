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
        "video/x-raw, format=I420, width=1280, height=720 ! "
        "videoconvert ! video/x-raw, format=RGBA ! "  // RGBA に変換
        "appsink name=appsink sync=false max-buffers=1 drop=true"
    );

    qDebug() << "Creating pipeline:" << pipelineStr;  // パイプライン文字列のデバッグ

    GError *error = nullptr;
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &error);
    if (!pipeline) {
        qWarning() << "Failed to create pipeline:" << error->message;
        g_error_free(error);
        return false;
    }

    qDebug() << "Pipeline created successfully";

    // 残りのコードは同じ...

    // 最後の部分を次のように修正
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    qDebug() << "Pipeline state change result:" << ret;
    
    // 状態変更が成功したか確認
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set pipeline to PLAYING";
        return false;
    }
    
    // 追加のデバッグ: パイプラインの状態を取得
    GstState state;
    ret = gst_element_get_state(pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
    qDebug() << "Pipeline current state:" << state << " (return code:" << ret << ")";
    
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
    static int frameCount = 0;
    
    GStreamerGrabber *self = static_cast<GStreamerGrabber *>(user_data);
    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample) {
        qWarning() << "Failed to pull sample";
        return GST_FLOW_ERROR;
    }

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    if (!buffer || !caps) {
        qWarning() << "Invalid buffer or caps";
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    
    // 10フレームごとにデバッグ出力（大量ログを防止）
    if (frameCount % 10 == 0) {
        qDebug() << "Frame received:" << frameCount 
                 << "Format:" << gst_structure_get_string(structure, "format")
                 << "Size:" << width << "x" << height;
    }
    
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        // サンプルデータの最初の数バイトを確認
        if (frameCount % 10 == 0) {
            qDebug() << "Buffer size:" << map.size << "First bytes:" 
                     << QString("0x%1 0x%2 0x%3 0x%4")
                        .arg(map.data[0], 2, 16, QChar('0'))
                        .arg(map.data[1], 2, 16, QChar('0'))
                        .arg(map.data[2], 2, 16, QChar('0'))
                        .arg(map.data[3], 2, 16, QChar('0'));
        }
        
        // QImageの作成
        QImage img((const uchar *)map.data, width, height, width * 4, QImage::Format_RGBA8888);
        if (img.isNull()) {
            qWarning() << "Failed to create QImage from buffer";
        } else {
            QMutexLocker locker(&self->mutex);
            self->lastFrame = img.copy();  // deep copy
            // 10フレームごとにデバッグ出力
            if (frameCount % 10 == 0) {
                qDebug() << "Frame copied to lastFrame, size:" << self->lastFrame.width() 
                         << "x" << self->lastFrame.height();
            }
        }
        gst_buffer_unmap(buffer, &map);
    } else {
        qWarning() << "Failed to map buffer";
    }

    gst_sample_unref(sample);
    emit self->newFrame();
    
    frameCount++;
    return GST_FLOW_OK;
}