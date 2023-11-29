#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "camera.h"
#include "settings.h"
#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"
#include "utils/sceneparser.h"
#include "utils/shaderloader.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent),
    cam(renderData.cameraData, 800, 600, 0.1f, 100.f)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    for (auto& vboPair : shapeTypeVBOs) {
        GLuint vbo = vboPair.second;
        glDeleteBuffers(1, &vbo);
    }

    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_shader);

    this->doneCurrent();
}

void Realtime::initializeShapeVBOs() {
    // Create and set up a VBO for each shape type
    std::vector<PrimitiveType> shapeTypes = {PrimitiveType::PRIMITIVE_SPHERE,
                                             PrimitiveType::PRIMITIVE_CUBE,
                                             PrimitiveType::PRIMITIVE_CYLINDER,
                                             PrimitiveType::PRIMITIVE_CONE,
    };
    for (const auto& type : shapeTypes) {
        // generate vbo
        GLuint vbo;

        // check if vbo for type already exists
        if (shapeTypeVBOs[type] > 0) {
            vbo = shapeTypeVBOs[type];
        } else {
            glGenBuffers(1, &vbo);
        }

        // Call bindVbo to generate shape data and bind it to the VBO
        bindVbo(type, vbo);

        // Store the VBO and VAO in the map
        shapeTypeVBOs[type] = vbo;
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glClearColor(0, 0, 0, 0);
    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");

    // setup VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    // Set up vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(3 * sizeof(GL_FLOAT)));

    shapeTypeVBOs[PrimitiveType::PRIMITIVE_SPHERE] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CUBE] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CYLINDER] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CONE] = 0;

    Realtime::initializeShapeVBOs();

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // 1 vao for each vbo efficiency

    initializedRun = true;
}

void Realtime::bindVbo(PrimitiveType shapeType, GLuint vbo) {
    switch (shapeType) {
        case PrimitiveType::PRIMITIVE_SPHERE:
            {
                Sphere sphere;
                sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                verts = sphere.generateShape();
            }
            break;

        case PrimitiveType::PRIMITIVE_CUBE:
            {
                Cube cube;
                cube.updateParams(settings.shapeParameter1);
                verts = cube.generateShape();
            }
            break;

        case PrimitiveType::PRIMITIVE_CYLINDER:
            {
                Cylinder cylinder;
                cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                verts = cylinder.generateShape();
            }
        break;

        case PrimitiveType::PRIMITIVE_CONE:
        {
                Cone cone;
                cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
                verts = cone.generateShape();
        }
        break;

        default:
            return;
    }

    // bind
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // data
    vertexCount[shapeType] = int(verts.size()) / 6;
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
}

void Realtime::paintGL() {

    // sentry
    if (renderData.shapes.empty()) {
        return;
    }

    // Students: anything requiring OpenGL calls every frame should be done here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);

    if (m_keyMap[Qt::Key_W]) {
        cam.moveForward(.1f);
    }
    if (m_keyMap[Qt::Key_S]) {
        cam.moveForward(-.1f);
    }

    // camera stuff, update every paintGL to prepare for movement in Action
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjectionMatrix();

    GLint viewLoc = glGetUniformLocation(m_shader, "viewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    GLint projLoc = glGetUniformLocation(m_shader, "projMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

    glm::vec4 camPos = inverse(view) * glm::vec4(0.0, 0.0, 0.0, 1.0);
    GLint camPosLoc = glGetUniformLocation(m_shader, "camPos");
    glUniform4fv(camPosLoc, 1, &camPos[0]);

    Realtime::initializeLightingData();

    for (const auto& shape : renderData.shapes) {
        // update model matrix and other uniforms specific to this shape
        glm::mat4 model = shape.ctm;

        glBindBuffer(GL_ARRAY_BUFFER, shapeTypeVBOs[shape.primitive.type]);

        glBindVertexArray(m_vao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(3 * sizeof(GL_FLOAT)));

        GLint modelMatrixLoc = glGetUniformLocation(m_shader, "modelMatrix");
        glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);

        glm::mat3 invTranspose = glm::mat3(glm::transpose(glm::inverse(model)));
        GLint inverseTransposeLoc = glGetUniformLocation(m_shader, "inverseTranspose");
        glUniformMatrix3fv(inverseTransposeLoc, 1, GL_FALSE, &invTranspose[0][0]);

        GLint ambientLoc = glGetUniformLocation(m_shader, "matAmbient");
        glUniform4fv(ambientLoc, 1, &shape.primitive.material.cAmbient[0]);

        GLint diffuseLoc = glGetUniformLocation(m_shader, "matDiffuse");
        glUniform4fv(diffuseLoc, 1, &shape.primitive.material.cDiffuse[0]);

        GLint specularLoc = glGetUniformLocation(m_shader, "matSpecular");
        glUniform4fv(specularLoc, 1, &shape.primitive.material.cSpecular[0]);

        GLint shinyLoc = glGetUniformLocation(m_shader, "shininess");
        glUniform1f(shinyLoc, shape.primitive.material.shininess);

        // draw
        glDrawArrays(GL_TRIANGLES, 0, vertexCount[shape.primitive.type]);

    }


    // Unbind VAO and shaders
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    width = w;
    height = h;
    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::initializeLightingData() {

    // get relevant renderData
    SceneGlobalData global = renderData.globalData;

    // pass light position and m_kd into the fragment shader as a uniform
    for (int i = 0; i < renderData.lights.size(); ++i) {
        SceneLightData light = renderData.lights[i];

        //std::cout << (int)light.type << std::endl;

        // type as int
        std::string typeString = "lights[" + std::to_string(i) + "].type";
        GLint lightTypeLoc = glGetUniformLocation(m_shader, typeString.c_str());
        glUniform1i(lightTypeLoc, (int)light.type);

        // dir
        std::string dirString = "lights[" + std::to_string(i) + "].dir";
        GLint lightDirLoc = glGetUniformLocation(m_shader, dirString.c_str());
        glUniform4fv(lightDirLoc, 1, &light.dir[0]);

        // dir
        std::string posString = "lights[" + std::to_string(i) + "].pos";
        GLint lightPosLoc = glGetUniformLocation(m_shader, posString.c_str());
        glUniform4fv(lightPosLoc, 1, &light.pos[0]);

        // color
        std::string colorString = "lights[" + std::to_string(i) + "].color";
        GLint lightColorLoc = glGetUniformLocation(m_shader, colorString.c_str());
        glUniform4fv(lightColorLoc, 1, &light.color[0]);

        // attenuation
        std::string attString = "lights[" + std::to_string(i) + "].att";
        GLint lightAttLoc = glGetUniformLocation(m_shader, attString.c_str());
        glUniform3fv(lightAttLoc, 1, &light.function[0]);

        // angle
        std::string angleString = "lights[" + std::to_string(i) + "].angle";
        GLint lightAngleLoc = glGetUniformLocation(m_shader, angleString.c_str());
        glUniform1f(lightAngleLoc, light.angle);

        // penumbra
        std::string penString = "lights[" + std::to_string(i) + "].penumbra";
        GLint lightPenLoc = glGetUniformLocation(m_shader, penString.c_str());
        glUniform1f(lightPenLoc, light.penumbra);

    }
    GLint lightCountLoc = glGetUniformLocation(m_shader, "lightCount");
    glUniform1i(lightCountLoc, renderData.lights.size());

    // global lighting params
    GLint kaLoc = glGetUniformLocation(m_shader, "k_a");
    glUniform1f(kaLoc, global.ka);

    GLint kdLoc = glGetUniformLocation(m_shader, "k_d");
    glUniform1f(kdLoc, global.kd);

    GLint ksLoc = glGetUniformLocation(m_shader, "k_s");
    glUniform1f(ksLoc, global.ks);
}

void Realtime::sceneChanged() {
    renderData = RenderData();
    SceneParser::parse(settings.sceneFilePath, renderData);
    cam = Camera(renderData.cameraData, width, height, settings.nearPlane, settings.farPlane);
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    if (initializedRun) {
        cam.setPlanes(settings.nearPlane, settings.farPlane);
        Realtime::initializeShapeVBOs();
    }
    update(); // asks for a PaintGL() call to occur

}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
