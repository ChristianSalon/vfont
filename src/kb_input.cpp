/**
 * @file kb_input.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <vector>
#include <stdexcept>

#include "kb_input.h"

std::vector<uint32_t> KbInput::_input; /**< Buffer of unicode code points for user input */

#if defined(USE_WIN32)

KbInput::utf16_t KbInput::currentCharacter = {};

#endif

/**
 * @brief Adds utf-8 encoded character to input buffer
 *
 * @param character Charcater to process
 */
void KbInput::registerCharacter(KbInput::utf8_t character) {
    uint32_t codePoint = KbInput::utf8ToCodePoint(character);
    _input.push_back(codePoint);

    std::cout << "UTF-8: Processing character U+" << std::hex << codePoint << ", 0x";
    switch (character.type) {
        case KbInput::Utf8Type::ONE_CODE_UNIT: {
            std::cout << (uint32_t)character.bytes_1 << std::endl;
            break;
        }
        case KbInput::Utf8Type::TWO_CODE_UNITS: {
            std::cout << (uint32_t)character.bytes_2[0] << (uint32_t)character.bytes_2[1] << std::endl;
            break;
        }
        case KbInput::Utf8Type::THREE_CODE_UNITS: {
            std::cout << (uint32_t)character.bytes_3[0] << (uint32_t)character.bytes_3[1] << (uint32_t)character.bytes_3[2] << std::endl;
            break;
        }
        case KbInput::Utf8Type::FOUR_CODE_UNITS: {
            std::cout << (uint32_t)character.bytes_4[0] << (uint32_t)character.bytes_4[1] << (uint32_t)character.bytes_4[2] << (uint32_t)character.bytes_4[3] << std::endl;
            break;
        }
    }
}

/**
 * @brief Adds utf-16 encoded character to input buffer
 *
 * @param character Charcater to process
 */
void KbInput::registerCharacter(KbInput::utf16_t character) {
    uint32_t codePoint = KbInput::utf16ToCodePoint(character);
    _input.push_back(codePoint);

    std::cout << "UTF-16: Processing character U+" << std::hex << codePoint << ", 0x";
    if (character.type == KbInput::Utf16Type::ONE_CODE_UNIT) {
        std::cout << character.bytes_2 << std::endl;
    }
    else {
        std::cout << character.bytes_4[0] << character.bytes_4[1] << std::endl;
    }
}

/**
 * @brief Encodes utf-8 character to utf-16 character
 *
 * @param character Utf-8 encoded character
 *
 * @return Utf-16 encoded character
 */
KbInput::utf16_t KbInput::utf8ToUtf16(KbInput::utf8_t character) {
    uint32_t codePoint = KbInput::utf8ToCodePoint(character);
    KbInput::utf16_t utf16Character = KbInput::codePointToUtf16(codePoint);

    return utf16Character;
}

/**
 * @brief Converts utf-8 encoded character to unicode code point
 *
 * @param character Utf-8 encoded character
 *
 * @return Unicode code point of character
 */
uint32_t KbInput::utf8ToCodePoint(KbInput::utf8_t character) {
    switch (character.type) {
        case KbInput::Utf8Type::ONE_CODE_UNIT: {
            return character.bytes_1;                       // byte1 = 0b0xxxxxxx
        }
        case KbInput::Utf8Type::TWO_CODE_UNITS: {
            return
                ((character.bytes_2[0] & 0x1F) << 6) +      // byte1 = 0b110xxxxx
                (character.bytes_2[1] & 0x3F);              // byte2 = 0b10xxxxxx
        }
        case KbInput::Utf8Type::THREE_CODE_UNITS: {
            return
                ((character.bytes_3[0] & 0x0F) << 12) +     // byte1 = 0b1110xxxx
                ((character.bytes_3[1] & 0x3F) << 6) +      // byte2 = 0b10xxxxxx
                (character.bytes_3[2] & 0x3F);              // byte3 = 0b10xxxxxx
        }
        case KbInput::Utf8Type::FOUR_CODE_UNITS: {
            return
                ((character.bytes_4[0] & 0x07) << 18) +     // byte1 = 0b11110xxx
                ((character.bytes_4[1] & 0x3F) << 12) +     // byte2 = 0b10xxxxxx
                ((character.bytes_4[2] & 0x3F) << 6) +      // byte3 = 0b10xxxxxx
                (character.bytes_4[3] & 0x3F);              // byte4 = 0b10xxxxxx
        }
    }

    throw std::runtime_error("Invalid UTF-8 character.");
}

/**
 * @brief Converts unicode code point to utf-8 encoded character
 * 
 * @param codePoint Unicode code point
 * 
 * @return Utf-8 encoded character
 */
KbInput::utf8_t KbInput::codePointToUtf8(uint32_t codePoint) {
    KbInput::utf8_t utf8Character;

    if (codePoint >= 0x0000 && codePoint <= 0x007F) {
        utf8Character.type = KbInput::Utf8Type::ONE_CODE_UNIT;
        utf8Character.bytes_1 = codePoint;                              // byte1 = 0b0xxxxxxx
    }
    else if (codePoint >= 0x0080 && codePoint <= 0x07FF) {
        utf8Character.type = KbInput::Utf8Type::TWO_CODE_UNITS;
        utf8Character.bytes_2[0] = (codePoint >> 6) + 0xC0;             // byte1 = 0b110xxxxx
        utf8Character.bytes_2[1] = (codePoint & 0x3F) + 0x80;           // byte2 = 0b10xxxxxx
    }
    else if (codePoint >= 0x0800 && codePoint <= 0xFFFF) {
        utf8Character.type = KbInput::Utf8Type::THREE_CODE_UNITS;
        utf8Character.bytes_3[0] = (codePoint >> 12) + 0xE0;            // byte1 = 0b1110xxxx
        utf8Character.bytes_3[1] = ((codePoint >> 6) & 0x3F) + 0x80;    // byte2 = 0b10xxxxxx
        utf8Character.bytes_3[2] = (codePoint & 0x3F) + 0x80;           // byte3 = 0b10xxxxxx
    }
    else if (codePoint >= 0x10000 && codePoint <= 0x10FFFF) {
        utf8Character.type = KbInput::Utf8Type::FOUR_CODE_UNITS;
        utf8Character.bytes_4[0] = (codePoint >> 18) + 0xF0;            // byte1 = 0b11110xxx
        utf8Character.bytes_4[1] = ((codePoint >> 12) & 0x3F) + 0x80;   // byte2 = 0b10xxxxxx
        utf8Character.bytes_4[2] = ((codePoint >> 6) & 0x3F) + 0x80;    // byte3 = 0b10xxxxxx
        utf8Character.bytes_4[3] = (codePoint & 0x3F) + 0x80;           // byte4 = 0b10xxxxxx
    }
    else {
        throw std::runtime_error("Invalid unicode code point.");
    }

    return utf8Character;
}

/**
 * @brief Encodes utf-16 character to utf-16 character
 *
 * @param character Utf-16 encoded character
 *
 * @return Utf-8 encoded character
 */
KbInput::utf8_t KbInput::utf16ToUtf8(KbInput::utf16_t character) {
    uint32_t codePoint = KbInput::utf16ToCodePoint(character);
    KbInput::utf8_t utf8Character = KbInput::codePointToUtf8(codePoint);

    return utf8Character;
}

/**
 * @brief Converts utf-16 encoded character to unicode code point
 *
 * @param character Utf-16 encoded character
 *
 * @return Unicode code point of character
 */
uint32_t KbInput::utf16ToCodePoint(KbInput::utf16_t character) {
    if (character.type == KbInput::Utf16Type::ONE_CODE_UNIT) {
        return (uint32_t)character.bytes_2;
    }
    else {
        uint16_t codeUnit1 = character.bytes_4[0];  // word1 = 0b110110yyyyyyyyyy
        uint16_t codeUnit2 = character.bytes_4[1];  // word2 = 0b110111xxxxxxxxxx
        codeUnit1 -= 0xD800;
        codeUnit2 -= 0xDC00;

        // U = 0x10000 + 0byyyyyyyyyyxxxxxxxxxx
        uint32_t codePoint = (codeUnit1 << 10) + codeUnit2 + 0x10000;

        return codePoint;
    }
}

/**
 * @brief Converts unicode code point to utf-16 encoded character
 *
 * @param codePoint Unicode code point
 *
 * @return Utf-16 encoded character
 */
KbInput::utf16_t KbInput::codePointToUtf16(uint32_t codePoint) {
    KbInput::utf16_t character;

    if (codePoint >= 0x0000 && codePoint <= 0xD7FF || codePoint >= 0xE000 && codePoint <= 0xFFFF) {
        // UTF-16 encoded code point is 1 code unit long
        character.type = KbInput::Utf16Type::ONE_CODE_UNIT;
        character.bytes_2 = codePoint;
    }
    else if (codePoint >= 0x010000 && codePoint <= 0x10FFFF) {
        // UTF-16 encoded code point is 2 code units long
        // U' = 0byyyyyyyyyyxxxxxxxxxx
        codePoint -= 0x10000;

        character.type = KbInput::Utf16Type::TWO_CODE_UNITS;
        character.bytes_4[0] = (codePoint & 0xFFC00) + 0xD800;    // word1 = 0b110110yyyyyyyyyy
        character.bytes_4[1] = (codePoint & 0x03FF) + 0xDC00;     // word2 = 0b110111xxxxxxxxxx
    }
    else {
        throw std::runtime_error("Invalid unicode code point.");
    }

    return character;
}
