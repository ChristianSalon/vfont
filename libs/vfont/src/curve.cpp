/**
 * @file curve.cpp
 * @author Christian Saloň
 */

#include "curve.h"

namespace vft {

/**
 * @brief Construct a quadratic bezier curve
 *
 * @param start Start point of bezier curve
 * @param control Control point of bezier curve
 * @param end End point of bezier curve
 */
Curve::Curve(uint32_t start, uint32_t control, uint32_t end) : start{start}, control{control}, end{end} {}

/**
 * @brief Curve constructor, set indices to zero
 */
Curve::Curve() : start{0}, control{0}, end{0} {}

/**
 * @brief Curve equality operator
 *
 * @param curve Curve to compare
 *
 * @return True if start, control and end indices match
 */
bool Curve::operator==(const Curve &curve) const {
    return this->start == curve.start && this->control == curve.control && this->end == curve.end;
}

}  // namespace vft
