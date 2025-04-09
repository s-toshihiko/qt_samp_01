#include "VideoRenderer.h"
#include <QOpenGLContext>
#include <QImage>

VideoRenderer::VideoRenderer(GStreamerGrabber* grabber)
    : grabber(grabber)
{
}

VideoRenderer::~VideoRenderer()
{
    if (textureId) {
        glDeleteTextures(1, &textureId);
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
}

void VideoRenderer::initGL()
{
    initializeOpenGLFunctions();

    static const char* vertexShaderSrc = R"(
        attribute vec2 vertexPosition;
        attribute vec2 texCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord = texCoord;
            gl_Position = vec4(vertexPosition, 0.0, 1.0);
        }
    )";

    static const char* fragmentShaderSrc = R"(
        varying highp vec2 vTexCoord;
        uniform sampler2D texture;
        void main() {
            gl_FragColor = texture2D(texture, vTexCoord);
        }
    )";

    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc);
    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc);
    shaderProgram.link();

    GLfloat vertices[] = {
        // positions    // tex coords
        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 0.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint posAttr = shaderProgram.attributeLocation("vertexPosition");
    GLint texAttr = shaderProgram.attributeLocation("texCoord");

    glEnableVertexAttribArray(posAttr);
    glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(texAttr);
    glVertexAttribPointer(texAttr, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glBindVertexArray(0);

    glDeleteBuffers(1, &ebo);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glInitialized = true;
}

void VideoRenderer::render()
{
    if (!glInitialized)
        initGL();

	qDebug() << "VideoRenderer::render() called";

    QImage frame = grabber->getCurrentFrame();
    if (frame.isNull()) return;

    glBindTexture(GL_TEXTURE_2D, textureId);
    QImage glImage = frame.convertToFormat(QImage::Format_RGBA8888);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgram.bind();
    glBindVertexArray(vao);
    shaderProgram.setUniformValue("texture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    shaderProgram.release();
}
