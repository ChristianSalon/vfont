/**
 * @file ortho_camera.cpp
 * @author Christian Saloň
 */

#include "ortho_camera.h"

/**
 * @brief OrthographicCamera constructor
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
OrthographicCamera::OrthographicCamera(glm::vec3 position,
                                       glm::vec3 rotation,
                                       float left,
                                       float right,
                                       float bottom,
                                       float top,
                                       float nearPlane,
                                       float farPlane)
    : BaseCamera{position, rotation} {
    this->setProjection(left, right, bottom, top, nearPlane, farPlane);
}

/**
 * @brief OrthographicCamera constructor with no rotation applied
 *
 * @param position Camera position
 * @param left Left clipping plane
 * @param right Right clipping plane
 * @param bottom Bottom clipping plane
 * @param top Top clipping plane
 * @param nearPalne Near clipping plane
 * @param farPlane Far clipping plane
 */
OrthographicCamera::OrthographicCamera(glm::vec3 position,
                                       float left,
                                       float right,
                                       float bottom,
                                       float top,
                                       float nearPlane,
                                       float farPlane)
    : BaseCamera{position, glm::vec3(0.f, 0.f, 0.f)} {
    this->setProjection(left, right, bottom, top, nearPlane, farPlane);
}

/**
 * @brief OrthographicCamera constructor
 */
OrthographicCamera::OrthographicCamera() : BaseCamera{glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f)} {
    this->setProjection(-1.f, 1.f, -1.f, 1.f, 0.f, 1.f);
}

/**
 * @brief Update projection matrix based on clipping planes
 *
 * @param left Left clipping plane
 * @param right Right clipping plane
 * @param bottom Bottom clipping plane
 * @param top Top clipping plane
 * @param nearPalne Near clipping plane
 * @param farPlane Far clipping plane
 */
void OrthographicCamera::setProjection(float left,
                                       float right,
                                       float bottom,
                                       float top,
                                       float nearPlane,
                                       float farPlane) {
    this->_left = left;
    this->_right = right;
    this->_bottom = bottom;
    this->_top = top;
    this->_nearPlane = nearPlane;
    this->_farPlane = farPlane;

    this->_projectionMatrix =
        glm::orthoLH_ZO(this->_left, this->_right, this->_bottom, this->_top, this->_nearPlane, this->_farPlane);
}
