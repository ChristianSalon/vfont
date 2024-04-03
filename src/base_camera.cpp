/**
 * @file base_camera.cpp
 * @author Christian Saloň
 */

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "base_camera.h"

BaseCamera::BaseCamera(glm::vec3 position, glm::vec3 rotation) {
    this->setPosition(position);
    this->setRotation(rotation);
}

void BaseCamera::setPosition(glm::vec3 position) {
    this->_position = position;
}

void BaseCamera::setRotation(glm::vec3 rotation) {
    this->_rotation = rotation;
    this->_viewMatrix = glm::lookAt(this->_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
}

 glm::mat4 BaseCamera::getViewMatrix() const {
    return this->_viewMatrix;
}

glm::mat4 BaseCamera::getProjectionMatrix() const {
    return this->_projectionMatrix;
}
