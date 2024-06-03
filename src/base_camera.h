/**
 * @file base_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

/**
 * @brief Defines all camera types implemented in demo app
 */
enum class CameraType {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

/**
 * @class BaseCamera
 * 
 * @brief Base class for all cameras
 */
class BaseCamera {

protected:

    glm::vec3 _position;            /**< Camera position in pixels */
    glm::vec3 _rotation;            /**< Camera rotation in degrees */
    glm::vec3 _direction;
    glm::vec3 _target;
    glm::vec3 _up;
    glm::vec3 _right;

    glm::mat4 _viewMatrix;          /**< Computed view matrix */
    glm::mat4 _projectionMatrix;    /**< Computed projection matrix */

public:

    BaseCamera(glm::vec3 position, glm::vec3 rotation);

    void translate(glm::vec3 translation);
    void rotate(glm::vec3 rotation);

    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

protected:

    void _setViewMatrix();

};
