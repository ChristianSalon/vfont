/**
 * @file text_block_builder.cpp
 * @author Christian Saloň
 */

#include "text_block_builder.h"

namespace vft {

/**
 * @brief Set font of text block
 *
 * @param font Font
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setFont(std::shared_ptr<Font> font) {
    this->_block.setFont(font);

    return *this;
}

/**
 * @brief Set font size of text block
 *
 * @param fontSize Font size
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setFontSize(unsigned int fontSize) {
    this->_block.setFontSize(fontSize);

    return *this;
}

/**
 * @brief Set width of text block
 *
 * @param width Width
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setWidth(int width) {
    this->_block.setWidth(width);

    return *this;
}

/**
 * @brief Set text color in text block
 *
 * @param color Text color
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setColor(glm::vec4 color) {
    this->_block.setColor(color);

    return *this;
}

/**
 * @brief Set position of text block
 *
 * @param position Position
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setPosition(glm::vec3 position) {
    this->_block.setPosition(position);

    return *this;
}

/**
 * @brief Set text align of text block
 *
 * @param textAlign Text align
 *
 * @return Reference to text block that is being built
 */
TextBlockBuilder &TextBlockBuilder::setTextAlign(std::unique_ptr<TextAlignStrategy> textAlign) {
    this->_block.setTextAlign(std::move(textAlign));

    return *this;
}

/**
 * @brief Get the built text block
 *
 * @return Built text block
 */
std::shared_ptr<TextBlock> TextBlockBuilder::build() {
    std::shared_ptr<TextBlock> textBlock = std::make_shared<TextBlock>(std::move(this->_block));
    this->_block = TextBlock{};

    return textBlock;
}

}  // namespace vft
