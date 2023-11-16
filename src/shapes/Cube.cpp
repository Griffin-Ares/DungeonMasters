#include "Cube.h"
#include <iostream>
#include <ostream>

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    glm::vec3 normal = glm::normalize(glm::cross(bottomLeft - topLeft, bottomRight - topLeft));

    // bottom left triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal); // normal for v1
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal); // normal for v2
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal); // normal for v3

    // top right triangle
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal); // normal for v1
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal); // normal for v2
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal); // normal for v3
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    float step = 1.0f / m_param1;
    glm::vec3 hDiff = topRight - topLeft;
    glm::vec3 vDiff = bottomLeft - topLeft;

    for (int i = 0; i < m_param1; ++i) {
        for (int j = 0; j < m_param1; ++j) {
            glm::vec3 tileTopLeft(
                topLeft + step * j * hDiff + step * i * vDiff
                );
            glm::vec3 tileTopRight(
                topLeft + step * (j + 1) * hDiff + step * i * vDiff
                );

            glm::vec3 tileBottomLeft(
                topLeft + step * j * hDiff + step * (i + 1) * vDiff
                );

            glm::vec3 tileBottomRight(
                topLeft + step * (j + 1) * hDiff + step * (i + 1) * vDiff
                );

            makeTile(tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight);
        }
    }
}

void Cube::setVertexData() {

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));


    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    // Back face
    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    // Left face
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f));

    // Right face
    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  -0.5f, 0.5f),
             glm::vec3( 0.5f,  -0.5f, -0.5f));

    // Top face
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f));

    // bottom face
    makeFace(glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f));

}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
