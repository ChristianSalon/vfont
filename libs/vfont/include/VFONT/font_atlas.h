/**
 * @file font_atlas.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <glm/vec2.hpp>

#include "font.h"

namespace vft {

class FontAtlas {
public:
    class CharacterRange {
    public:
        char32_t start;
        char32_t end;

        CharacterRange(char32_t start, char32_t end) : start{start}, end{end} {}

        unsigned int size() const { return end - start + 1; }
    };

    struct GlyphInfo {
        glm::vec2 uvTopLeft;
        glm::vec2 uvBottomRight;
    };

protected:
    std::string _fontFamily{};
    unsigned int _width{1024};
    unsigned int _height{1024};

    std::vector<uint8_t> _texture{};
    std::unordered_map<uint32_t, GlyphInfo> _glyphs{{0, GlyphInfo{glm::vec2{0, 0}, glm::vec2{0, 0}}}};

public:
    FontAtlas(std::shared_ptr<Font> font, std::vector<uint32_t> glyphIds);
    FontAtlas(std::shared_ptr<Font> font, std::vector<CharacterRange> characterRanges);
    FontAtlas(std::shared_ptr<Font> font, std::u32string characters);
    FontAtlas(std::shared_ptr<Font> font);
    ~FontAtlas() = default;

    GlyphInfo getGlyph(uint32_t glyphId) const;

    std::string getFontFamily() const;
    glm::uvec2 getSize() const;
    const std::vector<uint8_t> &getTexture() const;

protected:
    std::vector<uint32_t> _getAllGlyphIds(std::shared_ptr<Font> font) const;
    std::vector<uint32_t> _getRangesGlyphIds(std::shared_ptr<Font> font,
                                             std::vector<CharacterRange> characterRanges) const;
    std::vector<uint32_t> _getUtf32GlyphIds(std::shared_ptr<Font> font, std::u32string characters) const;
};

}  // namespace vft