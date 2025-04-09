#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QTime>  // QTime クラスを使うためのインクルード
#include "GStreamerGrabber.h"

class VideoRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    VideoRenderer(GStreamerGrabber* grabber);
    ~VideoRenderer();

    void render() override;

private:
    void initGL();

    GStreamerGrabber* grabber = nullptr;
    QOpenGLShaderProgram shaderProgram;
    GLuint textureId = 0;
    GLuint vbo = 0;
    GLuint vao = 0;
    bool glInitialized = false;
};

#endif // VIDEORENDERER_H
