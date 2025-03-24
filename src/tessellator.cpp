/**
 * @file tessellator.cpp
 * @author Christian Saloň
 */

#include "tessellator.h"

namespace vft {

/**
 * @brief Tessellator constructor, initializes freetype outline decompose functions
 */
Tessellator::Tessellator() {
    this->_moveToFunc = [](const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        // Start processing new contour
        glm::vec2 newVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t newVertexIndex = pThis->_getVertexIndex(newVertex);
        if (newVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(newVertex);
            pThis->vertexIndex++;
        }

        // Update glyph data
        pThis->lastVertex = newVertex;
        pThis->lastVertexIndex = newVertexIndex;
        pThis->contourStartVertexIndex = newVertexIndex;
        pThis->contourCount++;

        return 0;
    };

    this->_lineToFunc = [](const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        glm::vec2 newVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t newVertexIndex = pThis->_getVertexIndex(newVertex);
        if (newVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(newVertex);
            pThis->vertexIndex++;
        }

        // Add line segment
        pThis->_currentGlyph.addLineSegment(Edge{pThis->lastVertexIndex, newVertexIndex});

        // Update glyph data
        pThis->lastVertex = newVertex;
        pThis->lastVertexIndex = newVertexIndex;

        return 0;
    };

    this->_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        glm::vec2 controlPoint{static_cast<float>(control->x), static_cast<float>(control->y)};
        uint32_t controlPointIndex = pThis->_getVertexIndex(controlPoint);
        if (controlPointIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(controlPoint);
            pThis->vertexIndex++;
        }

        glm::vec2 endPoint{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t endPointIndex = pThis->_getVertexIndex(endPoint);
        if (endPointIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(endPoint);
            pThis->vertexIndex++;
        }

        // Add quadratic bezier curve segment
        pThis->_currentGlyph.addCurveSegment(Curve{pThis->lastVertexIndex, controlPointIndex, endPointIndex});

        // Update glyph data
        pThis->lastVertex = endPoint;
        pThis->lastVertexIndex = endPointIndex;

        return 0;
    };

    this->_cubicToFunc = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
        throw std::runtime_error("Tessellator::_cubicToFunc(): Fonts with cubic bezier curves are not supported");
        return 0;
    };
}

/**
 * @brief Composes a glyph using freetype
 *
 * @param glyphId Id of glyph to compose
 * @param font Font of glyph
 */
Glyph Tessellator::_composeGlyph(uint32_t glyphId, std::shared_ptr<Font> font) {
    // Get glyph from .ttf file
    if (FT_Load_Glyph(font->getFace(), glyphId, FT_LOAD_NO_SCALE)) {
        throw std::runtime_error("Tessellator::_composeGlyph(): Error loading glyph");
    }
    FT_GlyphSlot slot = font->getFace()->glyph;

    // Initialize member variables
    this->vertexIndex = 0;
    this->lastVertex = glm::vec2{0, 0};
    this->lastVertexIndex = 0;
    this->contourStartVertexIndex = 0;
    this->contourCount = 0;
    this->_currentGlyph = Glyph{};

    // Decompose outlines to vertices and vertex indices
    FT_Outline_Funcs outlineFunctions{.move_to = this->_moveToFunc,
                                      .line_to = this->_lineToFunc,
                                      .conic_to = this->_conicToFunc,
                                      .cubic_to = this->_cubicToFunc,
                                      .shift = 0,
                                      .delta = 0};
    FT_Outline_Decompose(&(slot->outline), &outlineFunctions, this);

    // Set composed glyph metrics
    this->_currentGlyph.setWidth(slot->metrics.width);
    this->_currentGlyph.setHeight(slot->metrics.height);
    this->_currentGlyph.setBearingX(slot->metrics.horiBearingX);
    this->_currentGlyph.setBearingY(slot->metrics.horiBearingY);
    this->_currentGlyph.setAdvanceX(slot->advance.x);
    this->_currentGlyph.setAdvanceY(slot->advance.y);

    return this->_currentGlyph;
}

/**
 * @brief Get index of given vertex from vertices of currently composed glyph
 *
 * @param vertex Given vertex
 *
 * @return Index of given vertex
 */
uint32_t Tessellator::_getVertexIndex(const glm::vec2 &vertex) {
    uint32_t i = 0;

    for (const glm::vec2 &glyphVertex : this->_currentGlyph.mesh.getVertices()) {
        if (glm::distance(vertex, glyphVertex) <= 1.f) {
            return i;
        }

        i++;
    }

    return this->vertexIndex;
}

}  // namespace vft
