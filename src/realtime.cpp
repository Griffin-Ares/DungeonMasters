#include "realtime.h"
#include "debug.h"

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
    glDeleteProgram(m_texture_shader);

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteVertexArrays(1, &m_fullscreen_vao);

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

    m_defaultFBO = 2; // CHANGE DEFAULT FRAMEBUFFER HERE
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

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
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    // setup VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Set up vertex attribute pointers
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), reinterpret_cast<void*>(3 * sizeof(GL_FLOAT))); // ERROR HERE!! invalid operation
    GLenum error1 = glGetError();
    if (error1 != GL_NO_ERROR) {
        std::cerr << "OpenGL Error 1: " << gluErrorString(error1) << std::endl;
    }

    shapeTypeVBOs[PrimitiveType::PRIMITIVE_SPHERE] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CUBE] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CYLINDER] = 0;
    shapeTypeVBOs[PrimitiveType::PRIMITIVE_CONE] = 0;

    Realtime::initializeShapeVBOs();

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // 1 vao for each vbo efficiency

    // NORMAL MAPPING STUFF STARTS
    // TEXTURES
    glUseProgram(m_shader);
    // load brick texture
    QString brick_filepath = QString(":/resources/brickfinal.jpg"); // prepare filepath
    m_brick_image = QImage(brick_filepath); // obtain image from filepath
    m_brick_image = m_brick_image.convertToFormat(QImage::Format_RGBA8888).mirrored(); // format image to fit OpenGL
    glGenTextures(1, &m_brick_texture); // generate brick texture
    glActiveTexture(GL_TEXTURE0); // set the active texture slot to texture slot 0
    glBindTexture(GL_TEXTURE_2D, m_brick_texture); // bind brick texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_brick_image.width(), m_brick_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_brick_image.bits()); // load image into brick texture
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind brick texture
    // set the default.frag uniform for brick texture
    m_brick_location = glGetUniformLocation(m_shader, "brickMap");
    //glUniform1i(brickTexLocation, 0);

    // load floor texture
    QString floor_filepath = QString(":/resources/floorfinal.png"); // prepare filepath
    m_floor_image = QImage(floor_filepath); // obtain image from filepath
    m_floor_image = m_floor_image.convertToFormat(QImage::Format_RGBA8888).mirrored(); // format image to fit OpenGL
    glGenTextures(1, &m_floor_texture); // generate floor texture
    glActiveTexture(GL_TEXTURE1); // set the active texture slot to texture slot 1
    glBindTexture(GL_TEXTURE_2D, m_floor_texture); // bind floor texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_floor_image.width(), m_floor_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_floor_image.bits()); // load image into floor texture
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind floor texture
    // set the default.frag uniform for floor texture
    m_floor_location = glGetUniformLocation(m_shader, "floorMap");
    //glUniform1i(floorTexLocation, 1);

    toggleTexture = false;

    // TBN matrices
    float positiveX[3][3] = {
        {0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0}
    };
    float negativeX[3][3] = {
        {0.0, 0.0, -1.0},
        {0.0, 1.0, 0.0},
        {-1.0, 0.0, 0.0}
    };
    float positiveZ[3][3] = {
        {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
    float negativeZ[3][3] = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, -1.0}
    };
    float positiveY[3][3] = {
        {1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0}
    };
    GLint matrixLocation1 = glGetUniformLocation(m_shader, "posX");
    glUniformMatrix3fv(matrixLocation1, 1, GL_FALSE, &positiveX[0][0]);
    GLint matrixLocation2 = glGetUniformLocation(m_shader, "negX");
    glUniformMatrix3fv(matrixLocation2, 1, GL_FALSE, &negativeX[0][0]);
    GLint matrixLocation3 = glGetUniformLocation(m_shader, "posZ");
    glUniformMatrix3fv(matrixLocation3, 1, GL_FALSE, &positiveZ[0][0]);
    GLint matrixLocation4 = glGetUniformLocation(m_shader, "negZ");
    glUniformMatrix3fv(matrixLocation4, 1, GL_FALSE, &negativeZ[0][0]);
    GLint matrixLocation5 = glGetUniformLocation(m_shader, "posY");
    glUniformMatrix3fv(matrixLocation5, 1, GL_FALSE, &positiveY[0][0]);
    //glUseProgram(0);
    // NORMAL MAPPING STUFF ENDS

//    glUseProgram(m_texture_shader);  // Use the texture shader program
//    GLint textureUniform = glGetUniformLocation(m_texture_shader, "texture");  // Replace with your actual uniform name
//    glUniform1i(textureUniform, 0);

    // FBO stuff
    std::vector<GLfloat> fullscreen_quad_data =
    { //     POSITIONS    //
         -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,
         -1.0f, -1.0f, 0.0f,
         0.0f,  0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,
         -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  0.0f,
    };

    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();

    initializedRun = true;
}

void Realtime::generateDungeon() {
    dungeon.destroyRooms();
    dungeon.generateRooms(settings.size * 2 + 5, settings.size * 2 + 5, settings.size);
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
    glUseProgram(m_shader);
    // toggling normal mapping
    GLint textureLoc = glGetUniformLocation(m_shader, "isTextured");
    if (toggleTexture) {
        glUniform1i(textureLoc, 1);
    } else {
        glUniform1i(textureLoc, 0);
    }

    // sentry
    if (renderData.shapes.empty()) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_fbo_width, m_fbo_height);

    // Students: anything requiring OpenGL calls every frame should be done here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shader);

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

    /*
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

    }*/

    // draw dungeon rooms
    for (const auto& room : dungeon.getRooms()) {
        // Draw each component of the room
        for (const auto& component : room.walls) {
            glm::mat4 model = component.ctm;

            glBindBuffer(GL_ARRAY_BUFFER, shapeTypeVBOs[component.primitive.type]);

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
            glUniform4fv(ambientLoc, 1, &component.primitive.material.cAmbient[0]);

            GLint diffuseLoc = glGetUniformLocation(m_shader, "matDiffuse");
            glUniform4fv(diffuseLoc, 1, &component.primitive.material.cDiffuse[0]);

            GLint specularLoc = glGetUniformLocation(m_shader, "matSpecular");
            glUniform4fv(specularLoc, 1, &component.primitive.material.cSpecular[0]);

            GLint shinyLoc = glGetUniformLocation(m_shader, "shininess");
            glUniform1f(shinyLoc, component.primitive.material.shininess);

            // bind normal maps
            glActiveTexture(GL_TEXTURE0); // set the active texture slot to texture slot 0
            glBindTexture(GL_TEXTURE_2D, m_brick_texture); // bind brick texture

            glActiveTexture(GL_TEXTURE1); // set the active texture slot to texture slot 0
            glBindTexture(GL_TEXTURE_2D, m_floor_texture); // bind brick texture

            glUniform1i(m_brick_location, 0);
            glUniform1i(m_floor_location, 1);

            // draw
            glDrawArrays(GL_TRIANGLES, 0, vertexCount[component.primitive.type]);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width * this->devicePixelRatio(), m_screen_height * this->devicePixelRatio());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintTexture(m_fbo_texture, true);

    // Unbind VAO, textures, and shaders
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}

void Realtime::paintTexture(GLuint texture, bool doPostProcess) {
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn
    GLint doPostProcessPixelLoc = glGetUniformLocation(m_texture_shader, "doPostProcessPixel");
    glUniform1i(doPostProcessPixelLoc, settings.perPixelFilter ? 1 : 0);

    GLint doPostProcessKernelLoc = glGetUniformLocation(m_texture_shader, "doPostProcessKernel");
    glUniform1i(doPostProcessKernelLoc, settings.kernelBasedFilter ? 1 : 0);

    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    width = w;
    height = h;
    // Students: anything requiring OpenGL calls when the program starts should be done here
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    // Task 34: Regenerate your FBOs
    makeFBO();
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


void Realtime::makeFBO() {
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_fbo_width, m_fbo_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
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
        toggleTexture = settings.normalMapping;
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
        cam.rotateCamera(deltaX, deltaY);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key_W]) {
        cam.moveForward(5.f, deltaTime);
    }
    if (m_keyMap[Qt::Key_S]) {
        cam.moveForward(-5.f, deltaTime);
    }
    if (m_keyMap[Qt::Key_A]) {
        cam.moveRight(-5.f, deltaTime);
    }
    if (m_keyMap[Qt::Key_D]) {
        cam.moveRight(5.f, deltaTime);
    }
    if (m_keyMap[Qt::Key_Space]) {
        cam.moveUp(5.f, deltaTime);
    }
    if (m_keyMap[Qt::Key_Control]) {
        cam.moveUp(-5.f, deltaTime);
    }

    if (m_keyMap[Qt::Key_B]) {
        cam.updatePosition(-5.f, deltaTime);
    }

    update(); // asks for a PaintGL() call to occur
}
