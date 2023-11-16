#include <stdexcept>
#include "camera.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const SceneCameraData &cameraData, const int width, const int height, const float nearPlane, const float farPlane)
    : data(cameraData), width(width), height(height), nearPlane(nearPlane), farPlane(farPlane) {
}

glm::mat4 Camera::getViewMatrix() {
    // relic function so unnecessarily bulky

    glm::mat4 mTrans = glm::mat4(1.0f);
    mTrans[3][0] = -data.pos.x;
    mTrans[3][1] = -data.pos.y;
    mTrans[3][2] = -data.pos.z;

    glm::vec3 up = glm::vec3(data.up);

    glm::vec3 w = glm::normalize(-data.look);
    glm::vec3 v = glm::normalize(up - (glm::dot(up, w) * w));
    glm::vec3 u = glm::cross(v, w);

    glm::mat4 mRot = glm::mat4(1.0f);
    mRot[0][0] = u[0];
    mRot[1][0] = u[1];
    mRot[2][0] = u[2];

    mRot[0][1] = v[0];
    mRot[1][1] = v[1];
    mRot[2][1] = v[2];

    mRot[0][2] = w[0];
    mRot[1][2] = w[1];
    mRot[2][2] = w[2];

    return mRot * mTrans;
}

glm::mat4 Camera::getProjectionMatrix() {
    float fov = getHeightAngle(); // radians
    float aspectRatio = getAspectRatio();
    float f = 1.0f / tan(fov / 2.0f); // cotangent of half the vertical field of view

    glm::mat4 remap = glm::mat4(1.0f);
    remap[2][2] = -2.0f;
    remap[3][2] = -1.0f;

    float c = -nearPlane/farPlane;

    glm::mat4 unhinged = glm::mat4(0.0f);
    unhinged[0][0] = 1.0f;
    unhinged[1][1] = 1.0f;
    unhinged[2][2] = 1.0f / (1.0f + c);
    unhinged[3][2] = -c / (1.0f + c);
    unhinged[2][3] = -1.0f;

    glm::mat4 perspective = glm::mat4(1.0f);
    perspective[0][0] = 1.0f / (farPlane * tan((getHeightAngle() * getAspectRatio()) / 2.0f));
    perspective[1][1] = 1.0f / (farPlane * tan(getHeightAngle() / 2.0f));
    perspective[2][2] = 1.0f / farPlane;

    return remap * unhinged * perspective;
}

float Camera::getAspectRatio() const {
    return static_cast<float>(width) / height;
}

float Camera::getHeightAngle() const {
    return data.heightAngle;
}

glm::vec4 Camera::getPos() {
    return data.pos;
}

float Camera::getFocalLength() const {
    return data.focalLength;
}

float Camera::getAperture() const {
    return data.aperture;
}
