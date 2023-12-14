#include <stdexcept>
#include "camera.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "beziercurve.h"

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
    // ratio of tangent and height / tangent and width

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
    perspective[0][0] = 1.0f / (farPlane * getAspectRatio() * (tan(getHeightAngle() / 2.0f)));
    perspective[1][1] = 1.0f / (farPlane * tan(getHeightAngle() / 2.f));
    perspective[2][2] = 1.0f / farPlane;

    return remap * unhinged * perspective;
}

void Camera::setPlanes(float newNearPlane, float newFarPlane) {
    nearPlane = newNearPlane;
    farPlane = newFarPlane;
}

float Camera::getAspectRatio() const {
    return (float) width / height;
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

void Camera::moveForward(float speed, float deltaTime) {
    glm::vec3 forward = glm::normalize(-glm::vec3(data.look));

    float distance = speed * deltaTime;

    data.pos -= glm::vec4(forward * distance, 0.0f);
}

void Camera::moveRight(float speed, float deltaTime) {
    glm::vec3 forward = glm::normalize(-glm::vec3(data.look));
    glm::vec3 up = glm::normalize(glm::vec3(data.up));
    glm::vec3 right = glm::cross(up, forward);

    float distance = speed * deltaTime;

    data.pos += glm::vec4(right * distance, 0.0f);
}

void Camera::moveUp(float speed, float deltaTime) {
    glm::vec3 upDirection = glm::normalize(glm::vec3(data.up));

    float distance = speed * deltaTime;

    data.pos += glm::vec4(upDirection * distance, 0.0f);
}

glm::mat4 createRotationMatrix(float angle, const glm::vec3& axis) {
    // glm::rotate not allowed...
    glm::vec3 normalizedAxis = glm::normalize(axis);
    float x = normalizedAxis.x;
    float y = normalizedAxis.y;
    float z = normalizedAxis.z;
    float c = cos(angle);
    float s = sin(angle);

    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix[0][0] = c + (1 - c) * x * x;
    rotationMatrix[0][1] = (1 - c) * x * y - s * z;
    rotationMatrix[0][2] = (1 - c) * x * z + s * y;

    rotationMatrix[1][0] = (1 - c) * y * x + s * z;
    rotationMatrix[1][1] = c + (1 - c) * y * y;
    rotationMatrix[1][2] = (1 - c) * y * z - s * x;

    rotationMatrix[2][0] = (1 - c) * z * x - s * y;
    rotationMatrix[2][1] = (1 - c) * z * y + s * x;
    rotationMatrix[2][2] = c + (1 - c) * z * z;

    return rotationMatrix;
}



void Camera::rotateCamera(float deltaX, float deltaY) {
    float sensitivity = 0.001f; // sens

    // yaw
    glm::mat4 yawMatrix = createRotationMatrix(-deltaX * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
    data.look = yawMatrix * data.look;
    data.up = yawMatrix * data.up;

    // pitch
    glm::vec3 right = glm::cross(glm::vec3(data.up), glm::vec3(data.look));
    glm::mat4 pitchMatrix = createRotationMatrix(deltaY * sensitivity, right);
    data.look = pitchMatrix * data.look;
    data.up = pitchMatrix * data.up;

    data.look = glm::normalize(data.look);
    data.up = glm::normalize(data.up);
}




void Camera::updatePosition(float deltaTime) {
    float t = 0.0f;
    BezierCurve bezier;

    bezier.setControlPoints((10,20,20), (40,20,40), (70,20,80), (90,20,100));

    glm::vec3 cameraPosition = bezier.CalculatePoint(t);

    // Update the camera view matrix
    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, targetPosition, upVector);

    float distance = speed * deltaTime;

    t += 0.01f * deltaTime; // Adjust the factor as needed

    data.pos += glm::vec4(cameraPosition * distance, 0.0f);
}
