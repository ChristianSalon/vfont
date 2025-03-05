/**
 * @file vulkan_triangulation_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_triangulation_text_renderer.h"

namespace vft {

void VulkanTriangulationTextRenderer::initialize() {
    VulkanTextRenderer::initialize();

    this->_tessellator = std::make_unique<TriangulationTessellator>();
    this->_createPipeline();
}

void VulkanTriangulationTextRenderer::destroy() {
    // Destroy vulkan buffers
    if (this->_indexBuffer != nullptr)
        this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    if (this->_vertexBuffer != nullptr)
        this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Destroy triangle pipeline
    if (this->_pipeline != nullptr)
        vkDestroyPipeline(this->_logicalDevice, this->_pipeline, nullptr);
    if (this->_pipelineLayout != nullptr)
        vkDestroyPipelineLayout(this->_logicalDevice, this->_pipelineLayout, nullptr);

    VulkanTextRenderer::destroy();
}

void VulkanTriangulationTextRenderer::draw() {
    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    vkCmdBindPipeline(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);
    vkCmdBindDescriptorSets(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout, 0, 1,
                            &this->_uboDescriptorSet, 0, nullptr);

    VkBuffer vertexBuffers[] = {this->_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(this->_commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(this->_commandBuffer, this->_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), character.getFontSize()};
            const Glyph &glyph = this->_cache->getGlyph(key);

            if (glyph.mesh.getVertexCount() > 0) {
                vft::CharacterPushConstants pushConstants{character.getModelMatrix(), this->_textBlocks[i]->getColor()};
                vkCmdPushConstants(this->_commandBuffer, this->_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                   sizeof(vft::CharacterPushConstants), &pushConstants);

                vkCmdDrawIndexed(this->_commandBuffer, glyph.mesh.getIndexCount(0), 1, this->_offsets.at(key), 0, 0);
            }
        }
    }
}

void VulkanTriangulationTextRenderer::update() {
    // Update glyph cache
    for (std::shared_ptr<TextBlock> block : this->_textBlocks) {
        for (const Character &character : block->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), character.getFontSize()};
            if (!this->_cache->exists(key)) {
                // Tessellate glyph and insert into cache
                Glyph glyph = this->_tessellator->composeGlyph(character.getGlyphId(), character.getFont(),
                                                               character.getFontSize());
                this->_cache->setGlyph(key, glyph);
            }
        }
    }

    // Destroy vulkan buffers
    this->_destroyBuffer(this->_indexBuffer, this->_indexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan buffers, make sure all glyphs to be rendered are in cache
    this->_createVertexAndIndexBuffers();
}

void VulkanTriangulationTextRenderer::_createVertexAndIndexBuffers() {
    this->_vertices.clear();
    this->_indices.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    for (int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), character.getFontSize()};

            if (!this->_offsets.contains(key)) {
                this->_offsets.insert({key, indexCount});

                const Glyph &glyph = this->_cache->getGlyph(key);
                this->_vertices.insert(this->_vertices.end(), glyph.mesh.getVertices().begin(),
                                       glyph.mesh.getVertices().end());
                this->_indices.insert(this->_indices.end(), glyph.mesh.getIndices(0).begin(),
                                      glyph.mesh.getIndices(0).end());

                // Add an offset to line segment indices of current character
                for (int j = indexCount; j < this->_indices.size(); j++) {
                    this->_indices.at(j) += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                indexCount += glyph.mesh.getIndexCount(0);
            }
        }
    }

    // Check if there are characters to render
    if (vertexCount == 0) {
        return;
    }

    // Create vertex buffer
    VkDeviceSize vertexBufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();
    this->_stageAndCreateVulkanBuffer(this->_vertices.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      this->_vertexBuffer, this->_vertexBufferMemory);

    // Create index buffer
    VkDeviceSize indexBufferSize = sizeof(this->_indices.at(0)) * this->_indices.size();
    this->_stageAndCreateVulkanBuffer(this->_indices.data(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                      this->_indexBuffer, this->_indexBufferMemory);
}

void VulkanTriangulationTextRenderer::_createPipeline() {
    std::vector<char> vertexShaderCode = this->_readFile("shaders/triangle-vert.spv");
    std::vector<char> fragmentShaderCode = this->_readFile("shaders/triangle-frag.spv");

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
    pushConstantRange.size = sizeof(vft::CharacterPushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &this->_uboDescriptorSetLayout;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &this->_pipelineLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTriangulationTextRenderer::_createPipeline(): Error creating vulkan pipeline layout");
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
    graphicsPipelineCreateInfo.layout = this->_pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &this->_pipeline) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTriangulationTextRenderer::_createPipeline(): Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

}  // namespace vft
