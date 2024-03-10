/**
 * @file orto_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

/**
 * @class OrtographicCamera
 * 
 * @brief Camera using ortographic projection
 */
class OrtographicCamera {

private:

    // In pixels
    glm::vec3 _position;            /**< Camera position */

    // In degrees
    glm::vec3 _rotation;            /**< Camera rotation */

    // In pixels
    float _left;                    /**< Left clipping plane */
    float _right;                   /**< Right clipping plane */
    float _bottom;                  /**< Bottom clipping plane */
    float _top;                     /**< Top clipping plane */
    float _nearPlane;               /**< Near clipping plane */
    float _farPlane;                /**< Far clipping plane */

    glm::mat4 _viewMatrix;          /**< View matrix */
    glm::mat4 _projectionMatrix;    /**< Projection matrix */

public:

    OrtographicCamera(
        glm::vec3 position,
        glm::vec3 rotation,
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );
    OrtographicCamera(
        glm::vec3 position,
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );
    OrtographicCamera();


    void setPosition(glm::vec3 position);
    void setRotation(glm::vec3 rotation);
    void setProjection(
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

};
