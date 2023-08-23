/**
 * @file kb_input.h
 * @author Christian Saloň
 */

#pragma once

#include <iostream>
#include <vector>

class KbInput {

public:

    enum class Utf8Type {
        ONE_CODE_UNIT,
        TWO_CODE_UNITS,
        THREE_CODE_UNITS,
        FOUR_CODE_UNITS
    };

    enum class Utf16Type {
        ONE_CODE_UNIT,
        TWO_CODE_UNITS
    };

    typedef struct utf8 {
        union {
            uint8_t bytes_1;
            uint8_t bytes_2[2];
            uint8_t bytes_3[3];
            uint8_t bytes_4[4];
        };
        Utf8Type type;
    } utf8_t;

    typedef struct utf16 {
        union {
            uint16_t bytes_2;
            uint16_t bytes_4[2];
        };
        Utf16Type type;
    } utf16_t;

#if defined(USE_WIN32)

    static utf16_t currentCharacter;

#endif

private:

    static std::vector<uint32_t> _input;

public:

    static void registerCharacter(utf8_t character);
    static void registerCharacter(utf16_t character);

    static utf16_t utf8ToUtf16(utf8_t character);
    static uint32_t utf8ToCodePoint(utf8_t character);
    static utf8_t codePointToUtf8(uint32_t codePoint);

    static utf8_t utf16ToUtf8(utf16_t character);
    static uint32_t utf16ToCodePoint(utf16_t character);
    static utf16_t codePointToUtf16(uint32_t codePoint);
};
