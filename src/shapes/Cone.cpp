#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = fmax(param1, 1);
    m_param2 = fmax(param2, 3);
    setVertexData();
}

void Cone::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    bool useNormal,
                    glm::vec3 normal) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!

    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!

    glm::vec3 normals[6];
    if (useNormal) {
        for (int i = 0; i < 6; i++) {
            normals[i] = normal;
        }
    } else {
        if (abs(topLeft.y - 0.5) < 0.001) {
            glm::vec3 leftDiff = bottomLeft - topLeft;
            glm::vec3 rightDiff = bottomRight - topRight;

            normals[0] = glm::vec3(2 * topLeft.x,  -0.5 * topLeft.y + 0.25,    2 * topLeft.z) -  0.5f * leftDiff;
            normals[3] = glm::vec3(2 * topRight.x, -0.5 * topRight.y + 0.25,   2 * topRight.z) - 0.5f * rightDiff;
        } else {
            normals[0] = glm::vec3(2 * topLeft.x,  -0.5 * topLeft.y + 0.25,    2 * topLeft.z);
            normals[3] = glm::vec3(2 * topRight.x, -0.5 * topRight.y + 0.25,   2 * topRight.z);
        }
        normals[1] = glm::vec3(2 * bottomLeft.x,  -0.5 * bottomLeft.y + 0.25,  2 * bottomLeft.z);
        normals[2] = glm::vec3(2 * bottomRight.x, -0.5 * bottomRight.y + 0.25, 2 * bottomRight.z);
    }

    // bottom left triangle
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normals[0]);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normals[1]);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normals[2]);


    // top right triangle
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normals[2]);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normals[3]);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normals[0]);
}

void Cone::makeCapWedge(bool top, float currentTheta, float nextTheta) {
    float r = 0.5f;

    float centerY = top ? 0.5f : -0.5f;
    glm::vec3 center(0.f, centerY, 0.f);
    glm::vec3 normal(0.f, centerY * 2.f, 0.f);

    glm::vec3 edgeVertex1(r * sin(currentTheta),  center.y, r * cos(currentTheta));
    glm::vec3 edgeVertex2(r * sin(nextTheta),     center.y, r * cos(nextTheta));


    glm::vec3 topLeft(center);
    glm::vec3 topRight(center);
    glm::vec3 bottomLeft(r * sin(currentTheta),  center.y, r * cos(currentTheta));
    glm::vec3 bottomRight(r * sin(nextTheta),    center.y, r * cos(nextTheta));

    float step = 1.0f / m_param1;
    glm::vec3 leftDiff = bottomLeft - topLeft;
    glm::vec3 rightDiff = bottomRight - topRight;

    for (int i = 0; i < m_param1; ++i) {
        glm::vec3 tileTopLeft(
            topLeft + step * i * leftDiff
            );
        glm::vec3 tileTopRight(
            topRight + step * i * rightDiff
            );

        glm::vec3 tileBottomLeft(
            topLeft + step * (i + 1) * leftDiff
            );

        glm::vec3 tileBottomRight(
            topRight + step * (i + 1) * rightDiff
            );

        makeTile(tileTopRight, tileTopLeft, tileBottomRight, tileBottomLeft, true, normal);
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    float r = 0.5f;

    // make cap
    makeCapWedge(false, currentTheta, nextTheta);

    // make cone
    glm::vec3 centerTop(0.f, 0.5f, 0.f);

    glm::vec3 center(0.f, -0.5f, 0.f);
    glm::vec3 normal = glm::vec3(0.f, -1.f, 0.f);

    glm::vec3 topLeft(centerTop);
    glm::vec3 topRight(centerTop);
    glm::vec3 bottomLeft(r * sin(currentTheta),  center.y, r * cos(currentTheta));
    glm::vec3 bottomRight(r * sin(nextTheta),    center.y, r * cos(nextTheta));

    float step = 1.0f / m_param1;
    glm::vec3 leftDiff = bottomLeft - topLeft;
    glm::vec3 rightDiff = bottomRight - topRight;

    for (int i = 0; i < m_param1; ++i) {
        glm::vec3 tileTopLeft(
            topLeft + step * i * leftDiff
            );
        glm::vec3 tileTopRight(
            topRight + step * i * rightDiff
            );

        glm::vec3 tileBottomLeft(
            topLeft + step * (i + 1) * leftDiff
            );

        glm::vec3 tileBottomRight(
            topRight + step * (i + 1) * rightDiff
            );

        if (abs(tileTopLeft.y - 0.5) < 0.001) {
            makeTile(tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight, false, glm::vec3());
        } else {
            makeTile(tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight, false, glm::vec3());
        }
    }

}

void Cone::setVertexData() {
    float thetaStep = glm::radians(360.f / m_param2);
    for (int j = 0; j < m_param2; ++j) { // Loop for the entire circle.
        float currentTheta = j * thetaStep;
        float nextTheta = (j + 1) * thetaStep;

        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
