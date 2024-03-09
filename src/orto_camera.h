/**
 * @file orto_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class OrtographicCamera {

private:

    glm::vec3 _position;

    // In degrees
    glm::vec3 _rotation;

    // In pixels
    float _left;
    float _right;
    float _bottom;
    float _top;
    float _nearPlane;
    float _farPlane;

    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;

public:

    OrtographicCamera(
        glm::vec3 position,
        glm::vec3 rotation,
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );
    OrtographicCamera(
        glm::vec3 position,
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );
    OrtographicCamera();


    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void setProjection(
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

};
