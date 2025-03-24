/**
 * @file sdf_tessellator.cpp
 * @author Christian Saloň
 */

#include "sdf_tessellator.h"

namespace vft {

/**
 * @brief SdfTessellator constructor
 */
SdfTessellator::SdfTessellator() {}

/**
 * @brief Composes a glyph ready for rendering using sdfs
 *
 * @param glyphId Id of glyph to compose
 * @param font Font of glyph
 * @param fontSize Font size of glyph
 */
Glyph SdfTessellator::composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize) {
    // Get glyph from .ttf file
    if (FT_Load_Glyph(font->getFace(), glyphId, FT_LOAD_RENDER)) {
        throw std::runtime_error("SdfTessellator::composeGlyph(): Error loading glyph");
    }
    if (FT_Render_Glyph(font->getFace()->glyph, FT_RENDER_MODE_SDF)) {
        throw std::runtime_error("FontAtlas::FontAtlas(): Error rasterizing sdf bitmap");
    }
    FT_GlyphSlot slot = font->getFace()->glyph;
    const FT_Bitmap &bitmap = slot->bitmap;

    // Set glyph metrics (in font units)
    glm::vec2 scale = font->getScalingVector(font->getPixelSize());
    this->_currentGlyph = Glyph{};
    this->_currentGlyph.setWidth(bitmap.width / scale.x);
    this->_currentGlyph.setHeight(bitmap.rows / scale.y);
    this->_currentGlyph.setBearingX(slot->bitmap_left / scale.x);
    this->_currentGlyph.setBearingY(slot->bitmap_top / scale.y);
    this->_currentGlyph.setAdvanceX(slot->advance.x / scale.x);
    this->_currentGlyph.setAdvanceY(slot->advance.y/ scale.y);

    std::vector<glm::vec2> vertices;
    std::vector<uint32_t> boundingBoxIndices;

    // Add vertices and indices of bounding box only if glyph has at least one vertex
    if (slot->outline.n_points > 0) {
        // Add bounding box vertices
        std::array<glm::vec2, 4> boundingBox = this->_currentGlyph.getBoundingBox();
        vertices.insert(vertices.end(), boundingBox.begin(), boundingBox.end());

        // Create bounding box indices that form two triangles
        // 1 ---- 2
        // | \    |
        // |  \   |
        // |   \  |
        // |    \ |
        // 0 ---- 3
        boundingBoxIndices = {0, 3, 1, 2, 1, 3};
    }

    GlyphMesh mesh{vertices, {boundingBoxIndices}};
    this->_currentGlyph.mesh = mesh;

    return this->_currentGlyph;
}

}  // namespace vft
