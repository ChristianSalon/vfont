/**
 * @file vulkan_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_text_renderer.h"

namespace vft {

/**
 * @brief Initialize vulkan text renderer
 */
void VulkanTextRenderer::initialize() {
    TextRenderer::initialize();

    this->_createDescriptorPool();

    this->_createUbo();
    this->_createUboDescriptorSetLayout();
    this->_createUboDescriptorSet();
}

/**
 * @brief Deallocate memory and destroy vulkan text renderer
 */
void VulkanTextRenderer::destroy() {
    this->_destroyBuffer(this->_uboBuffer, this->_uboMemory);

    if (this->_descriptorPool != nullptr)
        vkDestroyDescriptorPool(this->_logicalDevice, this->_descriptorPool, nullptr);
    if (this->_uboDescriptorSetLayout != nullptr)
        vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_uboDescriptorSetLayout, nullptr);
}

/**
 * @brief Set vulkan uniform buffer to the uniform buffer object
 * @param ubo
 */
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

/**
 * @brief Getter for vulkan physical device
 *
 * @return Physical device
 */
VkPhysicalDevice VulkanTextRenderer::getPhysicalDevice() {
    return this->_physicalDevice;
}

/**
 * @brief Getter for vulkan logical device
 *
 * @return Logical device
 */
VkDevice VulkanTextRenderer::getLogicalDevice() {
    return this->_logicalDevice;
}

/**
 * @brief Getter for vulkan command pool
 *
 * @return Command pool
 */
VkCommandPool VulkanTextRenderer::getCommandPool() {
    return this->_commandPool;
}

/**
 * @brief Getter for vulkan graphics queue
 *
 * @return Graphics queue
 */
VkQueue VulkanTextRenderer::getGraphicsQueue() {
    return this->_graphicsQueue;
}

/**
 * @brief Getter for vulkan render pass
 *
 * @return Render pass
 */
VkRenderPass VulkanTextRenderer::getRenderPass() {
    return this->_renderPass;
}

/**
 * @brief Getter for vulkan command buffer
 *
 * @return Command buffer
 */
VkCommandBuffer VulkanTextRenderer::getCommandBuffer() {
    return this->_commandBuffer;
}

/**
 * @brief Create a vulkan buffer
 *
 * @param size Size of buffer
 * @param usage Vulkan buffer usage flags
 * @param properties Vulkan memory property flags
 * @param buffer Handle to vulkan buffer
 * @param bufferMemory Handle to vulkan buffer memory
 */
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

/**
 * @brief Selects a suitable memory type for Vulkan allocation.
 *
 * @param memoryType A bitmask specifying the memory type indices that are compatible
 * @param properties The required memory property flags
 *
 * @return The index of a suitable memory type with specified properties
 */
uint32_t VulkanTextRenderer::_selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryType & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("VulkanTextRenderer::_selectMemoryType(): Error selecting memory for vulkan buffer");
}

/**
 * @brief Copies data from one vulkan buffer to another
 *
 * This function records and submits a temporaray command buffer to copy data from the source buffer to the destination
 * buffer
 *
 * @param sourceBuffer Vulkan buffer containing the source data
 * @param destinationBuffer Vulkan buffer to copy data into
 * @param bufferSize The size of the data to copy, in bytes
 */
void VulkanTextRenderer::_copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize) {
    VkCommandBuffer commandBuffer = this->_beginOneTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    this->_endOneTimeCommands(commandBuffer);
}

/**
 * @brief Stages data and creates a vulkan buffer with the given usage
 *
 * This function first creates a temporary staging buffer in host-visible memory, copies the provided data into it, and
 * then transfers the data to the final vulkan buffer with the specified usage flags
 *
 * @param data Pointer to the data to be copied
 * @param size Size of the data to be copied in bytes
 * @param destinationUsage Vulkan buffer usage flags for the destination buffer
 * @param destinationBuffer The created vulkan buffer
 * @param destinationMemory The allocated vulkan device memory
 */
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

/**
 * @brief Destroys a vulkan buffer and frees its associated memory.
 *
 * @param buffer The vulkan buffer to be destroyed. It will be set to nullptr after destruction.
 * @param bufferMemory The vulkan device memory associated with the buffer. It will be set to nullptr after being freed.
 */
void VulkanTextRenderer::_destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    if (buffer == nullptr)
        return;

    vkDeviceWaitIdle(this->_logicalDevice);

    vkDestroyBuffer(this->_logicalDevice, buffer, nullptr);
    vkFreeMemory(this->_logicalDevice, bufferMemory, nullptr);

    buffer = nullptr;
    bufferMemory = nullptr;
}

/**
 * @brief Create and begin a command buffer used only once
 *
 * @return Created command buffer
 */
VkCommandBuffer VulkanTextRenderer::_beginOneTimeCommands() {
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

    return commandBuffer;
}

/**
 * @brief End and destroy command buffer used for one time commands
 *
 * @param commandBuffer Command buffer to destroy
 */
void VulkanTextRenderer::_endOneTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(this->_graphicsQueue);

    vkFreeCommandBuffers(this->_logicalDevice, this->_commandPool, 1, &commandBuffer);
}

/**
 * @brief Returns the content of file given a file name
 *
 * @param fileName Name of file to read
 *
 * @return Contents of given file in bytes
 */
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

/**
 * @brief Creates a vulkan shader module from the shader code
 *
 * @param shaderCode Contents of the shader
 *
 * @return Vulkan shader module
 */
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

/**
 * @brief Created a vulkan buffer with the contents of the uniform buffer object
 */
void VulkanTextRenderer::_createUbo() {
    VkDeviceSize bufferSize = sizeof(vft::UniformBufferObject);

    this->_createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->_uboBuffer,
                        this->_uboMemory);

    vkMapMemory(this->_logicalDevice, this->_uboMemory, 0, bufferSize, 0, &this->_mappedUbo);
}

/**
 * @brief Creates a vulkan descriptor pool
 */
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

/**
 * @brief Creates a vulkan descriptor set layout for the uniform buffer object
 */
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

/**
 * @brief Create a vulkan descriptor set for the uniform buffer object
 */
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
