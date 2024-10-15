/**
 * @file drawer.cpp
 * @author Christian Saloň
 */

#include "drawer.h"

namespace vft {

Drawer::Drawer(GlyphCache &cache) : _cache{ cache } {};

Drawer::~Drawer() {
    for (int i = 0; i < 2; i++) {
        this->_destroyBuffer(this->_ubo.at(i), this->_uboMemory.at(i));
    }

    vkDestroyDescriptorPool(this->_logicalDevice, this->_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_uboDescriptorSetLayout, nullptr);
}

void Drawer::init(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkRenderPass renderPass
) {
    this->_physicalDevice = physicalDevice;
    this->_logicalDevice = logicalDevice;
    this->_commandPool = commandPool;
    this->_graphicsQueue = graphicsQueue;
    this->_renderPass = renderPass;

    this->_createUbo();

    this->_createUboDescriptorSetLayout();
    this->_createDescriptorPool();
    this->_createUboDescriptorSets();
}

void Drawer::setUniformBuffers(vft::UniformBufferObject ubo) {
    static size_t i = 0;

    memcpy(this->_mappedUbo.at((i++) % 2), &ubo, sizeof(ubo));
}

uint32_t Drawer::_selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryType & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Error selecting memory for vulkan vertex buffer");
}

void Drawer::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(this->_logicalDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan buffer");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->_logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = _selectMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->_logicalDevice, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan buffer memory");
    }

    vkBindBufferMemory(this->_logicalDevice, buffer, bufferMemory, 0);
}

void Drawer::_copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize) {
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

void Drawer::_stageAndCreateVulkanBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags destinationUsage, VkBuffer& destinationBuffer, VkDeviceMemory& destinationMemory) {
    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    this->_createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // Copy data from CPU to staging buffer
    void* buffer;
    vkMapMemory(this->_logicalDevice, stagingBufferMemory, 0, size, 0, &buffer);
    memcpy(buffer, data, size);
    vkUnmapMemory(this->_logicalDevice, stagingBufferMemory);

    // Copy data from staging buffer to newly created vulkan buffer
    this->_createBuffer(size, destinationUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, destinationBuffer, destinationMemory);
    this->_copyBuffer(stagingBuffer, destinationBuffer, size);

    // Destroy and deallocate memory from the staging buffer
    vkDestroyBuffer(this->_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->_logicalDevice, stagingBufferMemory, nullptr);
}

void Drawer::_destroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    if (buffer == nullptr)
        return;

    vkDeviceWaitIdle(this->_logicalDevice);

    vkDestroyBuffer(this->_logicalDevice, buffer, nullptr);
    vkFreeMemory(this->_logicalDevice, bufferMemory, nullptr);

    buffer = nullptr;
    bufferMemory = nullptr;
}

std::vector<char> Drawer::_readFile(std::string fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Error opening file " + fileName);
    }

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

VkShaderModule Drawer::_createShaderModule(const std::vector<char>& shaderCode) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(this->_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan shader module");
    }

    return shaderModule;
}

void Drawer::_createUbo() {
    VkDeviceSize bufferSize = sizeof(vft::UniformBufferObject);

    this->_ubo.resize(2);
    this->_uboMemory.resize(2);
    this->_mappedUbo.resize(2);

    for (int i = 0; i < 2; i++) {
        this->_createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            this->_ubo.at(i),
            this->_uboMemory.at(i)
        );

        vkMapMemory(this->_logicalDevice, this->_uboMemory.at(i), 0, bufferSize, 0, &(this->_mappedUbo.at(i)));
    }
}

void Drawer::_createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(2);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(2);

    if (vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &(this->_descriptorPool)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor pool");
    }
}

void Drawer::_createUboDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.binding = 0;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if (vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr, &(this->_uboDescriptorSetLayout)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor set layout");
    }

}

void Drawer::_createUboDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(2, this->_uboDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(2);
    allocateInfo.pSetLayouts = layouts.data();

    this->_uboDescriptorSets.resize(2);

    if (vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, this->_uboDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan descriptor sets");
    }

    for (int i = 0; i < 2; i++) {
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = this->_ubo.at(i);
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(vft::UniformBufferObject);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = this->_uboDescriptorSets.at(i);
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        vkUpdateDescriptorSets(this->_logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
    }
}

}
