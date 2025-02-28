/**
 * @file text_block_builder.cpp
 * @author Christian Saloň
 */

#include "text_block_builder.h"

namespace vft {

TextBlockBuilder::TextBlockBuilder() : _block{} {}

TextBlockBuilder &TextBlockBuilder::setFont(std::shared_ptr<Font> font) {
    this->_block.setFont(font);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setFontSize(unsigned int fontSize) {
    this->_block.setFontSize(fontSize);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setWidth(int width) {
    this->_block.setWidth(width);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setKerning(bool kerning) {
    this->_block.setKerning(kerning);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setColor(glm::vec4 color) {
    this->_block.setColor(color);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setPosition(glm::vec3 position) {
    this->_block.setPosition(position);

    return *this;
}

TextBlockBuilder &TextBlockBuilder::setTextAlign(std::unique_ptr<TextAlignStrategy> textAlign) {
    this->_block.setTextAlign(std::move(textAlign));

    return *this;
}

std::shared_ptr<TextBlock> TextBlockBuilder::build() {
    std::shared_ptr<TextBlock> textBlock = std::make_shared<TextBlock>(std::move(this->_block));
    this->_block = TextBlock{};

    return textBlock;
}

}  // namespace vft
