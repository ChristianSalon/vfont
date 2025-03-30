/**
 * @file vulkan_tessellation_shaders_text_renderer.cpp
 * @author Christian Saloň
 */

#include "vulkan_tessellation_shaders_text_renderer.h"

namespace vft {

/**
 * @brief Initialize vulkan text renderer
 * 
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param graphicsQueue Vulkan graphics queue
 * @param commandPool Vulkan command pool
 * @param renderPass Vulkan render pass
 * @param msaaSampleCount Number of samples used for multisampling
 * @param commandBuffer Vulkan command buffer
 */
VulkanTessellationShadersTextRenderer::VulkanTessellationShadersTextRenderer(VkPhysicalDevice physicalDevice,
                                                                             VkDevice logicalDevice,
                                                                             VkQueue graphicsQueue,
                                                                             VkCommandPool commandPool,
                                                                             VkRenderPass renderPass,
                                                                             VkSampleCountFlagBits msaaSampleCount,
                                                                             VkCommandBuffer commandBuffer)
    : VulkanTextRenderer{physicalDevice, logicalDevice,   graphicsQueue, commandPool,
                         renderPass,     msaaSampleCount, commandBuffer} {
    this->_initialize();

    this->_createLineSegmentsPipeline();
    this->_createCurveSegmentsPipeline();
}

/**
 * @brief Deallocate memory and destroy vulkan text renderer
 */
VulkanTessellationShadersTextRenderer::~VulkanTessellationShadersTextRenderer() {
    // Destroy vulkan buffers
    if (this->_lineSegmentsIndexBuffer != nullptr)
        this->_destroyBuffer(this->_lineSegmentsIndexBuffer, this->_lineSegmentsIndexBufferMemory);
    if (this->_curveSegmentsIndexBuffer != nullptr)
        this->_destroyBuffer(this->_curveSegmentsIndexBuffer, this->_curveSegmentsIndexBufferMemory);
    if (this->_vertexBuffer != nullptr)
        this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Destroy line segments pipeline
    if (this->_lineSegmentsPipeline != nullptr)
        vkDestroyPipeline(this->_logicalDevice, this->_lineSegmentsPipeline, nullptr);
    if (this->_lineSegmentsPipelineLayout != nullptr)
        vkDestroyPipelineLayout(this->_logicalDevice, this->_lineSegmentsPipelineLayout, nullptr);

    // Destroy curve segments pipeline
    if (this->_curveSegmentsPipeline != nullptr)
        vkDestroyPipeline(this->_logicalDevice, this->_curveSegmentsPipeline, nullptr);
    if (this->_curveSegmentsPipelineLayout != nullptr)
        vkDestroyPipelineLayout(this->_logicalDevice, this->_curveSegmentsPipelineLayout, nullptr);
}

/**
 * @brief Add draw commands to the command buffer for drawing all glyphs in tracked text blocks
 */
void VulkanTessellationShadersTextRenderer::draw() {
    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    vkCmdBindPipeline(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_lineSegmentsPipeline);
    vkCmdBindDescriptorSets(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_lineSegmentsPipelineLayout, 0,
                            1, &this->_uboDescriptorSet, 0, nullptr);

    VkBuffer vertexBuffers[] = {this->_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(this->_commandBuffer, 0, 1, vertexBuffers, offsets);

    // Draw line segments
    vkCmdBindIndexBuffer(this->_commandBuffer, this->_lineSegmentsIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (unsigned int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};

            if (this->_offsets.at(key).lineSegmentsCount > 0) {
                CharacterPushConstants pushConstants{character.getModelMatrix(), this->_textBlocks[i]->getColor()};
                vkCmdPushConstants(this->_commandBuffer, this->_lineSegmentsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                                   0, sizeof(vft::CharacterPushConstants), &pushConstants);

                vkCmdDrawIndexed(this->_commandBuffer, this->_offsets.at(key).lineSegmentsCount, 1,
                                 this->_offsets.at(key).lineSegmentsOffset, 0, 0);
            }
        }
    }

    // Draw curve segments
    vkCmdBindPipeline(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_curveSegmentsPipeline);
    vkCmdBindDescriptorSets(this->_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_curveSegmentsPipelineLayout,
                            0, 1, &this->_uboDescriptorSet, 0, nullptr);

    // vkCmdBindVertexBuffers(this->_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(this->_commandBuffer, this->_curveSegmentsIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    for (unsigned int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};
            const Glyph &glyph = this->_cache->getGlyph(key);

            if (this->_offsets.at(key).curveSegmentsCount > 0) {
                CharacterPushConstants pushConstants{character.getModelMatrix(), this->_textBlocks[i]->getColor()};
                vkCmdPushConstants(this->_commandBuffer, this->_curveSegmentsPipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(vft::CharacterPushConstants), &pushConstants);

                ViewportPushConstants viewportPushConstants{this->_viewportWidth, this->_viewportHeight};
                vkCmdPushConstants(this->_commandBuffer, this->_curveSegmentsPipelineLayout,
                                   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(vft::CharacterPushConstants),
                                   sizeof(ViewportPushConstants), &viewportPushConstants);

                vkCmdDrawIndexed(this->_commandBuffer, this->_offsets.at(key).curveSegmentsCount, 1,
                                 this->_offsets.at(key).curveSegmentsOffset, 0, 0);
            }
        }
    }
}

/**
 * @brief Creates a new vertex and index buffer after a change in tracked text blocks
 */
void VulkanTessellationShadersTextRenderer::update() {
    TessellationShadersTextRenderer::update();

    // Check if there are characters to render
    if (this->_vertices.size() == 0) {
        return;
    }

    // Destroy vulkan buffers
    this->_destroyBuffer(this->_lineSegmentsIndexBuffer, this->_lineSegmentsIndexBufferMemory);
    this->_destroyBuffer(this->_curveSegmentsIndexBuffer, this->_curveSegmentsIndexBufferMemory);
    this->_destroyBuffer(this->_vertexBuffer, this->_vertexBufferMemory);

    // Create vulkan vertex buffer
    VkDeviceSize bufferSize = sizeof(this->_vertices.at(0)) * this->_vertices.size();
    this->_stageAndCreateVulkanBuffer(this->_vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      this->_vertexBuffer, this->_vertexBufferMemory);

    // Check if at least one line segment exists
    if (this->_lineSegmentsIndices.size() > 0) {
        // Create vulkan index buffer for line segments
        VkDeviceSize bufferSize = sizeof(this->_lineSegmentsIndices.at(0)) * this->_lineSegmentsIndices.size();
        this->_stageAndCreateVulkanBuffer(this->_lineSegmentsIndices.data(), bufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_lineSegmentsIndexBuffer,
                                          this->_lineSegmentsIndexBufferMemory);
    }

    // Check if at least one curve segment exists
    if (this->_curveSegmentsIndices.size() > 0) {
        // Create vulkan index buffer for curve segments
        VkDeviceSize bufferSize = sizeof(this->_curveSegmentsIndices.at(0)) * this->_curveSegmentsIndices.size();
        this->_stageAndCreateVulkanBuffer(this->_curveSegmentsIndices.data(), bufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this->_curveSegmentsIndexBuffer,
                                          this->_curveSegmentsIndexBufferMemory);
    }
}

/**
 * @brief Create vulkan pipeline for displaying glyph's inner triangles
 */
void VulkanTessellationShadersTextRenderer::_createLineSegmentsPipeline() {
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
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

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

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr,
                               &this->_lineSegmentsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTessellationShadersTextRenderer::_createLineSegmentsPipeline(): Error creating vulkan pipeline "
            "layout");
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
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    graphicsPipelineCreateInfo.layout = this->_lineSegmentsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &this->_lineSegmentsPipeline) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTessellationShadersTextRenderer::_createLineSegmentsPipeline(): Error creating vulkan graphics "
            "pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

/**
 * @brief Create vulkan pipeline for displaying glyph's curve segments
 */
void VulkanTessellationShadersTextRenderer::_createCurveSegmentsPipeline() {
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
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    std::array<VkPushConstantRange, 2> pushConstantRanges;

    pushConstantRanges[0].size = sizeof(vft::CharacterPushConstants);
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pushConstantRanges[1].size = sizeof(ViewportPushConstants);
    pushConstantRanges[1].offset = sizeof(vft::CharacterPushConstants);
    pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &this->_uboDescriptorSetLayout;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();

    if (vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr,
                               &this->_curveSegmentsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTessellationShadersTextRenderer::_createCurveSegmentsPipeline(): Error creating vulkan pipeline "
            "layout");
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
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    graphicsPipelineCreateInfo.layout = this->_curveSegmentsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &this->_curveSegmentsPipeline) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanTessellationShadersTextRenderer::_createCurveSegmentsPipeline(): Error creating vulkan graphics "
            "pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, tcsShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, tesShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

}  // namespace vft
