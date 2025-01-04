/**
 * @file renderer_decorator.cpp
 * @author Christian Saloň
 */

#include "renderer_decorator.h"

namespace vft {

RendererDecorator::RendererDecorator(Renderer *renderer) : _renderer{renderer} {}

RendererDecorator::~RendererDecorator() {
    delete this->_renderer;
    std::cout << "~RendererDecorator\n";
}

void RendererDecorator::init(TessellationStrategy tessellationStrategy, VulkanContext vulkanContext) {
    this->_renderer->init(tessellationStrategy, vulkanContext);
}

void RendererDecorator::destroy() {
    this->_renderer->destroy();
}
void RendererDecorator::add(std::shared_ptr<TextBlock> text) {
    this->_renderer->add(text);
}

void RendererDecorator::draw(VkCommandBuffer commandBuffer) {
    this->_renderer->draw(commandBuffer);
}

void RendererDecorator::setUniformBuffers(UniformBufferObject ubo) {
    this->_renderer->setUniformBuffers(ubo);
}

void RendererDecorator::setViewportSize(unsigned int width, unsigned int height) {
    this->_renderer->setViewportSize(width, height);
}

void RendererDecorator::setCache(GlyphCache &cache) {
    this->_renderer->setCache(cache);
}

void RendererDecorator::setCacheSize(unsigned long maxSize) {
    this->_renderer->setCacheSize(maxSize);
}

VulkanContext RendererDecorator::getVulkanContext() {
    return this->_renderer->getVulkanContext();
}

}  // namespace vft
