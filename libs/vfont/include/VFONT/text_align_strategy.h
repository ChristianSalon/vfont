/**
 * @file text_align_strategy.h
 * @author Christian Saloň
 */

#pragma once

#include <stdexcept>

#include <glm/vec2.hpp>

namespace vft {

class TextAlignStrategy {
public:
    virtual glm::vec2 getLineOffset(double lineSize, double maxLineSize) = 0;
};

class LeftTextAlign : public TextAlignStrategy {
public:
    glm::vec2 getLineOffset(double lineSize, double maxLineSize) override;
};

class CenterTextAlign : public TextAlignStrategy {
public:
    glm::vec2 getLineOffset(double lineSize, double maxLineSize) override;
};

class RightTextAlign : public TextAlignStrategy {
public:
    glm::vec2 getLineOffset(double lineSize, double maxLineSize) override;
};

}  // namespace vft
