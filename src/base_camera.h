/**
 * @file base_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

enum class CameraType {
    PERSPECTIVE,
    ORTOGRAPHIC
};

class BaseCamera {

protected:

    // In pixels
    glm::vec3 _position;

    // In degrees
    glm::vec3 _rotation;

    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;

public:

    BaseCamera(glm::vec3 position, glm::vec3 rotation);

    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

};
