/**
 * @file glyph.cpp
 * @author Christian Saloň
 */

#include "glyph.h"

namespace vft {

/**
 * @brief Store line segment of glyph
 *
 * @param edge Line segment
 */
void Glyph::addLineSegment(Edge edge) {
    this->_lineSegments.push_back(edge);
}

/**
 * @brief Store curve segment of glyph
 *
 * @param curve Curve segment
 */
void Glyph::addCurveSegment(Curve curve) {
    this->_curveSegments.push_back(curve);
}

/**
 * @brief Get bounding box of glyph
 *
 * @return Bounding box
 */
std::array<glm::vec2, 4> Glyph::getBoundingBox() const {
    long xMin = this->_bearingX;
    long xMax = this->_bearingX + this->_width;
    long yMin = this->_bearingY - this->_height;
    long yMax = this->_bearingY;

    return {glm::vec2(xMin, yMin), glm::vec2(xMin, yMax), glm::vec2(xMax, yMax), glm::vec2(xMax, yMin)};
}

/**
 * @brief Get all line segments of glyph
 *
 * @return Line segments
 */
const std::vector<Edge> &Glyph::getLineSegmentsIndices() const {
    return this->_lineSegments;
}

/**
 * @brief Get all curve segments of glyph
 *
 * @return Curve segments
 */
const std::vector<Curve> &Glyph::getCurveSegmentsIndices() const {
    return this->_curveSegments;
}

/**
 * @brief Get count of indices used by all line segments. Each line segment consists of two indices
 *
 * @return Index count
 */
uint32_t Glyph::getLineSegmentsIndexCount() const {
    return this->_lineSegments.size() * 2;
}

/**
 * @brief Get count of indices used by all curve segments. Each curve segment consists of three indices
 *
 * @return Index count
 */
uint32_t Glyph::getCurveSegmentsIndexCount() const {
    return this->_curveSegments.size() * 3;
}

/**
 * @brief Setter for width of glyph
 *
 * @param width Width of glyph
 */
void Glyph::setWidth(long width) {
    this->_width = width;
}

/**
 * @brief Setter for height of glyph
 *
 * @param height Height of glyph
 */
void Glyph::setHeight(long height) {
    this->_height = height;
}

/**
 * @brief Setter for x bearing
 *
 * @param bearingX X bearing
 */
void Glyph::setBearingX(long bearingX) {
    this->_bearingX = bearingX;
}

/**
 * @brief Setter for y bearing
 *
 * @param bearingY Y bearing
 */
void Glyph::setBearingY(long bearingY) {
    this->_bearingY = bearingY;
}

/**
 * @brief Set by how much should the x coordinate update after rendering glyph
 *
 * @param advanceX X value of advance vector
 */
void Glyph::setAdvanceX(long advanceX) {
    this->_advanceX = advanceX;
}

/**
 * @brief Set by how much should the y coordinate update after rendering glyph
 *
 * @param advanceY Y value of advance vector
 */
void Glyph::setAdvanceY(long advanceY) {
    this->_advanceY = advanceY;
}

/**
 * @brief Getter for width of glyph
 *
 * @return Width
 */
long Glyph::getWidth() const {
    return this->_width;
}

/**
 * @brief Getter for height of glyph
 *
 * @return Height
 */
long Glyph::getHeight() const {
    return this->_height;
}

/**
 * @brief Getter for x bearing
 *
 * @return X bearing
 */
long Glyph::getBearingX() const {
    return this->_bearingX;
}

/**
 * @brief Getter for y bearing
 *
 * @return Y bearing
 */
long Glyph::getBearingY() const {
    return this->_bearingY;
}

/**
 * @brief Get by how much should the pen's x coordinate update after rendering glyph
 *
 * @return X value of advance vector
 */
long Glyph::getAdvanceX() const {
    return this->_advanceX;
}

/**
 * @brief Get by how much should the pen's y coordinate update after rendering glyph
 *
 * @return Y value of advance vector
 */
long Glyph::getAdvanceY() const {
    return this->_advanceY;
}

}  // namespace vft
