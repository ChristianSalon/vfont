/**
 * @file glyph.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <array>
#include <cstdint>

#include <glm/vec2.hpp>

#include "glyph_mesh.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class Glyph
 *
 * @brief Represents glyph vertex data and metrics expressed in font units
 */
class Glyph {

public:

    GlyphMesh mesh;

protected:

    std::vector<vft::Curve> _curveSegments;
    std::vector<vft::Edge> _lineSegments;

    long _width;
    long _height;

    long _bearingX;
    long _bearingY;

    /**
     * Indicates by how much to increment the X coordinate of pen position.
     * This includes the glyph's width plus the space behind.
     * Used for horizontal layouts
     */
    long _advanceX;

    /**
     * Indicates by how much to increment the Y coordinate of pen position.
     * Not specified for horizontal layouts
     */
    long _advanceY;

public:

    Glyph();

    void addLineSegment(vft::Edge edge);
    void addCurveSegment(vft::Curve curve);

    std::array<glm::vec2, 4> getBoundingBox() const;

    void setWidth(long width);
    void setHeight(long height);
    void setBearingX(long bearingX);
    void setBearingY(long bearingY);
    void setAdvanceX(long advanceX);
    void setAdvanceY(long advanceY);


    const std::vector<vft::Edge> &getLineSegmentsIndices() const;
    const std::vector<vft::Curve> &getCurveSegmentsIndices() const;
    uint32_t getLineSegmentsIndexCount() const;
    uint32_t getCurveSegmentsIndexCount() const;

    long getAdvanceX() const;
    long getAdvanceY() const;

};

}
