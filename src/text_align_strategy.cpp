/**
 * @file text_align_strategy.cpp
 * @author Christian Saloň
 */

#include "text_align_strategy.h"

namespace vft {

/**
 * @brief Get line offset from left edge based on max line size and actual line size
 *
 * @param lineSize Size of line
 * @param maxLineSize Max size of a line
 *
 * @return Offset vector
 */
glm::vec2 LeftTextAlign::getLineOffset(double lineSize, double maxLineSize) const {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("LeftTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("LeftTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2(0.f, 0.f);
}
/**
 * @brief Get line offset from left edge based on max line size and actual line size
 *
 * @param lineSize Size of line
 * @param maxLineSize Max size of a line
 *
 * @return Offset vector
 */
glm::vec2 CenterTextAlign::getLineOffset(double lineSize, double maxLineSize) const {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("CenterTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("CenterTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2((maxLineSize - lineSize) / 2.f, 0.f);
}
/**
 * @brief Get line offset from left edge based on max line size and actual line size
 *
 * @param lineSize Size of line
 * @param maxLineSize Max size of a line
 *
 * @return Offset vector
 */
glm::vec2 RightTextAlign::getLineOffset(double lineSize, double maxLineSize) const {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("RightTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("RightTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2(maxLineSize - lineSize, 0.f);
}

}  // namespace vft
