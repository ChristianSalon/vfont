/**
 * @file base_camera.cpp
 * @author Christian Saloň
 */

#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "base_camera.h"

/**
 * @brief BaseCamera constructor
 * 
 * @param position Camera position
 * @param rotation Camera rotation
 */
BaseCamera::BaseCamera(glm::vec3 position, glm::vec3 rotation) {
    this->_direction = glm::vec3(0.f, 0.f, -1.f);
    this->_target = glm::vec3(0.f, 0.f, position.z + 1.f);
    this->_right = glm::vec3(1.f, 0.f, 0.f);
    this->_up = glm::vec3(0.f, -1.f, 0.f);

    this->setRotation(rotation);
    this->setPosition(position);
}

/**
 * @brief Translates the camera position
 * 
 * @param translation Translation vector (delta)
 */
void BaseCamera::translate(glm::vec3 translation) {
    this->_position += translation;
    this->_setViewMatrix();
}

/**
 * @brief Rotates camera
 * 
 * @param rotation Rotation vector (delta)
 */
void BaseCamera::rotate(glm::vec3 rotation) {
    this->_rotation += rotation;
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
    this->_direction.x = cos(glm::radians(this->_rotation.y)) * cos(glm::radians(this->_rotation.x));
    this->_direction.y = sin(glm::radians(this->_rotation.x));
    this->_direction.z = sin(glm::radians(this->_rotation.y)) * cos(glm::radians(this->_rotation.x));
    this->_direction = glm::normalize(this->_direction);

    this->_right = glm::normalize(glm::cross(this->_direction, this->_up));
    this->_up = glm::normalize(glm::cross(this->_right, this->_direction));
    this->_target = this->_position - this->_direction;

    this->_viewMatrix = glm::lookAt(this->_position, this->_target, this->_up);
}
