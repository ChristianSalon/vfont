/**
 * @file shaper.cpp
 * @author Christian Saloň
 */

#include "shaper.h"

namespace vft {

/**
 * @brief Shape utf-32 encoded text using harfbuzz
 *
 * This function processes input text encoded in utf-32, which is provided in logical order.
 * Using the specified properties, it applies text shaping and reorders the glyphs into visual order.
 * The output is divided into lines, ensuring proper text rendering.
 *
 * @param text Utf-32 encoded text
 * @param font Font of text
 * @param fontSize Font size of text
 * @param direction Direction in which to render text (e.g. left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 *
 * @return Shaped characters divided into lines (by CR, LF or CRLF)
 */
std::vector<std::vector<ShapedCharacter>> Shaper::shape(std::u32string text,
                                                        std::shared_ptr<Font> font,
                                                        unsigned int fontSize,
                                                        hb_direction_t direction,
                                                        hb_script_t script,
                                                        hb_language_t language) {
    Shaper::_preprocessInput(text);

    // Get indices of line breaks in input text
    std::vector<unsigned int> newLines;
    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == U_LF) {
            newLines.push_back(i);
        }
    }
    newLines.push_back(text.size());

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
            hb_buffer_add_utf32(buffer, reinterpret_cast<const uint32_t *>(text.data()), text.size(), lineStart,
                                lineEnd - lineStart);

            // Set text properties
            hb_buffer_set_direction(buffer, direction);
            hb_buffer_set_script(buffer, script);
            hb_buffer_set_language(buffer, language);

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

/**
 * @brief Preprocess input utf-32 text
 *
 * @param text Utf-32 encoded input text
 */
void Shaper::_preprocessInput(std::u32string &text) {
    for (unsigned int i = 0; i < text.size(); i++) {
        if (text[i] == U_TAB) {
            // Replace TAB with 4 spaces
            text.insert(text.begin() + i, 4, U_SPACE);
            // Erase the original tab
            text.erase(text.begin() + i + 4);
        } else if (i + 1 < text.size() && text[i] == U_CR && text[i + 1] == U_LF) {
            // Erase CR, leave only LF
            text.erase(text.begin() + i);
        } else if (text[i] == U_CR) {
            // Replace CR with LF
            text[i] = U_LF;
        }
    }
}

}  // namespace vft
