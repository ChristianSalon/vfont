/**
 * @file base_camera.cpp
 * @author Christian Saloň
 */

#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "base_camera.h"

BaseCamera::BaseCamera(glm::vec3 position, glm::vec3 rotation) {
    this->_direction = glm::vec3(0.f, 0.f, -1.f);
    this->_target = glm::vec3(0.f, 0.f, position.z + 1.f);
    this->_right = glm::vec3(1.f, 0.f, 0.f);
    this->_up = glm::vec3(0.f, -1.f, 0.f);

    this->setRotation(rotation);
    this->setPosition(position);
}

void BaseCamera::translate(glm::vec3 translation) {
    this->_position += translation;
    this->_setViewMatrix();
}

void BaseCamera::rotate(glm::vec3 rotation) {
    this->_rotation += rotation;
    this->_setViewMatrix();
}

void BaseCamera::setPosition(glm::vec3 position) {
    this->_position = position;
    this->_setViewMatrix();
}

void BaseCamera::setRotation(glm::vec3 rotation) {
    this->_rotation = rotation;
    this->_setViewMatrix();
}

 glm::mat4 BaseCamera::getViewMatrix() const {
    return this->_viewMatrix;
}

glm::mat4 BaseCamera::getProjectionMatrix() const {
    return this->_projectionMatrix;
}

void BaseCamera::_setViewMatrix() {
    this->_direction.x = cos(glm::radians(this->_rotation.y)) * cos(glm::radians(this->_rotation.x));
    this->_direction.y = sin(glm::radians(this->_rotation.x));
    this->_direction.z = sin(glm::radians(this->_rotation.y)) * cos(glm::radians(this->_rotation.x));
    this->_direction = glm::normalize(this->_direction);

    this->_right = glm::normalize(glm::cross(this->_direction, this->_up));
    this->_up = glm::normalize(glm::cross(this->_right, this->_direction));
    this->_target = this->_position - this->_direction;

    this->_viewMatrix = glm::lookAt(this->_position, this->_target, this->_up);

    /* std::cout << "Position = (" << this->_position.x << ", " << this->_position.y << ", " << this->_position.z << ")" << std::endl
        << "Direction = (" << this->_direction.x << ", " << this->_direction.y << ", " << this->_direction.z << ")" << std::endl
        << "Target = (" << this->_target.x << ", " << this->_target.y << ", " << this->_target.z << ")" << std::endl
        << "Right = (" << this->_right.x << ", " << this->_right.y << ", " << this->_right.z << ")" << std::endl
        << "Up = (" << this->_up.x << ", " << this->_up.y << ", " << this->_up.z << ")" << std::endl; */
}
