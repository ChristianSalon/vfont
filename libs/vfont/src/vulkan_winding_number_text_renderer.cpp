﻿/**
 * @file vulkan_winding_number_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_winding_number_text_renderer.h"

namespace vft {

/**
 * @brief Initialize vulkan text renderer
 */
void VulkanWindingNumberTextRenderer::initialize() {
    VulkanTextRenderer::initialize();

    this->_tessellator = std::make_unique<WindingNumberTessellator>();
    this->_createSegmentsDescriptorSetLayout();
    this->_createSegmentsDescriptorSet();
    this->_createSegmentsPipeline();
}

/**
 * @brief Deallocate memory and destroy vulkan text renderer
 */
void VulkanWindingNumberTextRenderer::destroy() {
    // Destroy vulkan buffers
    if (this->_boundingBoxIndexBuffer != nullptr)
        this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    if (this->_segmentsBuffer != nullptr)
        this->_destroyBuffer(this->_segmentsBuffer, this->_segmentsBufferMemory);
    if (this->_vertexBuffer != nullptr)
        this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Destroy descriptor set
    if (this->_vertexBuffer != nullptr)
        vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_segmentsDescriptorSetLayout, nullptr);

    // Destroy pipeline
    if (this->_vertexBuffer != nullptr)
        vkDestroyPipeline(this->_logicalDevice, this->_segmentsPipeline, nullptr);
    if (this->_vertexBuffer != nullptr)
        vkDestroyPipelineLayout(this->_logicalDevice, this->_segmentsPipelineLayout, nullptr);

    VulkanTextRenderer::destroy();
}

/**
 * @brief Add draw commands to the command buffer for drawing all glyphs in tracked text blocks
 */
void VulkanWindingNumberTextRenderer::draw() {
    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    vkCmdBindPipeline(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_segmentsPipeline);

    std::array<VkDescriptorSet, 2> sets = {this->_uboDescriptorSet, this->_segmentsDescriptorSet};
    vkCmdBindDescriptorSets(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_segmentsPipelineLayout, 0,
                            sets.size(), sets.data(), 0, nullptr);

    VkBuffer vertexBuffers[] = {this->_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(this->_commandBuffer, 0, 1, vertexBuffers, offsets);

    // Draw line and curve segments
    vkCmdBindIndexBuffer(this->_commandBuffer, this->_boundingBoxIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};
            const Glyph &glyph = this->_cache->getGlyph(key);

            if (glyph.mesh.getVertexCount() > 0) {
                SegmentsInfo segmentsInfo =
                    this->_segmentsInfo.at(this->_offsets.at(key).at(SEGMENTS_INFO_OFFSET_BUFFER_INDEX));

                CharacterPushConstants pushConstants{
                    character.getModelMatrix(),           this->_textBlocks.at(i)->getColor(),
                    segmentsInfo.lineSegmentsStartIndex,  segmentsInfo.lineSegmentsCount,
                    segmentsInfo.curveSegmentsStartIndex, segmentsInfo.curveSegmentsCount};
                vkCmdPushConstants(this->_commandBuffer, this->_segmentsPipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(CharacterPushConstants), &pushConstants);

                vkCmdDrawIndexed(
                    this->_commandBuffer,
                    glyph.mesh.getIndexCount(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX), 1,
                    this->_offsets.at(key).at(BOUNDING_BOX_OFFSET_BUFFER_INDEX), 0, 0);
            }
        }
    }
}

/**
 * @brief Creates a new vertex and index buffer after a change in tracked text blocks
 */
void VulkanWindingNumberTextRenderer::update() {
    // Update glyph cache
    for (std::shared_ptr<TextBlock> block : this->_textBlocks) {
        for (const Character &character : block->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};
            if (!this->_cache->exists(key)) {
                // Tessellate glyph and insert into cache
                Glyph glyph = this->_tessellator->composeGlyph(character.getGlyphId(), character.getFont(),
                                                               character.getFontSize());
                this->_cache->setGlyph(key, glyph);
            }
        }
    }

    // Destroy vulkan buffers
    this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    this->_destroyBuffer(this->_segmentsBuffer, this->_segmentsBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan buffers
    this->_createVertexAndIndexBuffers();
}

/**
 * @brief Create vulkan vertex and index buffer for all glyphs in tracked text blocks
 */
void VulkanWindingNumberTextRenderer::_createVertexAndIndexBuffers() {
    this->_vertices.clear();
    this->_boundingBoxIndices.clear();
    this->_segments.clear();
    this->_segmentsInfo.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t boundingBoxIndexCount = 0;
    uint32_t segmentsCount = 0;
    uint32_t segmentsInfoCount = 0;

    for (int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};
            const Glyph &glyph = this->_cache->getGlyph(key);

            if (!this->_offsets.contains(key)) {
                this->_offsets.insert({key, {boundingBoxIndexCount, segmentsInfoCount}});

                this->_vertices.insert(this->_vertices.end(), glyph.mesh.getVertices().begin(),
                                       glyph.mesh.getVertices().end());
                this->_boundingBoxIndices.insert(
                    this->_boundingBoxIndices.end(),
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).end());

                std::vector<glm::vec2> vertices = glyph.mesh.getVertices();

                std::vector<uint32_t> lineSegments =
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_LINE_BUFFER_INDEX);
                uint32_t lineCount = lineSegments.size() / 2;
                for (int j = 0; j < lineSegments.size(); j += 2) {
                    this->_segments.push_back(vertices.at(lineSegments.at(j)));
                    this->_segments.push_back(vertices.at(lineSegments.at(j + 1)));
                }

                std::vector<uint32_t> curveSegments =
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX);
                uint32_t curveCount = curveSegments.size() / 3;
                for (int j = 0; j < curveSegments.size(); j += 3) {
                    this->_segments.push_back(vertices.at(curveSegments.at(j)));
                    this->_segments.push_back(vertices.at(curveSegments.at(j + 1)));
                    this->_segments.push_back(vertices.at(curveSegments.at(j + 2)));
                }

                this->_segmentsInfo.push_back(SegmentsInfo{
                    segmentsCount, lineCount, segmentsCount + static_cast<uint32_t>(lineSegments.size()), curveCount});

                // Add an offset to line segment indices of current character
                for (int j = boundingBoxIndexCount; j < this->_boundingBoxIndices.size(); j++) {
                    this->_boundingBoxIndices.at(j) += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                boundingBoxIndexCount +=
                    glyph.mesh.getIndexCount(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX);
                segmentsCount += lineSegments.size() + curveSegments.size();
                segmentsInfoCount++;
            }
        }
    }

    // Check if there are characters to render
    if (vertexCount == 0) {
        return;
    }

    // Create vertex buffer
    VkDeviceSize bufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();
    this->_stageAndCreateVulkanBuffer(this->_vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      this->_vertexBuffer, this->_vertexBufferMemory);

    if (boundingBoxIndexCount > 0) {
        // Create index buffer for bounding boxes
        VkDeviceSize bufferSize = sizeof(this->_boundingBoxIndices.at(0)) * this->_boundingBoxIndices.size();
        this->_stageAndCreateVulkanBuffer(this->_boundingBoxIndices.data(), bufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_boundingBoxIndexBuffer,
                                          this->_boundingBoxIndexBufferMemory);
    }

    // Check if at least one line or curve segment exists
    if (segmentsCount > 0) {
        this->_createSsbo();
    }
}

/**
 * @brief Create vulkan descriptor pool
 */
void VulkanWindingNumberTextRenderer::_createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(1);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = static_cast<uint32_t>(2);

    if (vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &(this->_descriptorPool)) !=
        VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanWindingNumberTextRenderer::_createDescriptorPool(): Error creating vulkan descriptor pool");
    }
}

/**
 * @brief Create vulkan ssbo containing line and curve segments of all glyphs
 */
void VulkanWindingNumberTextRenderer::_createSsbo() {
    VkDeviceSize segmentsBufferSize = sizeof(this->_segments.at(0)) * this->_segments.size();
    this->_stageAndCreateVulkanBuffer(this->_segments.data(), segmentsBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                      this->_segmentsBuffer, this->_segmentsBufferMemory);

    VkDescriptorBufferInfo lineSegmentsBufferInfo{};
    lineSegmentsBufferInfo.buffer = this->_segmentsBuffer;
    lineSegmentsBufferInfo.offset = 0;
    lineSegmentsBufferInfo.range = segmentsBufferSize;

    std::array<VkWriteDescriptorSet, 1> writeDescriptorSets = {};

    writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSets[0].dstSet = this->_segmentsDescriptorSet;
    writeDescriptorSets[0].dstBinding = 0;
    writeDescriptorSets[0].dstArrayElement = 0;
    writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSets[0].descriptorCount = 1;
    writeDescriptorSets[0].pBufferInfo = &lineSegmentsBufferInfo;

    vkUpdateDescriptorSets(this->_logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

/**
 * @brief Create vulkan ssbo descriptor set layout
 */
void VulkanWindingNumberTextRenderer::_createSegmentsDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings;

    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = layoutBindings.size();
    layoutCreateInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr,
                                    &(this->_segmentsDescriptorSetLayout)) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanWindingNumberTextRenderer::_createSegmentsDescriptorSetLayout(): Error creating vulkan descriptor "
            "set layout");
    }
}

/**
 * @brief Create vulkan ssbo descriptor set
 */
void VulkanWindingNumberTextRenderer::_createSegmentsDescriptorSet() {
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &this->_segmentsDescriptorSetLayout;

    if (vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, &this->_segmentsDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanWindingNumberTextRenderer::_createSegmentsDescriptorSet(): Error allocating vulkan descriptor sets");
    }
}

/**
 * @brief Create vulkan pipeline for displaying glyph using the winding number algorithm
 */
void VulkanWindingNumberTextRenderer::_createSegmentsPipeline() {
    std::vector<char> vertexShaderCode = this->_readFile("shaders/winding_number-vert.spv");
    std::vector<char> fragmentShaderCode = this->_readFile("shaders/winding_number-frag.spv");

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
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkVertexInputBindingDescription vertexInputBindingDescription{};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.stride = sizeof(glm::vec2);
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexInputAttributeDescription{};
    vertexInputAttributeDescription.binding = 0;
    vertexInputAttributeDescription.location = 0;
    vertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescription.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;

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
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(CharacterPushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {this->_uboDescriptorSetLayout,
                                                                 this->_segmentsDescriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr,
                               &this->_segmentsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanWindingNumberTextRenderer::_createSegmentsPipeline(): Error creating vulkan pipeline layout");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = this->_segmentsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &this->_segmentsPipeline) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanWindingNumberTextRenderer::_createSegmentsPipeline(): Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

}  // namespace vft
