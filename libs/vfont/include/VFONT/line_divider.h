/**
 * @file line_divider.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>

#include "character.h"

namespace vft {

typedef struct {
    double width;
    double height;
    double x;
    double y;
} LineData;

class LineDivider {
protected:
    double _maxLineSize;

    std::map<unsigned int, LineData> _lines;
    std::vector<Character> _characters;

public:
    const std::map<unsigned int, LineData> &divide(unsigned int startCharacterIndex = 0);

    void setCharacters(const std::vector<Character> &characters);
    void setMaxLineSize(double maxLineSize);

    std::pair<unsigned int, LineData> getLineOfCharacter(unsigned int characterIndex);

    inline const std::map<unsigned int, LineData> &getLines();
};

inline const std::map<unsigned int, LineData> &LineDivider::getLines() {
    return this->_lines;
};

}  // namespace vft
