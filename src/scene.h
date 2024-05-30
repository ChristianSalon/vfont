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

#include <VFONT/text_renderer.h>
#include <VFONT/text_renderer_utils.h>
#include <VFONT/font.h>
#include <VFONT/text_block.h>
#include <VFONT/text_renderer.h>

#include "window.h"
#include "base_camera.h"
#include "perspective_camera.h"
#include "orto_camera.h"

/**
 * @class Scene
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

    /**
     * @brief Uniform buffer object
     */
    struct UniformBufferObject {
        glm::mat4 view;
        glm::mat4 projection;
    };

    static const int MAX_FRAMES_IN_FLIGHT;                  /**< Number of frames being concurrently processed */
    std::vector<const char *> extensions;                   /**< Selected vulkan extensions */
    std::vector<const char *> deviceExtensions;             /**< Selected vulkan device extensions */
    std::vector<const char *> validationLayers;             /**< Selected vulkan validation layers */

    vft::TextRenderer renderer;
    std::shared_ptr<MainWindow> _window;                    /**< Application window */
    std::unique_ptr<BaseCamera> _camera;                    /**< Camera object */
    CameraType _cameraType;                                 /**< Type of camera used for rendering */

    uint32_t _currentFrameIndex;                            /**< Current frame used for rendering */

    std::vector<VkBuffer> _uniformBuffers;                  /**< Uniform buffers */
    std::vector<VkDeviceMemory> _uniformBuffersMemory;      /**< Uniform buffers memory */
    std::vector<void *> _mappedUniformBuffers;              /**< Uniform buffers pointers */

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
    VkDescriptorSetLayout _descriptorSetLayout;             /**< Vulkan descriptor set layout */
    VkDescriptorPool _descriptorPool;                       /**< Vulkan descriptor pool */
    std::vector<VkDescriptorSet> _descriptorSets;           /**< Vulkan descriptor sets */
    VkPipelineLayout _pipelineLayout;                       /**< Vulkan pipeline layout */
    VkPipeline _graphicsPipeline;                           /**< Vulkan graphics pipeline */
    std::vector<VkFramebuffer> _framebuffers;               /**< Vulkan frame buffers */
    VkCommandPool _commandPool;                             /**< Vulkan command pool */
    VkBuffer _vertexBuffer;                                 /**< Vulkan vertex buffer */
    VkDeviceMemory _vertexBufferMemory;                     /**< Vulkan vertex buffer memory */
    std::vector<VkCommandBuffer> _commandBuffers;           /**< Vulkan command buffers */
    std::vector<VkSemaphore> _imageAvailableSemaphores;     /**< Semaphore to signal that an image was acquired from the swap chain */
    std::vector<VkSemaphore> _renderFinishedSemaphores;     /**< Semaphore to signal if vulkan finished rendering and can begin presenting */
    std::vector<VkFence> _inFlightFences;                   /**< Fence to ensure only one frame at a time is being rendered */

public:

    Scene(CameraType cameraType);
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
    std::vector<char> _readFile(std::string fileName);
    VkShaderModule _createShaderModule(const std::vector<char> &shaderCode);
    void _createDescriptorSetLayout();
    uint32_t _selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties);

    void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void _createUniformBuffers();
    void _createDescriptorPool();
    void _createDescriptorSets();
    void _setUniformBuffers();

    void _createGraphicsPipeline();
    void _createFramebuffers();
    void _createCommandPool();
    void _createCommandBuffers();
    void _recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void _createSynchronizationObjects();
    void _drawFrame();
};
