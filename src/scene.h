/**
 * @file scene.h
 * @author Christian Saloň
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <memory>
#include <cstring>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>

#include <VFONT/vulkan_text_renderer.h>
#include <VFONT/text_renderer.h>
#include <VFONT/font.h>
#include <VFONT/text_block.h>
#include <VFONT/vulkan_triangulation_text_renderer.h>
#include <VFONT/vulkan_tessellation_shaders_text_renderer.h>
#include <VFONT/vulkan_winding_number_text_renderer.h>
#include <VFONT/i_vulkan_text_renderer.h>
#include <VFONT/vulkan_timed_renderer.h>

#include "window.h"
#include "base_camera.h"
#include "perspective_camera.h"
#include "ortho_camera.h"

/**
 * @class Scene
 *
 * @brief Enables 3D rendering with Vulkan
 */
class Scene {

protected:

    /**
     * @brief Represents supported vulkan queue families of physical device
     */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return
                graphicsFamily.has_value() &&
                presentFamily.has_value();
        }
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

    std::vector<const char *> extensions;                   /**< Selected vulkan extensions */
    std::vector<const char *> deviceExtensions;             /**< Selected vulkan device extensions */
    std::vector<const char *> validationLayers;             /**< Selected vulkan validation layers */

    bool _measureTime;
    std::shared_ptr<vft::IVulkanTextRenderer> _renderer;                            /**< Default text renderer */
    std::shared_ptr<MainWindow> _window;                    /**< Application window */
    std::unique_ptr<BaseCamera> _camera;                    /**< Camera object */
    CameraType _cameraType;                                 /**< Type of camera used for rendering */

    VkInstance _instance;                                   /**< Vulkan instance */
    VkSurfaceKHR _surface;                                  /**< Vulkan surface */
    VkPhysicalDevice _physicalDevice;                       /**< Vulkan physical device */
    VkDevice _logicalDevice;                                /**< Vulkan logical device */
    VkQueue _graphicsQueue;                                 /**< Vulkan graphics queue */
    VkQueue _presentQueue;                                  /**< Vulkan present queue */
    VkSwapchainKHR _swapChain;                              /**< Vulkan swap chain */
    std::vector<VkImage> _swapChainImages;                  /**< Vulkan swap chain images */
    VkFormat _swapChainImageFormat;                         /**< Vulkan swap chain image format */
    VkExtent2D _swapChainExtent;                            /**< Vulkan swap chain extent */
    std::vector<VkImageView> _swapChainImageViews;          /**< Vulkan swap chain image views */
    VkRenderPass _renderPass;                               /**< Vulkan render pass */
    std::vector<VkFramebuffer> _framebuffers;               /**< Vulkan frame buffers */
    VkCommandPool _commandPool;                             /**< Vulkan command pool */
    VkCommandBuffer _commandBuffer;           /**< Vulkan command buffers */
    VkSemaphore _imageAvailableSemaphore;     /**< Semaphore to signal that an image was acquired from the swap chain */
    VkSemaphore _renderFinishedSemaphore;     /**< Semaphore to signal if vulkan finished rendering and can begin presenting */
    VkFence _inFlightFence;                   /**< Fence to ensure only one frame at a time is being rendered */

public:

    Scene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool measureTime = false);
    ~Scene();

    void run();

    void updateWindowDimensions(int width, int height);
    void updateCameraPosition(float x, float y, float z);
    void updateCameraRotation(float x, float y);

protected:

    void _initVulkan();
    void _createWindow();
    void _mainLoop();

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
};
