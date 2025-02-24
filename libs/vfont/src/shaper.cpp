/**
 * @file shaper.cpp
 * @author Christian Saloň
 */

#include "shaper.h"

namespace vft {

std::vector<std::vector<ShapedCharacter>> Shaper::shape(std::vector<uint32_t> codePoints,
                                                        std::shared_ptr<Font> font,
                                                        unsigned int fontSize) {
    Shaper::_preprocessInput(codePoints);

    // Get indices of line breaks in input text
    std::vector<unsigned int> newLines;
    for (unsigned int i = 0; i < codePoints.size(); i++) {
        if (codePoints[i] == vft::U_LF) {
            newLines.push_back(i);
        }
    }
    newLines.push_back(codePoints.size());

    // Create font objects
    hb_face_t *hbFace = hb_ft_face_create(font->getFace(), 0);
    hb_font_t *hbFont = hb_font_create(hbFace);

    // Construct shaping output for all glyphs
    std::vector<std::vector<ShapedCharacter>> output;
    output.resize(newLines.size());

    unsigned int lineStart = 0;
    unsigned int lineIndex = 0;
    for (unsigned int lineEnd : newLines) {
        // Skip empty line
        if (lineStart != lineEnd) {
            // Create harfbuzz buffer
            hb_buffer_t *buffer = hb_buffer_create();

            // Add input to buffer
            hb_buffer_add_utf32(buffer, codePoints.data(), codePoints.size(), lineStart, lineEnd - lineStart);

            // Set text properties
            hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
            hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
            hb_buffer_set_language(buffer, hb_language_from_string("en", -1));

            // Shape text
            hb_shape(hbFont, buffer, nullptr, 0);

            // Get shaping output
            unsigned int glyphCount;
            hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
            hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buffer, &glyphCount);

            // Scale used for converting from font units to pixels
            glm::vec2 scale = font->getScalingVector(fontSize);

            for (unsigned int i = 0; i < glyphCount; i++) {
                ShapedCharacter shapedCharacter{glyphInfos[i].codepoint,
                                                glyphInfos[i].cluster,
                                                glyphPositions[i].x_advance * scale.x,
                                                glyphPositions[i].y_advance * scale.y,
                                                glyphPositions[i].x_offset * scale.x,
                                                glyphPositions[i].y_offset * scale.y};
                output.at(lineIndex).push_back(shapedCharacter);
            }

            // Destroy shaped buffer
            hb_buffer_destroy(buffer);
        }

        lineStart = lineEnd + 1;
        lineIndex++;
    }

    // Destroy font objects
    hb_face_destroy(hbFace);
    hb_font_destroy(hbFont);

    return output;
}

void Shaper::_preprocessInput(std::vector<uint32_t> &codePoints) {
    for (unsigned int i = 0; i < codePoints.size(); i++) {
        if (codePoints[i] == vft::U_TAB) {
            // Replace TAB with 4 spaces
            codePoints.insert(codePoints.begin() + i, 4, vft::U_SPACE);
            // Erase the original tab
            codePoints.erase(codePoints.begin() + i + 4);
        } else if (i + 1 < codePoints.size() && codePoints[i] == vft::U_CR && codePoints[i + 1] == vft::U_LF) {
            // Erase CR, leave only LF
            codePoints.erase(codePoints.begin() + i);
        } else if (codePoints[i] == vft::U_CR) {
            // Replace CR with LF
            codePoints[i] = vft::U_LF;
        }
    }
}

}  // namespace vft
