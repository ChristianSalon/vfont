/**
 * @file vulkan_text_renderer_decorator.cpp
 * @author Christian Saloň
 */

#include "vulkan_text_renderer_decorator.h"

namespace vft {

/**
 * @brief VulkanTextRendererDecorator constructor
 *
 * @param renderer Vulkan text renderer
 */
VulkanTextRendererDecorator::VulkanTextRendererDecorator(VulkanTextRenderer *renderer) : _renderer{renderer} {}

/**
 * @brief VulkanTextRendererDecorator destructor
 */
VulkanTextRendererDecorator::~VulkanTextRendererDecorator() {
    delete this->_renderer;
}

/**
 * @brief Delegate initialization to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::initialize() {
    this->_renderer->initialize();
}

/**
 * @brief Delegate destruction to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::destroy() {
    this->_renderer->destroy();
}

/**
 * @brief Delegate adding text block to wrapped vulkan text renderer
 *
 * @param text Text block
 */
void VulkanTextRendererDecorator::add(std::shared_ptr<TextBlock> text) {
    this->_renderer->add(text);
}

/**
 * @brief Delegate draw to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::draw() {
    this->_renderer->draw();
}

/**
 * @brief Delegate update to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::update() {
    this->_renderer->update();
}

/**
 * @brief Delegate setUniformBuffers to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setUniformBuffers(UniformBufferObject ubo) {
    this->_renderer->setUniformBuffers(ubo);
}

/**
 * @brief Delegate setViewportSize to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setViewportSize(unsigned int width, unsigned int height) {
    this->_renderer->setViewportSize(width, height);
}

/**
 * @brief Delegate setCache to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setCache(std::shared_ptr<GlyphCache> cache) {
    this->_renderer->setCache(cache);
}

/**
 * @brief Delegate setPhysicalDevice to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
    this->_renderer->setPhysicalDevice(physicalDevice);
}

/**
 * @brief Delegate setLogicalDevice to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setLogicalDevice(VkDevice logicalDevice) {
    this->_renderer->setLogicalDevice(logicalDevice);
}

/**
 * @brief Delegate setCommandPool to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setCommandPool(VkCommandPool commandPool) {
    this->_renderer->setCommandPool(commandPool);
}

/**
 * @brief Delegate setGraphicsQueue to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setGraphicsQueue(VkQueue graphicsQueue) {
    this->_renderer->setGraphicsQueue(graphicsQueue);
}

/**
 * @brief Delegate setRenderPass to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setRenderPass(VkRenderPass renderPass) {
    this->_renderer->setRenderPass(renderPass);
}

/**
 * @brief Delegate setCommandBuffer to wrapped vulkan text renderer
 */
void VulkanTextRendererDecorator::setCommandBuffer(VkCommandBuffer commandBuffer) {
    this->_renderer->setCommandBuffer(commandBuffer);
}

/**
 * @brief Delegate getPhysicalDevice to wrapped vulkan text renderer
 */
VkPhysicalDevice VulkanTextRendererDecorator::getPhysicalDevice() {
    return this->_renderer->getPhysicalDevice();
}

/**
 * @brief Delegate getLogicalDevice to wrapped vulkan text renderer
 */
VkDevice VulkanTextRendererDecorator::getLogicalDevice() {
    return this->_renderer->getLogicalDevice();
}

/**
 * @brief Delegate getCommandPool to wrapped vulkan text renderer
 */
VkCommandPool VulkanTextRendererDecorator::getCommandPool() {
    return this->_renderer->getCommandPool();
}

/**
 * @brief Delegate getGraphicsQueue to wrapped vulkan text renderer
 */
VkQueue VulkanTextRendererDecorator::getGraphicsQueue() {
    return this->_renderer->getGraphicsQueue();
}

/**
 * @brief Delegate getRenderPass to wrapped vulkan text renderer
 */
VkRenderPass VulkanTextRendererDecorator::getRenderPass() {
    return this->_renderer->getRenderPass();
}

/**
 * @brief Delegate getCommandBuffer to wrapped vulkan text renderer
 */
VkCommandBuffer VulkanTextRendererDecorator::getCommandBuffer() {
    return this->_renderer->getCommandBuffer();
}

void VulkanTextRendererDecorator::addFontAtlas(const FontAtlas &atlas) {
    this->_renderer->addFontAtlas(atlas);
}

}  // namespace vft
