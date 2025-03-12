/**
 * @file tessellator.cpp
 * @author Christian Saloň
 */

#include "tessellator.h"

namespace vft {

Tessellator::Tessellator() {
    this->_moveToFunc = [](const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        // Start processing new contour
        glm::vec2 newVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        long newVertexIndex = pThis->_getVertexIndex(newVertex);

        if (newVertexIndex < 0) {
            pThis->_currentGlyph.mesh.addVertex(newVertex);
            pThis->_currentGlyphData.contourStartVertexId = pThis->_currentGlyphData.vertexId;
            pThis->_currentGlyphData.vertexId++;
        } else {
            pThis->_currentGlyphData.contourStartVertexId = newVertexIndex;
        }

        pThis->_currentGlyphData.lastVertex = newVertex;
        pThis->_currentGlyphData.contourCount++;
        return 0;
    };

    this->_lineToFunc = [](const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        uint32_t lastVertexIndex = pThis->_getVertexIndex(pThis->_currentGlyphData.lastVertex);

        glm::vec2 newVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        long newVertexIndex = pThis->_getVertexIndex(newVertex);
        if (newVertexIndex < 0) {
            pThis->_currentGlyph.mesh.addVertex(newVertex);
            newVertexIndex = pThis->_currentGlyphData.vertexId;
            pThis->_currentGlyphData.vertexId++;
        }

        // Add line segment
        pThis->_currentGlyph.addLineSegment(Edge{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)});

        pThis->_currentGlyphData.lastVertex = newVertex;
        return 0;
    };

    this->_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        Tessellator *pThis = reinterpret_cast<Tessellator *>(user);

        uint32_t startPointIndex = pThis->_getVertexIndex(pThis->_currentGlyphData.lastVertex);
        glm::vec2 startPoint = pThis->_currentGlyph.mesh.getVertices().at(startPointIndex);

        glm::vec2 controlPoint{static_cast<float>(control->x), static_cast<float>(control->y)};
        long controlPointIndex = pThis->_getVertexIndex(controlPoint);
        if (controlPointIndex < 0) {
            pThis->_currentGlyph.mesh.addVertex(controlPoint);
            controlPointIndex = pThis->_currentGlyphData.vertexId;
            pThis->_currentGlyphData.vertexId++;
        }

        glm::vec2 endPoint{static_cast<float>(to->x), static_cast<float>(to->y)};
        long endPointIndex = pThis->_getVertexIndex(endPoint);
        if (endPointIndex < 0) {
            pThis->_currentGlyph.mesh.addVertex(endPoint);
            endPointIndex = pThis->_currentGlyphData.vertexId;
            pThis->_currentGlyphData.vertexId++;
        }

        // Add quadratic bezier curve segment
        pThis->_currentGlyph.addCurveSegment(
            Curve{startPointIndex, static_cast<uint32_t>(controlPointIndex), static_cast<uint32_t>(endPointIndex)});

        pThis->_currentGlyphData.lastVertex = endPoint;
        return 0;
    };

    this->_cubicToFunc = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
        throw std::runtime_error("Tessellator::_cubicToFunc(): Fonts with cubic bezier curves are not supported");
        return 0;
    };
}

/**
 * @brief Creates a triangulated glyph if not in cache and inserts glyph into cache
 *
 * @param codePoint Unicode code point of glyph to triangulate
 * @param font Font to use for triangulation
 */
Glyph Tessellator::_composeGlyph(uint32_t glyphId, std::shared_ptr<Font> font) {
    // Get glyph from .ttf file
    if (FT_Load_Glyph(font->getFace(), glyphId, FT_LOAD_NO_SCALE)) {
        throw std::runtime_error("Tessellator::_composeGlyph(): Error loading glyph");
    }
    FT_GlyphSlot slot = font->getFace()->glyph;

    // Decompose outlines to vertices and vertex indices
    this->_currentGlyph = Glyph{};
    this->_currentGlyphData = ComposedGlyphData{};
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

long Tessellator::_getVertexIndex(const glm::vec2 &vertex) {
    long i = 0;

    for (const glm::vec2 &glyphVertex : this->_currentGlyph.mesh.getVertices()) {
        if (glm::distance(vertex, glyphVertex) <= 1.f) {
            return i;
        }

        i++;
    }

    return -1;
}

}  // namespace vft
