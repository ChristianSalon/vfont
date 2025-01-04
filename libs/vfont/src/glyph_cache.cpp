/**
 * @file glyph_cache.cpp
 * @author Christian Saloň
 */

#include "glyph_cache.h"

namespace vft {

GlyphKey::GlyphKey(std::string fontName, uint32_t codePoint, unsigned int fontSize)
    : fontName{fontName}, codePoint{codePoint}, fontSize{fontSize} {}

GlyphCache::GlyphCache(unsigned long maxSize) : _maxSize{maxSize} {}

GlyphCache::GlyphCache() : _maxSize{ULONG_MAX} {}

void GlyphCache::setGlyph(GlyphKey key, Glyph glyph) {
    if (this->_maxSize <= 0) {
        return;
    }

    if (this->exists(key)) {
        // Update key to be most recently used (front of list)
        this->_updateToMRU(key);
        return;
    }

    if (this->_cache.size() >= this->_maxSize) {
        this->_eraseLRU();
    }

    this->_cache.insert({key, glyph});
    this->_used.push_front(key);
}

const Glyph &GlyphCache::getGlyph(GlyphKey key) {
    if (!this->exists(key)) {
        throw std::runtime_error("Glyph cache does not contain selected glyph.");
    }

    // Update key to be most recently used (front of list)
    this->_updateToMRU(key);

    return this->_cache.at(key);
}

bool GlyphCache::exists(GlyphKey key) {
    if (this->_cache.find(key) == this->_cache.end()) {
        return false;
    }

    return true;
}

void GlyphCache::clearGlyph(GlyphKey key) {
    if (!this->exists(key)) {
        return;
    }

    this->_cache.erase(key);
    std::erase(this->_used, key);
}

void GlyphCache::clearAll() {
    this->_cache.clear();
}

void GlyphCache::setMaxSize(unsigned long maxSize) {
    this->_maxSize = maxSize;

    while (this->_cache.size() > this->_maxSize) {
        this->_eraseLRU();
    }
}

void GlyphCache::_eraseLRU() {
    GlyphKey lru = this->_used.back();
    this->_cache.erase(lru);
    this->_used.pop_back();
}

void GlyphCache::_updateToMRU(GlyphKey key) {
    if (!this->exists(key)) {
        throw std::runtime_error("Glyph cache does not contain selected glyph.");
    }

    this->_used.splice(this->_used.begin(), this->_used, std::find(this->_used.begin(), this->_used.end(), key));
}

}  // namespace vft
