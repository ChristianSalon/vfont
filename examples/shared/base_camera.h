/**
 * @file base_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

/**
 * @brief Defines all camera types implemented in demo app
 */
enum class CameraType { PERSPECTIVE, ORTHOGRAPHIC };

/**
 * @brief Base class for all cameras
 */
class BaseCamera {
protected:
    glm::vec3 _position{0.f, 0.f, -1.f}; /**< Camera position in pixels */
    glm::vec3 _rotation{0.f, 0.f, 0.f};  /**< Camera rotation in degrees */
    glm::vec3 _direction{0.f, 0.f, 1.f};
    glm::vec3 _target{0.f, 0.f, 0.f};
    glm::vec3 _up{0.f, 1.f, 0.f};
    glm::vec3 _right{1.f, 0.f, 0.f};

    glm::mat4 _viewMatrix{1.f};       /**< Computed view matrix */
    glm::mat4 _projectionMatrix{1.f}; /**< Computed projection matrix */

public:
    BaseCamera(glm::vec3 position, glm::vec3 rotation);

    void translate(glm::vec3 translation);
    void rotate(glm::vec3 rotation);

    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void zoom(float delta);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

protected:
    void _setViewMatrix();
};
