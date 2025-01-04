/**
 * @file glyph_cache.h
 * @author Christian Saloň
 */

#pragma once

#include <algorithm>
#include <climits>
#include <cstdint>
#include <functional>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "glyph.h"

namespace vft {

class GlyphKey {
public:
    std::string fontName;
    uint32_t codePoint;
    unsigned int fontSize;

    GlyphKey(std::string fontName, uint32_t codePoint, unsigned int fontSize);

    bool operator==(const GlyphKey &rhs) const = default;
};

struct GlyphKeyHash {
    std::size_t operator()(const GlyphKey &key) const {
        std::size_t fontNameHash = std::hash<std::string>()(key.fontName);
        std::size_t codePointHash = std::hash<uint32_t>()(key.codePoint);
        std::size_t fontSizeHash = std::hash<unsigned int>()(key.fontSize);
        return fontSizeHash ^ (codePointHash << 1) ^ (fontNameHash << 2);
    }
};

class GlyphCache {
private:
    unsigned long _maxSize{ULONG_MAX};

    std::unordered_map<GlyphKey, Glyph, GlyphKeyHash> _cache{};
    std::list<GlyphKey> _used{};

public:
    GlyphCache(unsigned long maxSize);
    GlyphCache();
    ~GlyphCache() = default;

    void setGlyph(GlyphKey key, Glyph glyph);
    const Glyph &getGlyph(GlyphKey key);
    bool exists(GlyphKey key);

    void clearGlyph(GlyphKey key);
    void clearAll();
    void setMaxSize(unsigned long maxSize);

protected:
    void _eraseLRU();
    void _updateToMRU(GlyphKey key);
};

}  // namespace vft
