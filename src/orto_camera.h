/**
 * @file orto_camera.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/vec3.hpp>

#include "base_camera.h"

/**
 * @class OrtographicCamera
 * 
 * @brief Camera using ortographic projection
 */

class OrtographicCamera : public BaseCamera {

private:
    // In pixels
    float _left;                    /**< Left clipping plane */
    float _right;                   /**< Right clipping plane */
    float _bottom;                  /**< Bottom clipping plane */
    float _top;                     /**< Top clipping plane */
    float _nearPlane;               /**< Near clipping plane */
    float _farPlane;                /**< Far clipping plane */
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

    void setProjection(
        float left,
        float right,
        float bottom,
        float top,
        float nearPlane,
        float farPlane
    );

};
