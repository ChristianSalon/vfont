/**
 * @file renderer_decorator.cpp
 * @author Christian Saloň
 */

#include "renderer_decorator.h"

namespace vft {

RendererDecorator::RendererDecorator(Renderer *renderer) : _renderer{renderer} {}

void RendererDecorator::init(TessellationStrategy tessellationStrategy,
                             VkPhysicalDevice physicalDevice,
                             VkDevice logicalDevice,
                             VkCommandPool commandPool,
                             VkQueue graphicsQueue,
                             VkRenderPass renderPass) {
    this->_renderer->init(tessellationStrategy, physicalDevice, logicalDevice, commandPool, graphicsQueue, renderPass);
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

}  // namespace vft
