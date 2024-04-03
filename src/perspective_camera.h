/**
 * @file perspective_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/vec3.hpp>

#include "base_camera.h"

class PerspectiveCamera : public BaseCamera {

private:

    float _fov; // In degrees
    float _aspectRatio;

    // In pixels
    float _nearPlane;
    float _farPlane;

public:

    PerspectiveCamera(
        glm::vec3 position,
        glm::vec3 rotation,
        float fov,
        float aspectRatio,
        float nearPlane,
        float farPlane
    );
    PerspectiveCamera(
        glm::vec3 position,
        float fov,
        float aspectRatio,
        float nearPlane,
        float farPlane
    );

    void setProjection(
        float fov,
        float aspectRatio,
        float nearPlane,
        float farPlane
    );

};
