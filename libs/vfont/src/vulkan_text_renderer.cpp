/**
 * @file vulkan_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_text_renderer.h"

namespace vft {

void VulkanTextRenderer::initialize() {
    TextRenderer::initialize();

    this->_createUbo();
    this->_createUboDescriptorSetLayout();
    this->_createDescriptorPool();
    this->_createUboDescriptorSet();
}

void VulkanTextRenderer::destroy() {
    this->_destroyBuffer(this->_uboBuffer, this->_uboMemory);

    if (this->_descriptorPool != nullptr)
        vkDestroyDescriptorPool(this->_logicalDevice, this->_descriptorPool, nullptr);
    if (this->_uboDescriptorSetLayout != nullptr)
        vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_uboDescriptorSetLayout, nullptr);
}

void VulkanTextRenderer::setUniformBuffers(UniformBufferObject ubo) {
    this->_ubo = ubo;
    memcpy(this->_mappedUbo, &this->_ubo, sizeof(this->_ubo));
}

/**
 * @brief Setter for vulkan physical device
 *
 * @param physicalDevice Vulkan physical device
 */
void VulkanTextRenderer::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
    if (physicalDevice == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setPhysicalDevice(): Physical device must not be null");
    }

    this->_physicalDevice = physicalDevice;
}

/**
 * @brief Setter for vulkan logical device
 *
 * @param logicalDevice Vulkan logical device
 */
void VulkanTextRenderer::setLogicalDevice(VkDevice logicalDevice) {
    if (logicalDevice == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setLogicalDevice(): Logical device must not be null");
    }

    this->_logicalDevice = logicalDevice;
}

/**
 * @brief Setter for vulkan command pool
 *
 * @param commandPool Vulkan command pool
 */
void VulkanTextRenderer::setCommandPool(VkCommandPool commandPool) {
    if (commandPool == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setCommandPool(): Command pool must not be null");
    }

    this->_commandPool = commandPool;
}

/**
 * @brief Setter for vulkan graphics queue
 *
 * @param graphicsQueue Vulkan graphics queue
 */
void VulkanTextRenderer::setGraphicsQueue(VkQueue graphicsQueue) {
    if (graphicsQueue == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setGraphicsQueue(): Graphics queue must not be null");
    }

    this->_graphicsQueue = graphicsQueue;
}

/**
 * @brief Setter for vulkan render pass
 *
 * @param renderPass Vulkan render pass
 */
void VulkanTextRenderer::setRenderPass(VkRenderPass renderPass) {
    if (renderPass == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setRenderPass(): Render pass must not be null");
    }

    this->_renderPass = renderPass;
}

/**
 * @brief Setter for vulkan command buffer
 *
 * @param commandBuffer Vulkan command buffer
 */
void VulkanTextRenderer::setCommandBuffer(VkCommandBuffer commandBuffer) {
    if (commandBuffer == nullptr) {
        throw std::invalid_argument("VulkanTextRenderer::setCommandBuffer(): Command buffer must not be null");
    }

    this->_commandBuffer = commandBuffer;
}

void VulkanTextRenderer::_createBuffer(VkDeviceSize size,
                                       VkBufferUsageFlags usage,
                                       VkMemoryPropertyFlags properties,
                                       VkBuffer &buffer,
                                       VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(this->_logicalDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("VulkanTextRenderer::_createBuffer(): Error creating vulkan buffer");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->_logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = _selectMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->_logicalDevice, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("VulkanTextRenderer::_createBuffer(): Error allocating vulkan buffer memory");
    }

    vkBindBufferMemory(this->_logicalDevice, buffer, bufferMemory, 0);
}

VkPhysicalDevice VulkanTextRenderer::getPhysicalDevice() {
    return this->_physicalDevice;
}

VkDevice VulkanTextRenderer::getLogicalDevice() {
    return this->_logicalDevice;
}

VkCommandPool VulkanTextRenderer::getCommandPool() {
    return this->_commandPool;
}

VkQueue VulkanTextRenderer::getGraphicsQueue() {
    return this->_graphicsQueue;
}

VkRenderPass VulkanTextRenderer::getRenderPass() {
    return this->_renderPass;
}

VkCommandBuffer VulkanTextRenderer::getCommandBuffer() {
    return this->_commandBuffer;
}

uint32_t VulkanTextRenderer::_selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryType & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error(
        "VulkanTextRenderer::_selectMemoryType(): Error selecting memory for vulkan vertex buffer");
}

void VulkanTextRenderer::_copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = this->_commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(this->_logicalDevice, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(this->_graphicsQueue);

    vkFreeCommandBuffers(this->_logicalDevice, this->_commandPool, 1, &commandBuffer);
}

void VulkanTextRenderer::_stageAndCreateVulkanBuffer(void *data,
                                                     VkDeviceSize size,
                                                     VkBufferUsageFlags destinationUsage,
                                                     VkBuffer &destinationBuffer,
                                                     VkDeviceMemory &destinationMemory) {
    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    this->_createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingBufferMemory);

    // Copy data from CPU to staging buffer
    void *buffer;
    vkMapMemory(this->_logicalDevice, stagingBufferMemory, 0, size, 0, &buffer);
    memcpy(buffer, data, size);
    vkUnmapMemory(this->_logicalDevice, stagingBufferMemory);

    // Copy data from staging buffer to newly created vulkan buffer
    this->_createBuffer(size, destinationUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        destinationBuffer, destinationMemory);
    this->_copyBuffer(stagingBuffer, destinationBuffer, size);

    // Destroy and deallocate memory from the staging buffer
    vkDestroyBuffer(this->_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->_logicalDevice, stagingBufferMemory, nullptr);
}

void VulkanTextRenderer::_destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    if (buffer == nullptr)
        return;

    vkDeviceWaitIdle(this->_logicalDevice);

    vkDestroyBuffer(this->_logicalDevice, buffer, nullptr);
    vkFreeMemory(this->_logicalDevice, bufferMemory, nullptr);

    buffer = nullptr;
    bufferMemory = nullptr;
}

std::vector<char> VulkanTextRenderer::_readFile(std::string fileName) {
    std::ifstream file{fileName, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        throw std::runtime_error("VulkanTextRenderer::_readFile(): Error opening file " + fileName);
    }

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

VkShaderModule VulkanTextRenderer::_createShaderModule(const std::vector<char> &shaderCode) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(this->_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("VulkanTextRenderer::_createShaderModule(): Error creating vulkan shader module");
    }

    return shaderModule;
}

void VulkanTextRenderer::_createUbo() {
    VkDeviceSize bufferSize = sizeof(vft::UniformBufferObject);

    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->_uboBuffer,
                        this->_uboMemory);

    vkMapMemory(this->_logicalDevice, this->_uboMemory, 0, bufferSize, 0, &this->_mappedUbo);
}

void VulkanTextRenderer::_createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(2);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(2);

    if (vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &(this->_descriptorPool)) !=
        VK_SUCCESS) {
        throw std::runtime_error("VulkanTextRenderer::_createDescriptorPool(): Error creating vulkan descriptor pool");
    }
}

void VulkanTextRenderer::_createUboDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.binding = 0;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                               VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if (vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr, &this->_uboDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTextRenderer::_createUboDescriptorSetLayout(): Error creating vulkan descriptor set layout");
    }
}

void VulkanTextRenderer::_createUboDescriptorSet() {
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &this->_uboDescriptorSetLayout;

    if (vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, &this->_uboDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTextRenderer::_createUboDescriptorSet(): Error allocating vulkan descriptor sets");
    }

    VkDescriptorBufferInfo descriptorBufferInfo{};
    descriptorBufferInfo.buffer = this->_uboBuffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = sizeof(vft::UniformBufferObject);

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = this->_uboDescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

    vkUpdateDescriptorSets(this->_logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

}  // namespace vft
