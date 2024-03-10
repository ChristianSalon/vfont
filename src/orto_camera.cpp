/**
 * @file orto_camera.cpp
 * @author Christian Saloň
 */

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "orto_camera.h"

/**
 * @brief Ortographic camera constructor
 * 
 * @param position Camera position
 * @param rotation Camera rotation
 * @param left Left clipping plane
 * @param right Right clipping plane
 * @param bottom Bottom clipping plane
 * @param top Top clipping plane
 * @param nearPalne Near clipping plane
 * @param farPlane Far clipping plane
 */
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

/**
 * @brief Ortographic camera constructor with no rotation applied
 * 
 * @param position Camera position
 * @param left Left clipping plane
 * @param right Right clipping plane
 * @param bottom Bottom clipping plane
 * @param top Top clipping plane
 * @param nearPalne Near clipping plane
 * @param farPlane Far clipping plane
 */
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

/**
 * @brief Ortographic camera constructor
 */
OrtographicCamera::OrtographicCamera() {
    this->setPosition(glm::vec3(0.f, 0.f, 0.f));
    this->setRotation(glm::vec3(0.f, 0.f, 0.f));
    this->setProjection(-1.f, 1.f, -1.f, 1.f, 0.f, 1.f);
}

/**
 * @brief Setter for camera position
 * 
 * @param position New camera position
 */
void OrtographicCamera::setPosition(glm::vec3 position) {
    this->_position = position;
}

/**
 * @brief Setter for camera rotation
 * 
 * @param position New camera rotation
 */
void OrtographicCamera::setRotation(glm::vec3 rotation) {
    this->_rotation = rotation;

    this->_viewMatrix = glm::lookAt(this->_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
}

/**
 * @brief Setter for camera projection
 * 
 * @param left Left clipping plane
 * @param right Right clipping plane
 * @param bottom Bottom clipping plane
 * @param top Top clipping plane
 * @param nearPalne Near clipping plane
 * @param farPlane Far clipping plane
 */
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

/**
 * @brief Getter for camera view matrix
 * 
 * @return View matrix
 */
 glm::mat4 OrtographicCamera::getViewMatrix() const {
    return this->_viewMatrix;
}

/**
 * @brief Getter for camera projection matrix
 * 
 * @return Projection matrix
 */
glm::mat4 OrtographicCamera::getProjectionMatrix() const {
    return this->_projectionMatrix;
}
