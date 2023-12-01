#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    Camera(const SceneCameraData& cameraData, const int width, const int height, const float nearPlane, const float farPlane);

    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix();

    glm::mat4 getProjectionMatrix();

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;

    glm::vec4 getPos();

    void setPlanes(float newNearPlane, float newFarPlane);

    // movement
    void moveForward(float distance, float deltaTime);
    void moveRight(float distance, float deltaTime);
    void moveUp(float speed, float deltaTime);
    void rotateCamera(float deltaX, float deltaY);

private:
    int width;
    int height;
    float nearPlane;
    float farPlane;
    SceneCameraData data;
};
