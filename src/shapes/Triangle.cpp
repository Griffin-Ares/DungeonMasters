#include "Triangle.h"

void Triangle::updateParams() {
    m_vertexData = std::vector<float>();
    setVertexData();
}

void Triangle::setVertexData() {
    // Task 1: update m_vertexData with the vertices and normals
    //         needed to tesselate a triangle
    // Note: you may find the insertVec3 function useful in adding your points into m_vertexData

    glm::vec3 vertex1(-0.5f, 0.5f, 0.0f);
    glm::vec3 vertex2(-0.5f, -0.5f, 0.0f);
    glm::vec3 vertex3(0.5f, -0.5f, 0.0f);

    glm::vec3 normal = glm::normalize(glm::cross(vertex2 - vertex1, vertex3 - vertex1));

    insertVec3(m_vertexData, vertex1);
    insertVec3(m_vertexData, normal); // normal for v1
    insertVec3(m_vertexData, vertex2);
    insertVec3(m_vertexData, normal); // normal for v2
    insertVec3(m_vertexData, vertex3);
    insertVec3(m_vertexData, normal); // normal for v3

}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Triangle::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
