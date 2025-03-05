/**
 * @file vulkan_text_renderer_decorator.cpp
 * @author Christian Saloň
 */

#include "vulkan_text_renderer_decorator.h"

namespace vft {

VulkanTextRendererDecorator::VulkanTextRendererDecorator(VulkanTextRenderer *renderer) : _renderer{renderer} {}

VulkanTextRendererDecorator::~VulkanTextRendererDecorator() {
    delete this->_renderer;
}

void VulkanTextRendererDecorator::initialize() {
    this->_renderer->initialize();
}

void VulkanTextRendererDecorator::destroy() {
    this->_renderer->destroy();
}
void VulkanTextRendererDecorator::add(std::shared_ptr<TextBlock> text) {
    this->_renderer->add(text);
}

void VulkanTextRendererDecorator::draw() {
    this->_renderer->draw();
}

void VulkanTextRendererDecorator::update() {
    this->_renderer->update();
}

void VulkanTextRendererDecorator::setUniformBuffers(UniformBufferObject ubo) {
    this->_renderer->setUniformBuffers(ubo);
}

void VulkanTextRendererDecorator::setViewportSize(unsigned int width, unsigned int height) {
    this->_renderer->setViewportSize(width, height);
}

void VulkanTextRendererDecorator::setCache(std::shared_ptr<GlyphCache> cache) {
    this->_renderer->setCache(cache);
}

void VulkanTextRendererDecorator::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
    this->_renderer->setPhysicalDevice(physicalDevice);
}

void VulkanTextRendererDecorator::setLogicalDevice(VkDevice logicalDevice) {
    this->_renderer->setLogicalDevice(logicalDevice);
}

void VulkanTextRendererDecorator::setCommandPool(VkCommandPool commandPool) {
    this->_renderer->setCommandPool(commandPool);
}

void VulkanTextRendererDecorator::setGraphicsQueue(VkQueue graphicsQueue) {
    this->_renderer->setGraphicsQueue(graphicsQueue);
}

void VulkanTextRendererDecorator::setRenderPass(VkRenderPass renderPass) {
    this->_renderer->setRenderPass(renderPass);
}

void VulkanTextRendererDecorator::setCommandBuffer(VkCommandBuffer commandBuffer) {
    this->_renderer->setCommandBuffer(commandBuffer);
}

VkPhysicalDevice VulkanTextRendererDecorator::getPhysicalDevice() {
    return this->_renderer->getPhysicalDevice();
}

VkDevice VulkanTextRendererDecorator::getLogicalDevice() {
    return this->_renderer->getLogicalDevice();
}

VkCommandPool VulkanTextRendererDecorator::getCommandPool() {
    return this->_renderer->getCommandPool();
}

VkQueue VulkanTextRendererDecorator::getGraphicsQueue() {
    return this->_renderer->getGraphicsQueue();
}

VkRenderPass VulkanTextRendererDecorator::getRenderPass() {
    return this->_renderer->getRenderPass();
}

VkCommandBuffer VulkanTextRendererDecorator::getCommandBuffer() {
    return this->_renderer->getCommandBuffer();
}

}  // namespace vft
