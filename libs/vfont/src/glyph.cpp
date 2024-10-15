/**
 * @file glyph.cpp
 * @author Christian Saloň
 */

#include "glyph.h"

namespace vft {

/**
 * @brief Glyph constructor
 */
Glyph::Glyph() : _advanceX{0}, _advanceY{0}, _width{0}, _height{0}, _bearingX{0}, _bearingY{0} {}

void Glyph::addLineSegment(vft::Edge edge) {
    this->_lineSegments.push_back(edge);
}

void Glyph::addCurveSegment(vft::Curve curve) {
    this->_curveSegments.push_back(curve);
}

std::array<glm::vec2, 4> Glyph::getBoundingBox() const {
    long xMin = this->_bearingX;
    long xMax = this->_bearingX + this->_width;
    long yMin = this->_bearingY - this->_height;
    long yMax = this->_bearingY;

    return {glm::vec2(xMin, yMin), glm::vec2(xMin, yMax), glm::vec2(xMax, yMax), glm::vec2(xMax, yMin)};
}

const std::vector<vft::Edge> &Glyph::getLineSegmentsIndices() const {
    return this->_lineSegments;
}

const std::vector<vft::Curve> &Glyph::getCurveSegmentsIndices() const {
    return this->_curveSegments;
}

uint32_t Glyph::getLineSegmentsIndexCount() const {
    return this->_lineSegments.size() * 2;
}

uint32_t Glyph::getCurveSegmentsIndexCount() const {
    return this->_curveSegments.size() * 3;
}

void Glyph::setWidth(long width) {
    this->_width = width;
}

void Glyph::setHeight(long height) {
    this->_height = height;
}

void Glyph::setBearingX(long bearingX) {
    this->_bearingX = bearingX;
}

void Glyph::setBearingY(long bearingY) {
    this->_bearingY = bearingY;
}

/**
 * @brief Set by how much should the X coordinate update after rendering glyph
 *
 * @param advanceX X value of advance vector
 */
void Glyph::setAdvanceX(long advanceX) {
    this->_advanceX = advanceX;
}

/**
 * @brief Set by how much should the Y coordinate update after rendering glyph
 *
 * @param advanceX Y value of advance vector
 */
void Glyph::setAdvanceY(long advanceY) {
    this->_advanceY = advanceY;
}

/**
 * @brief Get by how much should the X coordinate update after rendering glyph
 *
 * @return X value of advance vector
 */
long Glyph::getAdvanceX() const {
    return this->_advanceX;
}

/**
 * @brief Get by how much should the Y coordinate update after rendering glyph
 *
 * @return Y value of advance vector
 */
long Glyph::getAdvanceY() const {
    return this->_advanceY;
}

}  // namespace vft
