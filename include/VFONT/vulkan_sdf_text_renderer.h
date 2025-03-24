/**
 * @file vulkan_sdf_text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "font_atlas.h"
#include "glyph_cache.h"
#include "sdf_tessellator.h"
#include "vulkan_text_renderer.h"

namespace vft {

class VulkanSdfTextRenderer : public VulkanTextRenderer {
public:
    /** Index into the array containing index buffer offsets of glyph's bounding boxes */
    static constexpr unsigned int BOUNDING_BOX_OFFSET_BUFFER_INDEX = 0;

    /**
     * @brief Represents a font atlas texture containing vulkan objects used to render charcaters
     */
    class FontTexture {
    public:
        FontAtlas atlas;                        /**< Font atlas containing glyphs */
        VkImage image{nullptr};                 /**< Vulkan image containing font texture */
        VkDeviceMemory memory{nullptr};         /**< Vulkan memory containing the font texture */
        VkImageView imageView{nullptr};         /**< Vulkan image view of font texture */
        VkSampler sampler{nullptr};             /**< Vulkan sampler of font texture */
        VkDescriptorSet descriptorSet{nullptr}; /**< Vulkan descriptor set of font texure */

        FontTexture(const FontAtlas &atlas,
                    VkImage image,
                    VkDeviceMemory memory,
                    VkImageView imageView,
                    VkSampler sampler,
                    VkDescriptorSet descriptorSet)
            : atlas{atlas},
              image{image},
              memory{memory},
              imageView{imageView},
              sampler{sampler},
              descriptorSet{descriptorSet} {}
        FontTexture(const FontAtlas &atlas) : atlas{atlas} {}
    };

    /**
     * @brief Vertex structure used for rendering text using sdfs
     */
    struct Vertex {
        glm::vec2 position; /**< Vertex position */
        glm::vec2 uv;       /**< Vertex uv */
    };

protected:
    /**
     * Hash map containing font textures of selected font atlases containng info about glyphs (key: Font family, value:
     * FontTexture object)
     */
    std::unordered_map<std::string, FontTexture> _fontTextures{};

    /**
     * Hash map containing glyph offsets into the index buffers (key: glyph key, value: array of offsets into the index
     * buffer)
     */
    std::unordered_map<GlyphKey, std::array<uint32_t, 1>, GlyphKeyHash> _offsets{};

    std::vector<Vertex> _vertices{};             /**< Vertex buffer */
    std::vector<uint32_t> _boundingBoxIndices{}; /**< Index buffer containing boundig box indices */

    VkBuffer _vertexBuffer{nullptr};                       /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory{nullptr};           /**< Vulkan vertex buffer memory */
    VkBuffer _boundingBoxIndexBuffer{nullptr};             /**< Vulkan index buffer for bonding boxes */
    VkDeviceMemory _boundingBoxIndexBufferMemory{nullptr}; /**< Vulkan index buffer memory for bonding boxes  */

    VkPipelineLayout _pipelineLayout{nullptr}; /**< Vulkan pipeline layout for rendering glyphs using sdfs */
    VkPipeline _pipeline{nullptr};             /**< Vulkan pipeline for rendering glyphs using sdfs */

    VkDescriptorSetLayout _fontAtlasDescriptorSetLayout{nullptr}; /**< Vulkan descriptor set layout for font atlases */
    std::vector<VkDescriptorSet> _fontAtlasDescriptorSets{};      /**< Vulkan descriptor sets for font atlases */

public:
    VulkanSdfTextRenderer(VkPhysicalDevice physicalDevice,
                          VkDevice logicalDevice,
                          VkQueue graphicsQueue,
                          VkCommandPool commandPool,
                          VkRenderPass renderPass,
                          VkCommandBuffer commandBuffer = nullptr);
    virtual ~VulkanSdfTextRenderer();

    void draw() override;
    void update() override;

    void addFontAtlas(const FontAtlas &atlas) override;

protected:
    void _createVertexAndIndexBuffers();
    void _createPipeline();

    void _createDescriptorPool() override;
    void _createFontAtlasDescriptorSetLayout();
    VkDescriptorSet _createFontAtlasDescriptorSet(VkImageView imageView, VkSampler sampler);

    void _copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void _transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
};

}  // namespace vft
