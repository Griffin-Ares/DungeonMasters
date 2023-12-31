#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "camera.h"
#include "dungeon.h"
#include "utils/sceneparser.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void generateDungeon();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    GLuint m_shader; // Stores id of shader program
    GLuint m_vbo;    // Stores id of VBO
    GLuint m_vao;    // Stores id of VAO

    //glm::mat4 m_model = glm::mat4(1);
    //glm::mat4 m_view  = glm::mat4(1);
    //glm::mat4 m_proj  = glm::mat4(1);

    void makeFBO();
    GLuint m_defaultFBO;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    GLuint m_texture_shader;

    // normal mapping
    QImage m_brick_image;
    QImage m_floor_image;
    GLuint m_brick_texture;
    GLuint m_floor_texture;
    GLuint m_brick_location;
    GLuint m_floor_location;
    bool toggleTexture;


    Dungeon dungeon;

    void paintTexture(GLuint texture, bool doPostProcess);

    Camera cam;

    bool initializedRun = false;
    int width;
    int height;
    RenderData renderData;

    void initializeShapeVBOs();
    void bindVbo(PrimitiveType shapeType, GLuint vbo);
    void initializeLightingData();

    std::unordered_map<PrimitiveType, GLuint> shapeTypeVBOs;
    std::unordered_map<PrimitiveType, GLuint> shapeTypeVAOs;
    std::unordered_map<PrimitiveType, int> vertexCount;

    std::vector<GLfloat> verts;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;
};
