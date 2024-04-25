/**
 * @file kb_input.h
 * @author Christian Saloň
 */

#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

/**
 * @class KbInput
 * 
 * @brief Provides data types, attributes and methods for proccessing keyboar input
*/
class KbInput {

public:

    /**
     * @brief Defines the size of an UTF-8 encoded character
     */
    enum class Utf8Type {
        ONE_CODE_UNIT,      /**< 1 byte */
        TWO_CODE_UNITS,     /**< 2 bytes */
        THREE_CODE_UNITS,   /**< 3 bytes */
        FOUR_CODE_UNITS     /**< 4 bytes */
    };

    /**
     * @brief Defines the size of an UTF-16 encoded character
     */
    enum class Utf16Type {
        ONE_CODE_UNIT,  /**< 2 bytes */
        TWO_CODE_UNITS  /**< 4 bytes */
    };

    /**
     * @brief UTF-8 represented character
     */
    typedef struct {
        union {
            uint8_t bytes_1;
            uint8_t bytes_2[2];
            uint8_t bytes_3[3];
            uint8_t bytes_4[4];
        };                      /**< Bytes of UTF-8 encoded character */
        Utf8Type type;          /**< Size of character */
    } utf8_t;

    /**
     * @brief UTF-16 represented character
     */
    typedef struct {
        union {
            uint16_t bytes_2;
            uint16_t bytes_4[2];
        };                      /**< Bytes of UTF-16 encoded character */
        Utf16Type type;         /**< Size of character */
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

private:

    static void _drawCharacter(uint32_t codePoint);

};
