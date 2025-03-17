/**
 * @file glyph_cache.cpp
 * @author Christian Saloň
 */

#include "glyph_cache.h"

namespace vft {

/**
 * @brief GlyphKey constructor
 *
 * @param fontName Fonat name of glyph
 * @param glyphId Glyph id of glyph
 * @param fontSize Font size of glyph
 */
GlyphKey::GlyphKey(std::string fontName, uint32_t glyphId, unsigned int fontSize)
    : fontName{fontName}, glyphId{glyphId}, fontSize{fontSize} {}

/**
 * @brief GlyphCache constructor
 *
 * @param maxSize Maximum size of cache
 */
GlyphCache::GlyphCache(unsigned long maxSize) : _maxSize{maxSize} {}

/**
 * @brief GlyphCache constructor
 */
GlyphCache::GlyphCache() : _maxSize{ULONG_MAX} {}

/**
 * @brief Add glyph to cache
 *
 * @param key Key of glyph to be added
 * @param glyph Glyph to be added
 */
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

/**
 * @brief Get glyph from cache
 *
 * @param key Key of glyph
 *
 * @return Glyph stored in cache
 */
const Glyph &GlyphCache::getGlyph(GlyphKey key) {
    if (!this->exists(key)) {
        throw std::runtime_error("GlyphCache::getGlyph(): Glyph cache does not contain selected glyph");
    }

    // Update key to be most recently used (front of list)
    this->_updateToMRU(key);

    return this->_cache.at(key);
}

/**
 * @brief Check whether glyph with given key is in cache
 *
 * @param key Key of glyph
 *
 * @return True if glyph is in cache, else false
 */
bool GlyphCache::exists(GlyphKey key) const {
    if (this->_cache.find(key) == this->_cache.end()) {
        return false;
    }

    return true;
}

/**
 * @brief Remove given glpyh from cache
 *
 * @param key Key of glyph to remove
 */
void GlyphCache::clearGlyph(GlyphKey key) {
    if (!this->exists(key)) {
        return;
    }

    this->_cache.erase(key);
    std::erase(this->_used, key);
}

/**
 * @brief Remove all glyphs from cache
 */
void GlyphCache::clearAll() {
    this->_cache.clear();
}

/**
 * @brief Set maximum size of cache
 *
 * @param maxSize New maximum size
 */
void GlyphCache::setMaxSize(unsigned long maxSize) {
    this->_maxSize = maxSize;

    while (this->_cache.size() > this->_maxSize) {
        this->_eraseLRU();
    }
}

/**
 * @brief Erase the least recently used glyph from cache
 */
void GlyphCache::_eraseLRU() {
    GlyphKey lru = this->_used.back();
    this->_cache.erase(lru);
    this->_used.pop_back();
}

/**
 * @brief Set glyph with given key as most recently used
 *
 * @param key Key of glyph
 */
void GlyphCache::_updateToMRU(GlyphKey key) {
    if (!this->exists(key)) {
        throw std::runtime_error("GlyphCache::_updateToMRU(): Glyph cache does not contain selected glyph");
    }

    this->_used.splice(this->_used.begin(), this->_used, std::find(this->_used.begin(), this->_used.end(), key));
}

}  // namespace vft
