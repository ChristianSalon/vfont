/**
 * @file orto_camera.cpp
 * @author Christian Saloň
 */

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "orto_camera.h"

OrtographicCamera::OrtographicCamera(
    glm::vec3 position,
    glm::vec3 rotation,
    float left,
    float right,
    float bottom,
    float top,
    float nearPlane,
    float farPlane
) {
    this->setPosition(position);
    this->setRotation(rotation);
    this->setProjection(left, right, bottom, top, nearPlane, farPlane);
}

OrtographicCamera::OrtographicCamera(
    glm::vec3 position,
    float left,
    float right,
    float bottom,
    float top,
    float nearPlane,
    float farPlane
) {
    this->setPosition(position);
    this->setRotation(glm::vec3(0.f, 0.f, 0.f));
    this->setProjection(left, right, bottom, top, nearPlane, farPlane);
}

OrtographicCamera::OrtographicCamera() {
    this->setPosition(glm::vec3(0.f, 0.f, 0.f));
    this->setRotation(glm::vec3(0.f, 0.f, 0.f));
    this->setProjection(-1.f, 1.f, -1.f, 1.f, 0.f, 1.f);
}

void OrtographicCamera::setPosition(glm::vec3 position) {
    this->_position = position;
}

void OrtographicCamera::setRotation(glm::vec3 rotation) {
    this->_rotation = rotation;

    this->_viewMatrix = glm::lookAt(this->_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
}

void OrtographicCamera::setProjection(
    float left,
    float right,
    float bottom,
    float top,
    float nearPlane,
    float farPlane
) {
    this->_left = left;
    this->_right = right;
    this->_bottom = bottom;
    this->_top = top;
    this->_nearPlane = nearPlane;
    this->_farPlane = farPlane;

    this->_projectionMatrix = glm::ortho(this->_left, this->_right, this->_bottom, this->_top, this->_nearPlane, this->_farPlane);
    this->_projectionMatrix[1][1] *= -1;
}

 glm::mat4 OrtographicCamera::getViewMatrix() const {
    return this->_viewMatrix;
}

glm::mat4 OrtographicCamera::getProjectionMatrix() const {
    return this->_projectionMatrix;
}
