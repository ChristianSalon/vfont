/**
 * @file font_atlas.cpp
 * @author Christian Saloň
 */

#include "font_atlas.h"

namespace vft {

/**
 * @brief Construct texture containing sdfs of selected glyphs using the freetype sdf rasterizer
 *
 * @param font Font used to rasterize glyphs into sdfs
 * @param glyphIds Glyph indices to rasterize
 */
FontAtlas::FontAtlas(std::shared_ptr<Font> font, std::vector<uint32_t> glyphIds) : _fontFamily{font->getFontFamily()} {
    // Set size of each glyph bitmap to approximetly 64 x 64
    // Freetype does not produce exact size of bitmap
    unsigned int oldPixelSize = font->getPixelSize();
    font->setPixelSize(64);

    // Get the size of texture needed to store all glyphs
    // Texture is a square where length is a power of two
    unsigned area = glyphIds.size() * 64 * 64;
    unsigned int size = 128;
    while (area * 1.5 > size * size) {
        size *= 2;
    }

    // Reset to old pixel size
    font->setPixelSize(oldPixelSize);

    // Set texture width and height
    this->_width = size;
    this->_height = size;

    // Initialize texture to zeros
    this->_texture.assign(this->_width * this->_height, 0);

    glm::uvec2 pen{0, 0};  // Current position in texture
    unsigned int currentRowHeight = 0;

    // Load all glyphs
    for (uint32_t glyphId : glyphIds) {
        // Generate sdf bitmap for glyph
        if (FT_Load_Glyph(font->getFace(), glyphId, FT_LOAD_RENDER)) {
            throw std::runtime_error("FontAtlas::FontAtlas(): Error loading glyph");
        }
        if (FT_Render_Glyph(font->getFace()->glyph, FT_RENDER_MODE_SDF)) {
            throw std::runtime_error("FontAtlas::FontAtlas(): Error rasterizing sdf bitmap");
        }
        FT_GlyphSlot slot = font->getFace()->glyph;
        const FT_Bitmap &bitmap = slot->bitmap;

        if (bitmap.width == 0 || bitmap.rows == 0) {
            this->_glyphs.insert({glyphId, GlyphInfo{glm::vec2{0, 0}, glm::vec2{0, 0}}});
            continue;
        }

        // Check if glyph will fit on current row in texture
        if (pen.x + bitmap.width > this->_width) {
            pen.x = 0;
            pen.y += currentRowHeight;
            currentRowHeight = 0;
        }

        // Bounding box of glyph contours
        unsigned int xMin = bitmap.width;
        unsigned int xMax = 0;
        unsigned int yMin = bitmap.rows;
        unsigned int yMax = 0;

        // Write bitmap data into atlas texture
        for (unsigned int y = 0; y < bitmap.rows; y++) {
            for (unsigned int x = 0; x < bitmap.width; x++) {
                uint8_t pixel = bitmap.buffer[y * bitmap.width + x];
                this->_texture[(pen.y + y) * this->_width + (pen.x + x)] = pixel;

                // Check if current pixel is in glyph (127 represents that pixel is on contour)
                if (pixel >= 127) {
                    xMin = std::max(std::min(x, xMin), static_cast<unsigned int>(0));
                    xMax = std::min(std::max(x, xMax), bitmap.width);
                    yMin = std::max(std::min(y, yMin), static_cast<unsigned int>(0));
                    yMax = std::min(std::max(y, yMax), bitmap.rows);
                }
            }
        }

        // Insert glyph data
        glm::vec2 uvTopLeft{(pen.x + xMin - 1) / static_cast<float>(this->_width),
                            (pen.y + yMin - 1) / static_cast<float>(this->_height)};
        glm::vec2 uvBottomRight{(pen.x + xMax + 1) / static_cast<float>(this->_width),
                                (pen.y + yMax + 1) / static_cast<float>(this->_height)};
        this->_glyphs.insert({glyphId, GlyphInfo{uvTopLeft, uvBottomRight}});

        // Update texture pen position
        pen.x += bitmap.width;
        currentRowHeight = std::max(currentRowHeight, bitmap.rows);
    }
}

/**
 * @brief Construct texture containing sdfs of selected unicode code points using the freetype sdf rasterizer
 *
 * @param font Font used to rasterize glyphs into sdfs
 * @param characterRanges Ranges of unicode code points to rasterize
 */
FontAtlas::FontAtlas(std::shared_ptr<Font> font, std::vector<CharacterRange> characterRanges)
    : FontAtlas{font, this->_getRangesGlyphIds(font, characterRanges)} {}

/**
 * @brief Construct texture containing sdfs of selected utf-32 encoded characters using the freetype sdf rasterizer
 *
 * @param font Font used to rasterize glyphs into sdfs
 * @param characters Utf-32 encoded characters to rasterize
 */
FontAtlas::FontAtlas(std::shared_ptr<Font> font, std::u32string characters)
    : FontAtlas{font, this->_getUtf32GlyphIds(font, characters)} {}

/**
 * @brief Construct texture containing sdfs of all glyphs in font file using the freetype sdf rasterizer
 *
 * @param font Font used to rasterize glyphs into sdfs
 */
FontAtlas::FontAtlas(std::shared_ptr<Font> font) : FontAtlas{font, this->_getAllGlyphIds(font)} {}

/**
 * @brief Get glyph info (UVs) of selected glyph
 *
 * @param glyphId Glyph index of selected glyph
 *
 * @return Glyph info of selected glyph
 */
FontAtlas::GlyphInfo FontAtlas::getGlyph(uint32_t glyphId) const {
    if (!this->_glyphs.contains(glyphId)) {
        throw std::invalid_argument("FontAtlas::getGlyph(): Font atlas does not contain glyph with given glyph id");
    }

    return this->_glyphs.at(glyphId);
}

/**
 * @brief Getter for font family of font atlas
 *
 * @return FOnt family
 */
std::string FontAtlas::getFontFamily() const {
    return this->_fontFamily;
}

/**
 * @brief Getter for the size of final texture
 *
 * @return Size of texture
 */
glm::uvec2 FontAtlas::getSize() const {
    return glm::uvec2{this->_width, this->_height};
}

/**
 * @brief Get raw bytes of texture
 *
 * @return Vector of bytes containing texture data
 */
const std::vector<uint8_t> &FontAtlas::getTexture() const {
    return this->_texture;
}

/**
 * @brief Get all glyph indices in selected font
 *
 * @param font Font used for loading glyphs
 *
 * @return All glyph indices
 */
std::vector<uint32_t> FontAtlas::_getAllGlyphIds(std::shared_ptr<Font> font) const {
    std::vector<uint32_t> glyphIds;
    uint32_t charcode;
    uint32_t glyphId;

    charcode = FT_Get_First_Char(font->getFace(), &glyphId);
    while (glyphId != 0) {
        glyphIds.push_back(glyphId);
        charcode = FT_Get_Next_Char(font->getFace(), charcode, &glyphId);
    }

    return glyphIds;
}

/**
 * @brief Get glyph indices for the selected ranges of unicode code point
 *
 * @param font Font used for loading glyphs
 * @param characterRanges Ranges of unicode code points
 *
 * @return Glyph indices representing unicode code points
 */
std::vector<uint32_t> FontAtlas::_getRangesGlyphIds(std::shared_ptr<Font> font,
                                                    std::vector<CharacterRange> characterRanges) const {
    std::vector<uint32_t> glyphIds;

    for (const CharacterRange &range : characterRanges) {
        for (unsigned int i = range.start; i <= range.end; i++) {
            glyphIds.push_back(FT_Get_Char_Index(font->getFace(), i));
        }
    }

    return glyphIds;
}

/**
 * @brief Get glyph indices for utf-32 encoded characters
 *
 * @param font Font used for loading glyphs
 * @param characters Utf-32 encoded characters
 *
 * @return Glyph indices representing utf-32 encoded characters
 */
std::vector<uint32_t> FontAtlas::_getUtf32GlyphIds(std::shared_ptr<Font> font, std::u32string characters) const {
    std::vector<uint32_t> glyphIds;

    for (const char32_t &character : characters) {
        glyphIds.push_back(FT_Get_Char_Index(font->getFace(), character));
    }

    return glyphIds;
}

}  // namespace vft