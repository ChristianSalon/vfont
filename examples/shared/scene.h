/**
 * @file scene.h
 * @author Christian Saloň
 */

#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>

#include <VFONT/font.h>
#include <VFONT/i_vulkan_text_renderer.h>
#include <VFONT/text_block.h>
#include <VFONT/text_renderer.h>
#include <VFONT/vulkan_sdf_text_renderer.h>
#include <VFONT/vulkan_tessellation_shaders_text_renderer.h>
#include <VFONT/vulkan_text_renderer.h>
#include <VFONT/vulkan_timed_renderer.h>
#include <VFONT/vulkan_triangulation_text_renderer.h>
#include <VFONT/vulkan_winding_number_text_renderer.h>

#include "base_camera.h"
#include "ortho_camera.h"
#include "perspective_camera.h"
#include "window.h"

/**
 * @brief Base class for scenes rendered with vulksn
 */
class Scene {
protected:
    /**
     * @brief Represents supported vulkan queue families of physical device
     */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    /**
     * @brief Represents the swap chain capabilities of physical device
     */
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
        VkCompositeAlphaFlagBitsKHR compositeAlphaMode;
    };

    std::vector<const char *> extensions;       /**< Selected vulkan extensions */
    std::vector<const char *> deviceExtensions; /**< Selected vulkan device extensions */
    std::vector<const char *> validationLayers; /**< Selected vulkan validation layers */

    bool _measureTime{false};                                     /**< Indicates whether to measure gpu draw time */
    std::shared_ptr<vft::IVulkanTextRenderer> _renderer{nullptr}; /**< Default text renderer */
    std::shared_ptr<MainWindow> _window{nullptr};                 /**< Application window */
    std::unique_ptr<BaseCamera> _camera{nullptr};                 /**< Camera object */
    CameraType _cameraType{CameraType::PERSPECTIVE};              /**< Type of camera used for rendering */

    VkInstance _instance{nullptr};                       /**< Vulkan instance */
    VkSurfaceKHR _surface{nullptr};                      /**< Vulkan surface */
    VkPhysicalDevice _physicalDevice{nullptr};           /**< Vulkan physical device */
    VkDevice _logicalDevice{nullptr};                    /**< Vulkan logical device */
    VkQueue _graphicsQueue{nullptr};                     /**< Vulkan graphics queue */
    VkQueue _presentQueue{nullptr};                      /**< Vulkan present queue */
    VkSwapchainKHR _swapChain{nullptr};                  /**< Vulkan swap chain */
    std::vector<VkImage> _swapChainImages{};             /**< Vulkan swap chain images */
    VkFormat _swapChainImageFormat{VK_FORMAT_UNDEFINED}; /**< Vulkan swap chain image format */
    VkExtent2D _swapChainExtent;                         /**< Vulkan swap chain extent */
    std::vector<VkImageView> _swapChainImageViews{};     /**< Vulkan swap chain image views */
    VkRenderPass _renderPass{nullptr};                   /**< Vulkan render pass */
    std::vector<VkFramebuffer> _framebuffers{};          /**< Vulkan frame buffers */
    VkCommandPool _commandPool{nullptr};                 /**< Vulkan command pool */
    VkCommandBuffer _commandBuffer{nullptr};             /**< Vulkan command buffers */
    VkSemaphore _imageAvailableSemaphore{
        nullptr}; /**< Semaphore to signal that an image was acquired from the swap chain */
    VkSemaphore _renderFinishedSemaphore{
        nullptr};                    /**< Semaphore to signal if vulkan finished rendering and can begin presenting */
    VkFence _inFlightFence{nullptr}; /**< Fence to ensure only one frame at a time is being rendered */

    VkImage _depthImage{nullptr};              /**< Depth buffer vulkan image */
    VkImageView _depthImageView{nullptr};      /**< Depth buffer vulkan image view */
    VkDeviceMemory _depthImageMemory{nullptr}; /**< Depth buffer vulkan image memory */

    bool _useMsaa{false};                                          /**< Indicates whether to use multisampling */
    VkSampleCountFlagBits _msaaSampleCount{VK_SAMPLE_COUNT_1_BIT}; /**< Number of samples used in multisampling */
    VkImage _msaaImage{nullptr};                                   /**< Multisampling vulkan image */
    VkImageView _msaaImageView{nullptr};                           /**< Multisampling vulkan image view */
    VkDeviceMemory _msaaImageMemory{nullptr};                      /**< Multisampling vulkan image memory */

public:
    Scene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool useMsaa, bool measureTime);
    ~Scene();

    void run();

protected:
    void _initVulkan();
    void _createWindow();
    void _mainLoop();

    void _updateWindowDimensions(int width, int height);

    bool _areExtensionsSupported();
    bool _areValidationLayersSupported();
    void _createInstance();
    void _createSurface();
    bool _areDeviceExtensionsSupported(VkPhysicalDevice physicalDevice);
    QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice physicalDevice);
    int _ratePhysicalDevice(VkPhysicalDevice physicalDevice);
    void _selectPhysicalDevice();
    void _createLogicalDevice();

    SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR _selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableSurfaceFormats);
    VkPresentModeKHR _selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D _selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
    void _cleanupSwapChain();
    void _recreateSwapChain();
    void _createSwapChain();
    void _createImageViews();
    void _createRenderPass();

    void _createFramebuffers();
    void _createCommandPool();
    void _createCommandBuffers();
    void _recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void _createSynchronizationObjects();
    void _drawFrame();

    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);
    void _createImage(uint32_t width,
                      uint32_t height,
                      VkSampleCountFlagBits sampleCount,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkImage &image,
                      VkDeviceMemory &imageMemory);
    VkImageView _createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkFormat _selectSupportedFormat(const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features);
    VkFormat _selectDepthFormat();
    void _createDepthResources();

    void _selectMsaaSampleCount();
    void _createMsaaResources();
};
