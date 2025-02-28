/**
 * @file text_align_strategy.cpp
 * @author Christian Saloň
 */

#include "text_align_strategy.h"

namespace vft {

glm::vec2 LeftTextAlign::getLineOffset(double lineSize, double maxLineSize) {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("LeftTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("LeftTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2(0.f, 0.f);
}

glm::vec2 CenterTextAlign::getLineOffset(double lineSize, double maxLineSize) {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("CenterTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("CenterTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2((maxLineSize - lineSize) / 2.f, 0.f);
}

glm::vec2 RightTextAlign::getLineOffset(double lineSize, double maxLineSize) {
    if (lineSize < 0 || maxLineSize < 0) {
        throw std::invalid_argument("RightTextAlign::getLineOffset(): Line sizes must be positive");
    }

    if (lineSize > maxLineSize) {
        throw std::invalid_argument("RightTextAlign::getLineOffset(): Max line size must ne bigger than line size");
    }

    return glm::vec2(maxLineSize - lineSize, 0.f);
}

}  // namespace vft
