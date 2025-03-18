/**
 * @file unicode.cpp
 * @author Christian Saloň
 */

#include "unicode.h"

namespace vft {

/**
 * @brief Convert a utf-8 encoded string to utf-16
 *
 * @param input Utf-8 encoded string
 *
 * @return Utf-16 encoded string
 */
std::u16string Unicode::utf8ToUtf16(std::u8string input) {
    return utf32ToUtf16(utf8ToUtf32(input));
}

/**
 * @brief Convert a utf-8 encoded string to utf-32
 *
 * @param input Utf-8 encoded string
 *
 * @return Utf-32 encoded string
 */
std::u32string Unicode::utf8ToUtf32(std::u8string input) {
    std::u32string output;

    for (auto it = input.begin(); it != input.end(); it = std::next(it)) {
        char32_t utf32Character = 0;
        unsigned int size = getSizeOfUtf8Character(*it);

        if (size == 1) {
            utf32Character = *it;  // byte1 = 0b0xxxxxxx
        } else if (size == 2) {
            utf32Character = ((*it & 0x1f) << 6)         // byte1 = 0b110xxxxx
                             + (*std::next(it) & 0x3f);  // byte2 = 0b10xxxxxx
            it = std::next(it);
        } else if (size == 3) {
            utf32Character = ((*it & 0x0f) << 12) +            // byte1 = 0b1110xxxx
                             ((*std::next(it) & 0x3f) << 6) +  // byte2 = 0b10xxxxxx
                             (*std::next(it, 2) & 0x3f);       // byte3 = 0b10xxxxxx
            it = std::next(it, 2);
        } else if (size == 4) {
            utf32Character = ((*it & 0x07) << 18) +               // byte1 = 0b11110xxx
                             ((*std::next(it) & 0x3f) << 12) +    // byte2 = 0b10xxxxxx
                             ((*std::next(it, 2) & 0x3f) << 6) +  // byte3 = 0b10xxxxxx
                             (*std::next(it, 3) & 0x3f);          // byte4 = 0b10xxxxxx
            it = std::next(it, 3);
        } else {
            throw std::runtime_error("Unicode::utf8ToUtf32(): Invalid size of utf-8 character");
        }

        output.push_back(utf32Character);
    }

    return output;
}

/**
 * @brief Convert a utf-16 encoded string to utf-8
 *
 * @param input Utf-16 encoded string
 *
 * @return Utf-8 encoded string
 */
std::u8string Unicode::utf16ToUtf8(std::u16string input) {
    return utf32ToUtf8(utf16ToUtf32(input));
}

/**
 * @brief Convert a utf-16 encoded string to utf-32
 *
 * @param input Utf-16 encoded string
 *
 * @return Utf-32 encoded string
 */
std::u32string Unicode::utf16ToUtf32(std::u16string input) {
    std::u32string output;

    for (auto it = input.begin(); it != input.end(); it = std::next(it)) {
        char32_t utf32Character = 0;
        unsigned int size = getSizeOfUtf16Character(*it);

        if (size == 2) {
            utf32Character = *it;
        } else if (size == 4) {
            char16_t codeUnit1 = *it;             // word1 = 0b110110yyyyyyyyyy
            char16_t codeUnit2 = *std::next(it);  // word2 = 0b110111xxxxxxxxxx
            codeUnit1 -= 0xD800;
            codeUnit2 -= 0xDC00;

            // U = 0x10000 + 0byyyyyyyyyyxxxxxxxxxx
            utf32Character = (codeUnit1 << 10) + codeUnit2 + 0x10000;

            it = std::next(it);
        } else {
            throw std::runtime_error("Unicode::utf16ToUtf32(): Invalid size of utf-16 character");
        }

        output.push_back(utf32Character);
    }

    return output;
}

/**
 * @brief Convert a utf-32 encoded string to utf-8
 *
 * @param input Utf-32 encoded string
 *
 * @return Utf-8 encoded string
 */
std::u8string Unicode::utf32ToUtf8(std::u32string input) {
    std::u8string output;

    for (auto it = input.begin(); it != input.end(); it = std::next(it)) {
        if (*it >= 0x0000 && *it <= 0x007f) {
            output.push_back(*it);  // byte1 = 0b0xxxxxxx
        } else if (*it >= 0x0080 && *it <= 0x07ff) {
            output.push_back((*it >> 6) + 0xCc0);   // byte1 = 0b110xxxxx
            output.push_back((*it & 0x3f) + 0x80);  // byte2 = 0b10xxxxxx
        } else if (*it >= 0x0800 && *it <= 0xffff) {
            output.push_back((*it >> 12) + 0xe0);          // byte1 = 0b1110xxxx
            output.push_back(((*it >> 6) & 0x3f) + 0x80);  // byte2 = 0b10xxxxxx
            output.push_back((*it & 0x3f) + 0x80);         // byte3 = 0b10xxxxxx
        } else if (*it >= 0x10000 && *it <= 0x10ffff) {
            output.push_back((*it >> 18) + 0xf0);           // byte1 = 0b11110xxx
            output.push_back(((*it >> 12) & 0x3f) + 0x80);  // byte2 = 0b10xxxxxx
            output.push_back(((*it >> 6) & 0x3f) + 0x80);   // byte3 = 0b10xxxxxx
            output.push_back((*it & 0x3f) + 0x80);          // byte4 = 0b10xxxxxx
        } else {
            throw std::runtime_error("Unicode::utf32ToUtf8(): Invalid utf-32 character");
        }
    }

    return output;
}

/**
 * @brief Convert a utf-32 encoded string to utf-16
 *
 * @param input Utf-32 encoded string
 *
 * @return Utf-16 encoded string
 */
std::u16string Unicode::utf32ToUtf16(std::u32string input) {
    std::u16string output;

    for (auto it = input.begin(); it != input.end(); it = std::next(it)) {
        if (*it >= 0x0000 && *it <= 0xd7ff || *it >= 0xe000 && *it <= 0xffff) {
            // UTF-16 encoded code point is 2 bytes long
            output.push_back(*it);
        } else if (*it >= 0x010000 && *it <= 0x10ffff) {
            // UTF-16 encoded code point is 4 bytes long

            // U' = 0byyyyyyyyyyxxxxxxxxxx
            char32_t codePoint = *it - 0x10000;
            output.push_back((codePoint & 0xffc00) + 0xd800);  // word1 = 0b110110yyyyyyyyyy
            output.push_back((codePoint & 0x03ff) + 0xdc00);   // word2 = 0b110111xxxxxxxxxx
        } else {
            throw std::runtime_error("Unicode::utf32ToUtf16(): Invalid utf-32 character");
        }
    }

    return output;
}

/**
 * @brief Get the size in bytes of a utf-8 encoded character
 *
 * @param firstByte First byte of utf-8 encoded character
 *
 * @return Number of bytes the utf-8 character is encoded into
 */
unsigned int Unicode::getSizeOfUtf8Character(char8_t firstByte) {
    if ((firstByte & 0x80) == 0x00) {
        return 1;  // byte1 = 0b0xxxxxxx
    } else if ((firstByte & 0xe0) == 0xc0) {
        return 2;  // byte1 = 0b110xxxxx
    } else if ((firstByte & 0xf0) == 0xe0) {
        return 3;  // byte1 = 0b1110xxxx
    } else if ((firstByte & 0xf8) == 0xf0) {
        return 4;  // byte1 = 0b11110xxx
    }

    throw std::invalid_argument("Unicode::getSizeOfUtf8Character(): Invalid first byte of utf-8 character");
}

/**
 * @brief Get the size in bytes of a utf-16 encoded character
 *
 * @param firstByte First byte of utf-16 encoded character
 *
 * @return Number of bytes the utf-16 character is encoded into
 */
unsigned int Unicode::getSizeOfUtf16Character(char16_t firstByte) {
    if (firstByte >= 0xd800 && firstByte <= 0xdbff) {
        return 4;
    }

    return 2;
}

}  // namespace vft
