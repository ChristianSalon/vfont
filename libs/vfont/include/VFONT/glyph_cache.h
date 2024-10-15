/**
 * @file glyph_cache.h
 * @author Christian Saloň
 */

#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <climits>
#include <functional>

#include "glyph.h"

namespace vft {

class GlyphKey {

public:

    std::string fontName;
    uint32_t codePoint;

    GlyphKey(std::string fontName, uint32_t codePoint);

    bool operator==(const GlyphKey &rhs) const = default;

};

struct GlyphKeyHash {
    std::size_t operator()(const GlyphKey& key) const {
        std::size_t fontNameHash = std::hash<std::string>()(key.fontName);
        std::size_t codePointHash = std::hash<uint32_t>()(key.codePoint);
        return codePointHash ^ (fontNameHash << 1);
    }
};

class GlyphCache {

private:

    std::unordered_map<GlyphKey, Glyph, GlyphKeyHash> _cache{};
    unsigned long _maxSize{ULONG_MAX};

public:

    GlyphCache(unsigned long maxSize);
    GlyphCache();
    ~GlyphCache() = default;

    void setGlyph(GlyphKey key, Glyph glyph);
    const Glyph &getGlyph(GlyphKey key) const;
    bool exists(GlyphKey key) const;

    void clearGlyph(GlyphKey key);
    void clearAll();

};

}
