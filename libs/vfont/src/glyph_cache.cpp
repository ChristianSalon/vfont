/**
 * @file glyph_cache.cpp
 * @author Christian Saloň
 */

#include "glyph_cache.h"

namespace vft {

GlyphKey::GlyphKey(std::string fontName, uint32_t codePoint) : fontName{ fontName }, codePoint{ codePoint } {}

GlyphCache::GlyphCache(unsigned long maxSize) : _maxSize{ maxSize } {}

GlyphCache::GlyphCache() : _maxSize{ULONG_MAX} {}

void GlyphCache::setGlyph(GlyphKey key, Glyph glyph) {
    if (!this->exists(key) && this->_cache.size() == this->_maxSize) {
        return;
    }

    this->_cache.insert({ key, glyph });
}

const Glyph &GlyphCache::getGlyph(GlyphKey key) const {
    return this->_cache.at(key);
}

bool GlyphCache::exists(GlyphKey key) const {
    if (this->_cache.find(key) == this->_cache.end()) {
        return false;
    }

    return true;
}

void GlyphCache::clearGlyph(GlyphKey key) {
    this->_cache.erase(key);
}

void GlyphCache::clearAll() {
    this->_cache.clear();
}

}
