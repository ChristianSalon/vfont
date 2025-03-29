/**
 * @file base_camera.cpp
 * @author Christian Saloň
 */

#include "base_camera.h"

/**
 * @brief BaseCamera constructor
 *
 * @param position Camera position
 * @param rotation Camera rotation
 */
BaseCamera::BaseCamera(glm::vec3 position, glm::vec3 rotation) : _position{position}, _rotation{rotation} {
    this->_setViewMatrix();
}

/**
 * @brief Translates the camera position
 *
 * @param translation Translation vector (delta)
 */
void BaseCamera::translate(glm::vec3 translation) {
    static const float translateFactor = 0.5;

    this->_position += translation * translateFactor;
    this->_setViewMatrix();
}

/**
 * @brief Rotates camera
 *
 * @param rotation Rotation vector (delta)
 */
void BaseCamera::rotate(glm::vec3 rotation) {
    static const float rotateFactor = 0.2;

    this->_rotation += rotation * rotateFactor;
    this->_setViewMatrix();
}

/**
 * @brief Zooms the camera based on camera direction
 *
 * @param delta By how much to zoom
 */
void BaseCamera::zoom(float delta) {
    static const float zoomFactor = 0.4;

    this->_position += this->_direction * delta * zoomFactor;
    this->_setViewMatrix();
}

/**
 * @brief Set new camera position
 *
 * @param position New position
 */
void BaseCamera::setPosition(glm::vec3 position) {
    this->_position = position;
    this->_setViewMatrix();
}

/**
 * @brief Set new camera rotation
 *
 * @param rotation New rotation
 */
void BaseCamera::setRotation(glm::vec3 rotation) {
    this->_rotation = rotation;
    this->_setViewMatrix();
}

/**
 * @brief Get view matrix based on camera properties
 *
 * @return View matrix
 */
glm::mat4 BaseCamera::getViewMatrix() const {
    return this->_viewMatrix;
}

/**
 * @brief Get projection matrix based on camera properties
 *
 * @return Projection matrix
 */
glm::mat4 BaseCamera::getProjectionMatrix() const {
    return this->_projectionMatrix;
}

/**
 * @brief Update view matrix after changing camera properties
 */
void BaseCamera::_setViewMatrix() {
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4{1.f}, glm::radians(this->_rotation.x), glm::vec3{1.f, 0.f, 0.f}) *
                               glm::rotate(glm::mat4{1.f}, glm::radians(this->_rotation.y), glm::vec3{0.f, 1.f, 0.f}) *
                               glm::rotate(glm::mat4{1.f}, glm::radians(this->_rotation.z), glm::vec3{0.f, 0.f, 1.f});

    this->_direction = glm::vec3(rotationMatrix * glm::vec4(0, 0, 1, 0));
    this->_right = glm::vec3(rotationMatrix * glm::vec4(1, 0, 0, 0));
    this->_up = glm::vec3(rotationMatrix * glm::vec4(0, 1, 0, 0));
    this->_target = this->_position + this->_direction;

    this->_viewMatrix = glm::lookAtLH(this->_position, this->_target, this->_up);
}
