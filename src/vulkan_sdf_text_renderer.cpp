/**
 * @file vulkan_sdf_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_sdf_text_renderer.h"

namespace vft {

/**
 * @brief Initialize vulkan text renderer
 */
VulkanSdfTextRenderer::VulkanSdfTextRenderer(VkPhysicalDevice physicalDevice,
                                             VkDevice logicalDevice,
                                             VkQueue graphicsQueue,
                                             VkCommandPool commandPool,
                                             VkRenderPass renderPass,
                                             VkSampleCountFlagBits msaaSampleCount,
                                             VkCommandBuffer commandBuffer)
    : VulkanTextRenderer{physicalDevice, logicalDevice,   graphicsQueue, commandPool,
                         renderPass,     msaaSampleCount, commandBuffer} {
    this->_initialize();

    this->_createFontAtlasDescriptorSetLayout();
    this->_createPipeline();
}

/**
 * @brief Deallocate memory and destroy vulkan text renderer
 */
VulkanSdfTextRenderer::~VulkanSdfTextRenderer() {
    // Destroy textures
    for (auto &texture : this->_fontTextures) {
        vkDestroySampler(this->_logicalDevice, texture.second.sampler, nullptr);
        vkDestroyImageView(this->_logicalDevice, texture.second.imageView, nullptr);
        vkDestroyImage(this->_logicalDevice, texture.second.image, nullptr);
        vkFreeMemory(this->_logicalDevice, texture.second.memory, nullptr);
    }

    // Destroy vulkan buffers
    if (this->_boundingBoxIndexBuffer != nullptr)
        this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    if (this->_vertexBuffer != nullptr)
        this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Destroy descriptor set
    if (this->_fontAtlasDescriptorSetLayout != nullptr)
        vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_fontAtlasDescriptorSetLayout, nullptr);

    // Destroy pipeline
    if (this->_pipeline != nullptr)
        vkDestroyPipeline(this->_logicalDevice, this->_pipeline, nullptr);
    if (this->_pipelineLayout != nullptr)
        vkDestroyPipelineLayout(this->_logicalDevice, this->_pipelineLayout, nullptr);
}

/**
 * @brief Add draw commands to the command buffer for drawing all glyphs in tracked text blocks
 */
void VulkanSdfTextRenderer::draw() {
    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    vkCmdBindPipeline(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

    VkBuffer vertexBuffers[] = {this->_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(this->_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(this->_commandBuffer, this->_boundingBoxIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    std::string lastFontFamily = "";

    // Draw bounding boxes
    for (unsigned int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};

            if (this->_offsets.at(key).boundingBoxCount > 0) {
                if (character.getFont()->getFontFamily() != lastFontFamily) {
                    // Bind descriptor sets if font texture should change
                    std::array<VkDescriptorSet, 2> sets = {
                        this->_uboDescriptorSet,
                        this->_fontTextures.at(character.getFont()->getFontFamily()).descriptorSet};
                    vkCmdBindDescriptorSets(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            this->_pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);

                    lastFontFamily = character.getFont()->getFontFamily();
                }

                // Push constants
                CharacterPushConstants pushConstants{character.getModelMatrix(), this->_textBlocks.at(i)->getColor()};
                vkCmdPushConstants(this->_commandBuffer, this->_pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(CharacterPushConstants), &pushConstants);

                vkCmdDrawIndexed(this->_commandBuffer, this->_offsets.at(key).boundingBoxCount, 1,
                                 this->_offsets.at(key).boundingBoxOffset, 0, 0);
            }
        }
    }
}

/**
 * @brief Creates a new vertex and index buffer after a change in tracked text blocks
 */
void VulkanSdfTextRenderer::update() {
    SdfTextRenderer::update();

    // Destroy vulkan buffers
    this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    // Create vertex buffer
    VkDeviceSize vertexBufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();
    this->_stageAndCreateVulkanBuffer(this->_vertices.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      this->_vertexBuffer, this->_vertexBufferMemory);

    // Create index buffer for bounding boxes
    VkDeviceSize indexBufferSize = sizeof(this->_boundingBoxIndices.at(0)) * this->_boundingBoxIndices.size();
    this->_stageAndCreateVulkanBuffer(this->_boundingBoxIndices.data(), indexBufferSize,
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_boundingBoxIndexBuffer,
                                      this->_boundingBoxIndexBufferMemory);
}

/**
 * @brief Add font atlas and create the required vulkan objects used to rneder text using sdf textures
 *
 * @param atlas New font atlas
 */
void VulkanSdfTextRenderer::addFontAtlas(const FontAtlas &atlas) {
    SdfTextRenderer::addFontAtlas(atlas);

    // Create staging buffer
    VkDeviceSize size = atlas.getSize().x * atlas.getSize().y;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    this->_createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingBufferMemory);

    // Copy data from CPU to staging buffer
    void *buffer;
    vkMapMemory(this->_logicalDevice, stagingBufferMemory, 0, size, 0, &buffer);
    memcpy(buffer, atlas.getTexture().data(), size);
    vkUnmapMemory(this->_logicalDevice, stagingBufferMemory);

    // Create vulkan image of font atlas
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = atlas.getSize().x;
    imageCreateInfo.extent.height = atlas.getSize().y;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = VK_FORMAT_R8_UNORM;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image{};
    if (vkCreateImage(this->_logicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("VulkanSdfTextRenderer::addFontAtlas(): Could not create vulkan image for font atlas");
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(this->_logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex =
        this->_selectMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory imageMemory;
    if (vkAllocateMemory(this->_logicalDevice, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::addFontAtlas(): Could not allocate vulkan memory for font atlas");
    }

    vkBindImageMemory(this->_logicalDevice, image, imageMemory, 0);

    this->_transitionImageLayout(image, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    this->_copyBufferToImage(stagingBuffer, image, atlas.getSize().x, atlas.getSize().y);
    this->_transitionImageLayout(image, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create vulkan image view
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_R8_UNORM;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(this->_logicalDevice, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::addFontAtlas(): Could not create vulkan image view for font atlas");
    }

    // Create vulkan sampler
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = false;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = false;
    samplerCreateInfo.compareEnable = false;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler sampler;
    if (vkCreateSampler(this->_logicalDevice, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::addFontAtlas(): Could not create vulkan sampler for font atlas");
    }

    // Create descriptor set used when rendering with given font atlas
    VkDescriptorSet descriptorSet = this->_createFontAtlasDescriptorSet(imageView, sampler);

    FontTexture texture{image, imageMemory, imageView, sampler, descriptorSet};
    this->_fontTextures.insert({atlas.getFontFamily(), texture});

    // Destroy and deallocate memory from the staging buffer
    vkDestroyBuffer(this->_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->_logicalDevice, stagingBufferMemory, nullptr);
}

/**
 * @brief Create vulkan descriptor pool
 */
void VulkanSdfTextRenderer::_createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 64;

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = 65;

    if (vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &this->_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::_createDescriptorPool(): Error creating vulkan descriptor pool");
    }
}

/**
 * @brief Create vulkan descriptor set layout for font atlases
 */
void VulkanSdfTextRenderer::_createFontAtlasDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBinding.binding = 0;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if (vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr,
                                    &this->_fontAtlasDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::_createFontAtlasDescriptorSetLayout(): Error creating vulkan descriptor set "
            "layout");
    }
}

/**
 * @brief Create vulkan descriptor set for font atlas
 *
 * @param imageView Vulkan image view of font atlas
 * @param sampler Vulkan smapler of font atlas
 *
 * @return Created vulkan descriptor set for font atlas
 */
VkDescriptorSet VulkanSdfTextRenderer::_createFontAtlasDescriptorSet(VkImageView imageView, VkSampler sampler) {
    VkDescriptorSet descriptorSet{};

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &this->_fontAtlasDescriptorSetLayout;

    if (vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanSdfTextRenderer::_createFontAtlasDescriptorSet(): Error allocating vulkan descriptor sets");
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(this->_logicalDevice, 1, &writeDescriptorSet, 0, nullptr);

    return descriptorSet;
}

/**
 * @brief Transitions an image from one layout to another
 *
 * @param image Vulkan image to transition
 * @param format Format of the image
 * @param oldLayout Current layout of the image
 * @param newLayout Target layout to transition the image to
 */
void VulkanSdfTextRenderer::_transitionImageLayout(VkImage image,
                                                   VkFormat format,
                                                   VkImageLayout oldLayout,
                                                   VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = this->_beginOneTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("VulkanSdfTextRenderer::_transitionImageLayout(): Invalid layout transition");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    this->_endOneTimeCommands(commandBuffer);
}

/**
 * @brief Copy data from vulkan buffer to vulkan image
 *
 * @param buffer Source vulkan buffer
 * @param image Destination vulkan image
 * @param width Width of vulkan image
 * @param height Height of vulkan image
 */
void VulkanSdfTextRenderer::_copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = this->_beginOneTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = VkOffset3D{0, 0, 0};
    region.imageExtent = VkExtent3D{width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    this->_endOneTimeCommands(commandBuffer);
}

/**
 * @brief Create vulkan pipeline for displaying glyphs using sdfs
 */
void VulkanSdfTextRenderer::_createPipeline() {
    std::vector<char> vertexShaderCode = this->_readFile("shaders/sdf-vert.spv");
    std::vector<char> fragmentShaderCode = this->_readFile("shaders/sdf-frag.spv");

    VkShaderModule vertexShaderModule = this->_createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = this->_createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescriptions{};

    vertexInputAttributeDescriptions[0].binding = 0;
    vertexInputAttributeDescriptions[0].location = 0;
    vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescriptions[1].offset = offsetof(SdfTextRenderer::Vertex, position);

    vertexInputAttributeDescriptions[1].binding = 0;
    vertexInputAttributeDescriptions[1].location = 1;
    vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescriptions[1].offset = offsetof(SdfTextRenderer::Vertex, uv);

    VkVertexInputBindingDescription vertexInputBindingDescription{};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.stride = sizeof(SdfTextRenderer::Vertex);
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = this->_msaaSampleCount;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.attachmentCount = 1;

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(CharacterPushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayout, 2> setLayouts{this->_uboDescriptorSetLayout, this->_fontAtlasDescriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &this->_pipelineLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("VulkanSdfTextRenderer::_createPipeline(): Error creating vulkan pipeline layout");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    graphicsPipelineCreateInfo.layout = this->_pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &this->_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("VulkanSdfTextRenderer::_createPipeline(): Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

}  // namespace vft
