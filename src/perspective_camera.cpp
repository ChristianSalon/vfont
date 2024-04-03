/**
 * @file perspective_camera.cpp
 * @author Christian Saloň
 */

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "perspective_camera.h"

PerspectiveCamera::PerspectiveCamera(
    glm::vec3 position,
    glm::vec3 rotation,
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane
) : BaseCamera{ position, rotation } {
    this->setProjection(fov, aspectRatio, nearPlane, farPlane);
}

PerspectiveCamera::PerspectiveCamera(
    glm::vec3 position,
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane
) : BaseCamera{ position, glm::vec3(0.f, 0.f, 0.f) } {
    this->setProjection(fov, aspectRatio, nearPlane, farPlane);
}

void PerspectiveCamera::setProjection(
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane
) {
    this->_fov = fov;
    this->_aspectRatio = aspectRatio;
    this->_nearPlane = nearPlane;
    this->_farPlane = farPlane;

    this->_projectionMatrix = glm::perspective(glm::radians(this->_fov), this->_aspectRatio, this->_nearPlane, this->_farPlane);
    this->_projectionMatrix[1][1] *= -1;
}
