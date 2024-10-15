/**
 * @file gpu_drawer.cpp
 * @author Christian Saloň
 */

#include <glm/vec4.hpp>
#include <iostream>

#include "gpu_drawer.h"

namespace vft {

GpuDrawer::GpuDrawer(GlyphCache& cache) : Drawer{ cache } {};

GpuDrawer::~GpuDrawer() {
    // Destroy vulkan buffers
    if (this->_boundingBoxIndexBuffer != nullptr)
        this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);

    if (this->_curveSegmentsIndexBuffer != nullptr)
        this->_destroyBuffer(this->_curveSegmentsIndexBuffer, this->_curveSegmentsIndexBufferMemory);

    if (this->_lineSegmentsBuffer != nullptr)
        this->_destroyBuffer(this->_lineSegmentsBuffer, this->_lineSegmentsBufferMemory);

    if (this->_vertexBuffer != nullptr)
        this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_lineSegmentsDescriptorSetLayout, nullptr);

    vkDestroyPipeline(this->_logicalDevice, this->_lineSegmentsPipeline, nullptr);
    vkDestroyPipelineLayout(this->_logicalDevice, this->_lineSegmentsPipelineLayout, nullptr);

    vkDestroyPipeline(this->_logicalDevice, this->_curveSegmentsPipeline, nullptr);
    vkDestroyPipelineLayout(this->_logicalDevice, this->_curveSegmentsPipelineLayout, nullptr);
}

void GpuDrawer::init(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    VkRenderPass renderPass
) {
    Drawer::init(physicalDevice, logicalDevice, commandPool, graphicsQueue, renderPass);

    this->_createLineSegmentsDescriptorSetLayout();
    this->_createLineSegmentsDescriptorSets();

    this->_createLineSegmentsPipeline();
    this->_createCurveSegmentsPipeline();
}

void GpuDrawer::draw(std::vector<std::shared_ptr<TextBlock>> textBlocks, VkCommandBuffer commandBuffer) {
    static size_t i = 0;

    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_lineSegmentsPipeline);

    std::array<VkDescriptorSet, 2> sets = { this->_uboDescriptorSets.at(i % 2), this->_lineSegmentsDescriptorSets.at(0) };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_lineSegmentsPipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);

    VkBuffer vertexBuffers[] = { this->_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Draw line segments
    vkCmdBindIndexBuffer(commandBuffer, this->_boundingBoxIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (int i = 0; i < textBlocks.size(); i++) {
        for (Character& character : textBlocks[i]->getCharacters()) {
            if (character.glyph.mesh.getVertexCount() > 0) {
                GlyphKey key{ textBlocks.at(i)->getFont()->getFontFamily(), character.getUnicodeCodePoint() };

                LineSegmentsInfo segmentsInfo = this->_lineSegmentsInfo.at(this->_offsets.at(key).at(LINE_SEGMENTS_INFO_OFFSET_BUFFER_INDEX));
                PushConstants pushConstants{ character.getModelMatrix(), textBlocks.at(i)->getColor(), segmentsInfo.startIndex, segmentsInfo.count };
                vkCmdPushConstants(commandBuffer, this->_lineSegmentsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);

                vkCmdDrawIndexed(commandBuffer, character.glyph.mesh.getIndexCount(GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX), 1, this->_offsets.at(key).at(BOUNDING_BOX_OFFSET_BUFFER_INDEX), 0, 0);
            }
        }
    }

    // Draw curve segments
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_curveSegmentsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_curveSegmentsPipelineLayout, 0, 1, &(this->_uboDescriptorSets.at(i % 2)), 0, nullptr);

    // vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, this->_curveSegmentsIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (int i = 0; i < textBlocks.size(); i++) {
        for (Character& character : textBlocks[i]->getCharacters()) {
            if (character.glyph.mesh.getVertexCount() > 0) {
                GlyphKey key{ textBlocks.at(i)->getFont()->getFontFamily(), character.getUnicodeCodePoint() };

                vft::CharacterPushConstants pushConstants{ character.getModelMatrix(), textBlocks.at(i)->getColor() };
                vkCmdPushConstants(commandBuffer, this->_curveSegmentsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vft::CharacterPushConstants), &pushConstants);

                vkCmdDrawIndexed(commandBuffer, character.glyph.mesh.getIndexCount(GLYPH_MESH_CURVE_BUFFER_INDEX), 1, this->_offsets.at(key).at(CURVE_OFFSET_BUFFER_INDEX), 0, 0);
            }
        }
    }

    i++;
}

void GpuDrawer::recreateBuffers(std::vector<std::shared_ptr<TextBlock>> textBlocks) {
    // Destroy vulkan buffers
    this->_destroyBuffer(this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    this->_destroyBuffer(this->_curveSegmentsIndexBuffer, this->_curveSegmentsIndexBufferMemory);
    this->_destroyBuffer(this->_lineSegmentsBuffer, this->_lineSegmentsBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan buffers
    this->_createVertexAndIndexBuffers(textBlocks);
}

void GpuDrawer::_createVertexAndIndexBuffers(std::vector<std::shared_ptr<TextBlock>>& textBlocks) {
    this->_vertices.clear();
    this->_boundingBoxIndices.clear();
    this->_curveSegmentsIndices.clear();
    this->_lineSegments.clear();
    this->_lineSegmentsInfo.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t boundingBoxIndexCount = 0;
    uint32_t curveSegmentsIndexCount = 0;
    uint32_t lineSegmentsCount = 0;
    uint32_t lineSegmentsInfoCount = 0;

    for (int i = 0; i < textBlocks.size(); i++) {
        for (Character& character : textBlocks[i]->getCharacters()) {
            GlyphKey key{ textBlocks[i]->getFont()->getFontFamily(), character.getUnicodeCodePoint() };
            if (!this->_offsets.contains(key)) {
                this->_offsets.insert({ key, { boundingBoxIndexCount, curveSegmentsIndexCount, lineSegmentsInfoCount } });

                this->_vertices.insert(this->_vertices.end(), character.glyph.mesh.getVertices().begin(), character.glyph.mesh.getVertices().end());
                this->_boundingBoxIndices.insert(this->_boundingBoxIndices.end(), character.glyph.mesh.getIndices(GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).begin(), character.glyph.mesh.getIndices(GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).end());
                this->_curveSegmentsIndices.insert(this->_curveSegmentsIndices.end(), character.glyph.mesh.getIndices(GLYPH_MESH_CURVE_BUFFER_INDEX).begin(), character.glyph.mesh.getIndices(GLYPH_MESH_CURVE_BUFFER_INDEX).end());

                std::vector<glm::vec2> vertices = character.glyph.mesh.getVertices();
                std::vector<uint32_t> lineSegments = character.glyph.mesh.getIndices(GLYPH_MESH_LINE_BUFFER_INDEX);
                uint32_t lineCount = 0;
                for (int j = 0; j < lineSegments.size(); j += 2) {
                    this->_lineSegments.push_back(LineSegment{ vertices.at(lineSegments.at(j)), vertices.at(lineSegments.at(j + 1)) });
                    lineCount++;
                }

                this->_lineSegmentsInfo.push_back(LineSegmentsInfo{ lineSegmentsCount, lineCount });

                // Add an offset to line segment indices of current character
                for (int j = boundingBoxIndexCount; j < this->_boundingBoxIndices.size(); j++) {
                    this->_boundingBoxIndices.at(j) += vertexCount;
                }

                // Add an offset to curve segment indices of current character
                for (int j = curveSegmentsIndexCount; j < this->_curveSegmentsIndices.size(); j++) {
                    this->_curveSegmentsIndices.at(j) += vertexCount;
                }

                vertexCount += character.glyph.mesh.getVertexCount();
                boundingBoxIndexCount += character.glyph.mesh.getIndexCount(GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX);
                curveSegmentsIndexCount += character.glyph.mesh.getIndexCount(GLYPH_MESH_CURVE_BUFFER_INDEX);
                lineSegmentsCount += lineCount;
                lineSegmentsInfoCount++;
            }
        }
    }

    // Check if there are characters to render
    if (vertexCount == 0) {
        return;
    }

    // Create vertex buffer
    VkDeviceSize bufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();
    this->_stageAndCreateVulkanBuffer(this->_vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, this->_vertexBuffer, this->_vertexBufferMemory);
    
    if (boundingBoxIndexCount > 0) {
        // Create index buffer for bounding boxes
        VkDeviceSize bufferSize = sizeof(this->_boundingBoxIndices.at(0)) * this->_boundingBoxIndices.size();
        this->_stageAndCreateVulkanBuffer(this->_boundingBoxIndices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_boundingBoxIndexBuffer, this->_boundingBoxIndexBufferMemory);
    }

    // Check if at least one curve segment exists
    if (curveSegmentsIndexCount > 0) {
        // Create index buffer for curve segments
        VkDeviceSize bufferSize = sizeof(this->_curveSegmentsIndices.at(0)) * this->_curveSegmentsIndices.size();
        this->_stageAndCreateVulkanBuffer(this->_curveSegmentsIndices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_curveSegmentsIndexBuffer, this->_curveSegmentsIndexBufferMemory);
    }

    // Check if at least one line segment exists
    if (lineSegmentsCount > 0) {
        this->_createSsbo();
    }
}

void GpuDrawer::_createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(2);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(2);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = static_cast<uint32_t>(4);

    if (vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &(this->_descriptorPool)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor pool");
    }
}

void GpuDrawer::_createSsbo() {
    VkDeviceSize lineSegmentsBufferSize = sizeof(this->_lineSegments.at(0)) * this->_lineSegments.size();
    this->_stageAndCreateVulkanBuffer(this->_lineSegments.data(), lineSegmentsBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, this->_lineSegmentsBuffer, this->_lineSegmentsBufferMemory);

    VkDescriptorBufferInfo lineSegmentsBufferInfo{};
    lineSegmentsBufferInfo.buffer = this->_lineSegmentsBuffer;
    lineSegmentsBufferInfo.offset = 0;
    lineSegmentsBufferInfo.range = lineSegmentsBufferSize;

    std::array<VkWriteDescriptorSet, 1> writeDescriptorSets = {};

    writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSets[0].dstSet = this->_lineSegmentsDescriptorSets.at(0);
    writeDescriptorSets[0].dstBinding = 0;
    writeDescriptorSets[0].dstArrayElement = 0;
    writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSets[0].descriptorCount = 1;
    writeDescriptorSets[0].pBufferInfo = &lineSegmentsBufferInfo;

    vkUpdateDescriptorSets(this->_logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

    /* this->_ssbo.resize(2);
    this->_ssboMemory.resize(2);
    this->_mappedSSBO.resize(2);

    VkDeviceSize bufferSize = sizeof(this->_lineSegments.at(0)) * this->_lineSegments.size();

    for (int i = 0; i < 2; i++) {
        this->_stageAndCreateVulkanBuffer(this->_lineSegments.data(), bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, this->_ssbo.at(i), this->_ssboMemory.at(i));

        VkDescriptorBufferInfo ssboBufferInfo{};
        ssboBufferInfo.buffer = this->_ssbo.at(i);
        ssboBufferInfo.offset = 0;
        ssboBufferInfo.range = bufferSize;

        std::array<VkWriteDescriptorSet, 1> writeDescriptorSets = {};

        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstSet = this->_lineSegmentsDescriptorSets.at(i);
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].pBufferInfo = &ssboBufferInfo;

        vkUpdateDescriptorSets(this->_logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
    } */
}

void GpuDrawer::_createLineSegmentsDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings;

    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = layoutBindings.size();
    layoutCreateInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr, &(this->_lineSegmentsDescriptorSetLayout)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor set layout");
    }

}

void GpuDrawer::_createLineSegmentsDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(2, this->_lineSegmentsDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = layouts.size();
    allocateInfo.pSetLayouts = layouts.data();

    this->_lineSegmentsDescriptorSets.resize(layouts.size());

    if (vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, this->_lineSegmentsDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan descriptor sets");
    }
}

void GpuDrawer::_createLineSegmentsPipeline() {
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

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkVertexInputBindingDescription vertexInputBindingDescription = vft::getVertexInutBindingDescription();
    VkVertexInputAttributeDescription vertexInputAttributeDescription = vft::getVertexInputAttributeDescription();

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
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
    pushConstantRange.size = sizeof(PushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { this->_uboDescriptorSetLayout, this->_lineSegmentsDescriptorSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &this->_lineSegmentsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan pipeline layout");
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
    graphicsPipelineCreateInfo.layout = this->_lineSegmentsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &this->_lineSegmentsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

void GpuDrawer::_createCurveSegmentsPipeline() {
    std::vector<char> vertexShaderCode = this->_readFile("shaders/curve-vert.spv");
    std::vector<char> tcsCode = this->_readFile("shaders/curve-tesc.spv");
    std::vector<char> tesCode = this->_readFile("shaders/curve-tese.spv");
    std::vector<char> fragmentShaderCode = this->_readFile("shaders/curve-frag.spv");

    VkShaderModule vertexShaderModule = this->_createShaderModule(vertexShaderCode);
    VkShaderModule tcsShaderModule = this->_createShaderModule(tcsCode);
    VkShaderModule tesShaderModule = this->_createShaderModule(tesCode);
    VkShaderModule fragmentShaderModule = this->_createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo tcsShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    vertexShaderStageCreateInfo.module = tcsShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo tesShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    vertexShaderStageCreateInfo.module = tesShaderModule;
    vertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[4] = {};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShaderModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    shaderStages[1].module = tcsShaderModule;
    shaderStages[1].pName = "main";

    shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[2].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    shaderStages[2].module = tesShaderModule;
    shaderStages[2].pName = "main";

    shaderStages[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[3].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[3].module = fragmentShaderModule;
    shaderStages[3].pName = "main";

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkVertexInputBindingDescription vertexInputBindingDescription = vft::getVertexInutBindingDescription();
    VkVertexInputAttributeDescription vertexInputAttributeDescription = vft::getVertexInputAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
    tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationStateCreateInfo.patchControlPoints = 3;

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
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
    pushConstantRange.size = sizeof(vft::CharacterPushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &this->_uboDescriptorSetLayout;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &this->_curveSegmentsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan pipeline layout");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 4;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pTessellationState = &tessellationStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = this->_curveSegmentsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &this->_curveSegmentsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, tcsShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, tesShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

}
