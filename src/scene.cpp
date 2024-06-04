/**
 * @file scene.cpp
 * @author Christian Saloň
 */

#include "scene.h"

constexpr int Scene::MAX_FRAMES_IN_FLIGHT = 2;

/**
 * @brief Scene constructor
 * 
 * @param cameraType Type of camera used for rendering
 */
Scene::Scene(CameraType cameraType) {
    this->_cameraType = cameraType;
    this->_currentFrameIndex = 0;

    this->extensions = {
    "VK_KHR_surface",

#if defined(USE_WIN32)
        "VK_KHR_win32_surface"
#elif defined(USE_X11)
        "VK_KHR_xlib_surface"
#elif defined(USE_WAYLAND)
        "VK_KHR_wayland_surface"
#endif

    };

    this->deviceExtensions = {
        "VK_KHR_swapchain"
    };

    this->validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    this->_instance = nullptr;
    this->_surface = nullptr;
    this->_physicalDevice = nullptr;
    this->_logicalDevice = nullptr;
    this->_graphicsQueue = nullptr;
    this->_presentQueue = nullptr;
    this->_swapChain = nullptr;
    this->_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    this->_swapChainExtent = {};
    this->_renderPass = nullptr;
    this->_pipelineLayout = nullptr;
    this->_graphicsPipeline = nullptr;
    this->_commandPool = nullptr;
    this->_vertexBuffer = nullptr;

    // Initialize window
    this->_createWindow();

    // Initialize camera
    if(this->_cameraType == CameraType::ORTHOGRAPHIC) {
        this->_camera.reset(new OrthographicCamera{
            glm::vec3(0.f, 0.f, -1000.f),
            0.f, static_cast<float>(this->_window->getWidth()),
            0.f, static_cast<float>(this->_window->getHeight()),
            0.f, 2000.f
        });
    }
    else {
        this->_camera.reset(new PerspectiveCamera{
            glm::vec3(0.f, 0.f, -500.f),
            80.f,
            static_cast<float>(this->_window->getWidth()) / static_cast<float>(this->_window->getHeight()),
            0.f, 2000.f
        });
    }

    // Initialize vulkan
    this->_initVulkan();

    // Initialize text renderer
    this->_renderer.init(
        this->_physicalDevice,
        this->_logicalDevice,
        this->_commandPool,
        this->_graphicsQueue,
        this->_pipelineLayout
    );
}

/**
 * @brief Scene destructor
 */
Scene::~Scene() {
    this->_renderer.destroy();

    this->_cleanupSwapChain();

    for(int i = 0; i < Scene::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(this->_logicalDevice, this->_uniformBuffers.at(i), nullptr);
        vkFreeMemory(this->_logicalDevice, this->_uniformBuffersMemory.at(i), nullptr);
    }
    vkDestroyDescriptorPool(this->_logicalDevice, this->_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(this->_logicalDevice, this->_descriptorSetLayout, nullptr);

    for(int i = 0; i < Scene::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(this->_logicalDevice, this->_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(this->_logicalDevice, this->_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(this->_logicalDevice, this->_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(this->_logicalDevice, this->_commandPool, nullptr);

    vkDestroyPipeline(this->_logicalDevice, this->_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(this->_logicalDevice, this->_pipelineLayout, nullptr);
    vkDestroyRenderPass(this->_logicalDevice, this->_renderPass, nullptr);

    vkDestroyDevice(this->_logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
    vkDestroyInstance(this->_instance, nullptr);
}

/**
 * @brief Runs the main loop of scene
 */
void Scene::run() {
    this->_mainLoop();
}

/**
 * @brief This function is called when the window dimensions change
 * 
 * @param width New window width
 * @param height New window height
 */
void Scene::updateWindowDimensions(int width, int height) {
    if(this->_cameraType == CameraType::ORTHOGRAPHIC) {
        reinterpret_cast<OrthographicCamera *>(this->_camera.get())->setProjection(
            0.f, static_cast<float>(width),
            0.f, static_cast<float>(height),
            0.f, 2000.f
        );
    }
    else {
        reinterpret_cast<PerspectiveCamera *>(this->_camera.get())->setProjection(
            50.f,
            static_cast<float>(width) / static_cast<float>(height),
            0.f, 2000.f
        );
    }
}

/**
 * @brief Update the camera position based on window event
 * 
 * @param x Delta x
 * @param y Delta y
 */
void Scene::updateCameraRotation(float x, float y) {
    static const float rotateFactor = 0.4;

    glm::vec3 rotation;
    rotation.x = (y / this->_window->getHeight()) * 360.f;
    rotation.y = (x / this->_window->getWidth()) * 360.f;
    rotation.z = 0;
    rotation *= rotateFactor;

    this->_camera->rotate(rotation);
}

/**
 * @brief Update the camera rotation based on window event
 * 
 * @param x Delta x
 * @param y Delta y
 * @param z Delta z
 */
void Scene::updateCameraPosition(float x, float y, float z) {
    static const float translateFactor = 0.4;

    this->_camera->translate(glm::vec3(x, y, z) * translateFactor);
}

/**
 * @brief Initializes vulkan objects
 */
void Scene::_initVulkan() {
    this->_createInstance();
    this->_createSurface();
    this->_selectPhysicalDevice();
    this->_createLogicalDevice();
    this->_createSwapChain();
    this->_createImageViews();
    this->_createRenderPass();
    this->_createUniformBuffers();
    this->_createDescriptorSetLayout();
    this->_createDescriptorPool();
    this->_createDescriptorSets();
    this->_createGraphicsPipeline();
    this->_createFramebuffers();
    this->_createCommandPool();
    this->_createCommandBuffers();
    this->_createSynchronizationObjects();
}

/**
 * @brief Creates window with default values
 */
void Scene::_createWindow() {
    this->_window.reset(new MainWindow());

    this->_window->setResizeCallback(
        [this](int width, int height) -> void {
            this->updateWindowDimensions(width, height);
        });
    this->_window->setDragCallback(
        [this](float x, float y, bool shiftPressed) -> void {
            if(shiftPressed) {
                this->updateCameraPosition(x, y, 0.f);
            }
            else {
                this->updateCameraRotation(x, y);
            }
        });
    this->_window->setScrollCallback(
        [this](float z) -> void {
            this->updateCameraPosition(0.f, 0.f, z);
        });

    this->_window->create();
}

/**
 * @brief Shows window and runs main loop
 */
void Scene::_mainLoop() {
    this->_window->show();

    while(this->_window->isActive()) {
        this->_drawFrame();
        this->_window->pollEvents();
    }

    vkDeviceWaitIdle(this->_logicalDevice);
}

/**
 * @brief Checks if all required vulkan extensions are supported
 *
 * @return True if all extensions are supported, else false
 */
bool Scene::_areExtensionsSupported() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensionsProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensionsProperties.data());

    for(const char *extension : extensions) {
        bool extensionFound = false;

        for(const VkExtensionProperties &extensionProperties : availableExtensionsProperties) {
            if(strcmp(extension, extensionProperties.extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }

        if(!extensionFound) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Checks if all required vulkan validations layers are supported
 *
 * @return True if all validation layers are supported, else false
 */
bool Scene::_areValidationLayersSupported() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> supportedLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

    for(const char *validationLayer : validationLayers) {
        bool layerFound = false;

        for(const VkLayerProperties &layerProperties : supportedLayers) {
            if(strcmp(validationLayer, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if(!layerFound) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Creates vulkan instance
 */
void Scene::_createInstance() {
    if(!this->_areExtensionsSupported()) {
        throw std::runtime_error("Selected vulkan extensions are not supported");
    }

    if(!this->_areValidationLayersSupported()) {
        throw std::runtime_error("Selected vulkan validation layers are not supported");
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "vfont-demo";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if(vkCreateInstance(&instanceCreateInfo, nullptr, &this->_instance) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan instance");
    }
}

/**
 * @brief Creates vulkan surface
 */
void Scene::_createSurface() {

#if defined(USE_WIN32)

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hwnd = this->_window->getHwmd();
    surfaceCreateInfo.hinstance = this->_window->getHInstance();

    if(vkCreateWin32SurfaceKHR(this->_instance, &surfaceCreateInfo, nullptr, &this->_surface) != VK_SUCCESS) {
        throw std::runtime_error("Error creating WIN32 vulkan surface");
    }

#elif defined(USE_X11)

    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy = this->_window->getDisplay();
    surfaceCreateInfo.window = this->_window->getWindow();

    if(vkCreateXlibSurfaceKHR(this->_instance, &surfaceCreateInfo, nullptr, &this->_surface) != VK_SUCCESS) {
        throw std::runtime_error("Error creating X11 vulkan surface");
    }

#elif defined(USE_WAYLAND)

    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.display = this->_window->getDisplay();
    surfaceCreateInfo.surface = this->_window->getSurface();

    if(vkCreateWaylandSurfaceKHR(this->_instance, &surfaceCreateInfo, nullptr, &this->_surface) != VK_SUCCESS) {
        throw std::runtime_error("Error creating Wayland vulkan surface");
    }

#endif

}

/**
 * @brief Checks if specified physical device supports required vulkan device extensions
 *
 * @param physicalDevice Physical device (graphics card)
 *
 * @return True if all extensions are supported, else false
 */
bool Scene::_areDeviceExtensionsSupported(VkPhysicalDevice physicalDevice) {
    uint32_t deviceExtensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableDeviceExtensionsProperties(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableDeviceExtensionsProperties.data());

    for(const char *requiredDeviceExtension : deviceExtensions) {
        bool deviceExtensionFound = false;

        for(const VkExtensionProperties &availableDeviceExtensionProperties : availableDeviceExtensionsProperties) {
            if(strcmp(requiredDeviceExtension, availableDeviceExtensionProperties.extensionName) == 0) {
                deviceExtensionFound = true;
                break;
            }
        }

        if(!deviceExtensionFound) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Checks and registers vulkan graphics and present queue
 *
 * @param physicalDevice Physical device (graphics card)
 *
 * @return Populated QueueFamilyIndices struct
 */
Scene::QueueFamilyIndices Scene::_findQueueFamilies(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    QueueFamilyIndices indices;
    int i = 0;
    for(const VkQueueFamilyProperties &queueFamilyProperties : queueFamilies) {
        if(queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, this->_surface, &presentSupport);
        if(presentSupport) {
            indices.presentFamily = i;
        }

        if(indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

/**
 * @brief Rates the suitability of the specified physical device based on performance and capabilities
 *
 * @param physicalDevice Physical device (graphics card)
 *
 * @return Score of physical device
 */
int Scene::_ratePhysicalDevice(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    int score = 0;

    if(!this->_areDeviceExtensionsSupported(physicalDevice)) {
        return score;
    }

    SwapChainSupportDetails swapChainSupport = this->_querySwapChainSupport(physicalDevice);
    bool isSwapChainSupported = !swapChainSupport.surfaceFormats.empty() && !swapChainSupport.presentModes.empty();
    if(!isSwapChainSupported) {
        return score;
    }

    QueueFamilyIndices indices = this->_findQueueFamilies(physicalDevice);
    if(!indices.isComplete()) {
        return score;
    }

    if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }
    else if(physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 10;
    }

    return score;
}

/**
 * @brief Selects which physical device (graphics card) will be used for rendering
 */
void Scene::_selectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);

    if(deviceCount == 0) {
        throw std::runtime_error("No GPU found which supports vulkan");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(this->_instance, &deviceCount, physicalDevices.data());

    std::multimap<int, VkPhysicalDevice> ratedPhysicalDevices;
    for(const VkPhysicalDevice &physicalDevice : physicalDevices) {
        ratedPhysicalDevices.insert(
            std::make_pair(this->_ratePhysicalDevice(physicalDevice), physicalDevice)
        );
    }

    if(ratedPhysicalDevices.rbegin()->first > 0) {
        this->_physicalDevice = ratedPhysicalDevices.rbegin()->second;
    }
    else {
        throw std::runtime_error("Error selecting GPU");
    }
}

/**
 * @brief Creates vulkan logical device based on selected physical device
 */
void Scene::_createLogicalDevice() {
    QueueFamilyIndices indices = this->_findQueueFamilies(this->_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for(uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = queueFamily;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();

    if(vkCreateDevice(this->_physicalDevice, &deviceCreateInfo, nullptr, &this->_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan logical device");
    }

    vkGetDeviceQueue(this->_logicalDevice, indices.graphicsFamily.value(), 0, &this->_graphicsQueue);
    vkGetDeviceQueue(this->_logicalDevice, indices.presentFamily.value(), 0, &this->_presentQueue);
}

/**
 * @brief Queries the swap chain capabilities of selected physical device
 *
 * @param physicalDevice Physical device (graphics card)
 *
 * @return Populated SwapChainSupportDetails struct
 */
Scene::SwapChainSupportDetails Scene::_querySwapChainSupport(VkPhysicalDevice physicalDevice) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, this->_surface, &details.surfaceCapabilities);

    // Select composite alpha mode
    if(details.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        details.compositeAlphaMode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if(details.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        details.compositeAlphaMode = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else if(details.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
        details.compositeAlphaMode = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }
    else if(details.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
        details.compositeAlphaMode = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }
    else {
        throw std::runtime_error("No supported composite alpha mode for vulkan");
    }

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, this->_surface, &surfaceFormatCount, nullptr);
    if(surfaceFormatCount != 0) {
        details.surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, this->_surface, &surfaceFormatCount, details.surfaceFormats.data());
    }

    uint32_t surfacePresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, this->_surface, &surfacePresentModeCount, nullptr);
    if(surfacePresentModeCount != 0) {
        details.presentModes.resize(surfacePresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, this->_surface, &surfacePresentModeCount, details.presentModes.data());
    }

    return details;
}

/**
 * @brief Selects the preferred swap chain surface format
 *
 * @param availableSurfaceFormats Vulkan surface formats of selected physical device
 *
 * @return Selected vulkan swap chain surface format
 */
VkSurfaceFormatKHR Scene::_selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableSurfaceFormats) {
    for(const VkSurfaceFormatKHR &availableSurfaceFormat : availableSurfaceFormats) {
        if(availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableSurfaceFormat;
        }
    }

    return availableSurfaceFormats.at(0);
}

/**
 * @brief Selects the preferred presenting mode
 *
 * @param availablePresentModes Supported vulkan present modes of selected physical device
 *
 * @return Selected vulkan present mode
 */
VkPresentModeKHR Scene::_selectSwapChainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for(const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

/**
 * @brief Creates the optimal vulkan swap chain extent
 *
 * @param surfaceCapabilities Supported surface capabilities of selected physical device
 *
 * @return Vulkan 2D extent
 */
VkExtent2D Scene::_selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
    if(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surfaceCapabilities.currentExtent;
    }

    VkExtent2D extent = {
        static_cast<uint32_t>(this->_window->getWidth()),
        static_cast<uint32_t>(this->_window->getHeight())
    };

    extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

    return extent;
}

/**
 * @brief Destroys vulkan swap chain, image views and frame buffers
 */
void Scene::_cleanupSwapChain() {
    for(VkFramebuffer swapChainFramebuffer : this->_framebuffers) {
        vkDestroyFramebuffer(this->_logicalDevice, swapChainFramebuffer, nullptr);
    }

    for(const VkImageView &swapChainImageView : this->_swapChainImageViews) {
        vkDestroyImageView(this->_logicalDevice, swapChainImageView, nullptr);
    }

    vkDestroySwapchainKHR(this->_logicalDevice, this->_swapChain, nullptr);
}

/**
 * @brief Creates a new swapchain when needed and destroys the previous one
 */
void Scene::_recreateSwapChain() {
    vkDeviceWaitIdle(this->_logicalDevice);
    this->_cleanupSwapChain();

    this->_createSwapChain();
    this->_createImageViews();
    this->_createFramebuffers();
}

/**
 * @brief Creates vulkan swap chain
 */
void Scene::_createSwapChain() {
    SwapChainSupportDetails swapChainSupport = this->_querySwapChainSupport(this->_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = this->_selectSwapChainSurfaceFormat(swapChainSupport.surfaceFormats);
    VkPresentModeKHR presentMode = this->_selectSwapChainPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = this->_selectSwapChainExtent(swapChainSupport.surfaceCapabilities);

    uint32_t imageCount = swapChainSupport.surfaceCapabilities.minImageCount + 1;
    if(
        swapChainSupport.surfaceCapabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.surfaceCapabilities.maxImageCount
    ) {
        imageCount = swapChainSupport.surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = this->_surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = this->_findQueueFamilies(this->_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if(indices.graphicsFamily != indices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = swapChainSupport.surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = swapChainSupport.compositeAlphaMode;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    if(vkCreateSwapchainKHR(this->_logicalDevice, &swapChainCreateInfo, nullptr, &this->_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan swap chain");
    }

    this->_swapChainImageFormat = surfaceFormat.format;
    this->_swapChainExtent = extent;

    vkGetSwapchainImagesKHR(this->_logicalDevice, this->_swapChain, &imageCount, nullptr);
    this->_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->_logicalDevice, this->_swapChain, &imageCount, this->_swapChainImages.data());
}

/**
 * @brief Creates vulkan image views
 */
void Scene::_createImageViews() {
    this->_swapChainImageViews.resize(this->_swapChainImages.size());

    int i = 0;
    for(const VkImage &swapChainImage : this->_swapChainImages) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = this->_swapChainImageFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(this->_logicalDevice, &imageViewCreateInfo, nullptr, &this->_swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error creating vulkan image views");
        }

        i++;
    }
}

/**
 * @brief Creates vulkan render pass
 */
void Scene::_createRenderPass() {
    VkAttachmentDescription colorAttachmentDescription{};
    colorAttachmentDescription.format = this->_swapChainImageFormat;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if(vkCreateRenderPass(this->_logicalDevice, &renderPassCreateInfo, nullptr, &this->_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan render pass");
    }
}

/**
 * @brief Used for reading shader code
 *
 * @param fileName Shader file name
 *
 * @return Shader code buffer
 */
std::vector<char> Scene::_readFile(std::string fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if(!file.is_open()) {
        throw std::runtime_error("Error opening file " + fileName);
    }

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

/**
 * @brief Creates vulkan shader module from specified shader
 *
 * @param shaderCode Shader code
 *
 * @return Vulkan shader module
 */
VkShaderModule Scene::_createShaderModule(const std::vector<char> &shaderCode) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    VkShaderModule shaderModule;

    if(vkCreateShaderModule(this->_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan shader module");
    }

    return shaderModule;
}

/**
 * @brief Creates vulkan descriptor set layout
 */
void Scene::_createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.binding = 0;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if(vkCreateDescriptorSetLayout(this->_logicalDevice, &layoutCreateInfo, nullptr, &(this->_descriptorSetLayout)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor set layout");
    }

}

/**
 * @brief Selects the best possible memory to create vulkan buffer
 *
 * @param memoryType Required memory type
 * @param properties Required memory properties
 *
 * @return Index of selected memory
 */
uint32_t Scene::_selectMemoryType(uint32_t memoryType, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);

    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if((memoryType & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Error selecting memory for vulkan buffer");
}

/**
 * @brief Creates and allocates memory for vulkan buffer
 *
 * @param size Size of buffer
 * @param usage Usage of buffer
 * @param properties Memory properties
 * @param buffer Vulkan buffer
 * @param bufferMemory Vulkan buffer memory
 */
void Scene::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(this->_logicalDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan buffer");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->_logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = this->_selectMemoryType(memoryRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(this->_logicalDevice, &memoryAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan buffer memory");
    }

    vkBindBufferMemory(this->_logicalDevice, buffer, bufferMemory, 0);
}

/**
 * @brief Creates vulkan uniform buffers
 */
void Scene::_createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    this->_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    this->_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    this->_mappedUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        this->_createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            this->_uniformBuffers.at(i),
            this->_uniformBuffersMemory.at(i)
        );

        vkMapMemory(this->_logicalDevice, this->_uniformBuffersMemory.at(i), 0, bufferSize, 0, &(this->_mappedUniformBuffers.at(i)));
    }
}

/**
 * @brief Creates vulkan descriptor pool
 */
void Scene::_createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(Scene::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(Scene::MAX_FRAMES_IN_FLIGHT);

    if(vkCreateDescriptorPool(this->_logicalDevice, &poolCreateInfo, nullptr, &(this->_descriptorPool)) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan descriptor pool");
    }
}

/**
 * @brief Creates vulkan descriptor sets
 */
void Scene::_createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(Scene::MAX_FRAMES_IN_FLIGHT, this->_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = this->_descriptorPool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(Scene::MAX_FRAMES_IN_FLIGHT);
    allocateInfo.pSetLayouts = layouts.data();

    this->_descriptorSets.resize(Scene::MAX_FRAMES_IN_FLIGHT);

    if(vkAllocateDescriptorSets(this->_logicalDevice, &allocateInfo, this->_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error allocating vulkan descriptor sets");
    }

    for(int i = 0; i < Scene::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = this->_uniformBuffers.at(i);
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = this->_descriptorSets.at(i);
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        vkUpdateDescriptorSets(this->_logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
    }
}

/**
 * @brief Updates uniform buffer currently used for rendering frame
 */
void Scene::_setUniformBuffers() {
    UniformBufferObject ubo{};
    ubo.view = this->_camera->getViewMatrix();
    ubo.projection = this->_camera->getProjectionMatrix();

    memcpy(this->_mappedUniformBuffers.at(this->_currentFrameIndex), &ubo, sizeof(ubo));
}

/**
 * @brief Creates vulkan graphics pipeline
 * Color blending is enabled
 */
void Scene::_createGraphicsPipeline() {
    std::vector<char> vertexShaderCode = this->_readFile("shaders/vert.spv");
    std::vector<char> fragmentShaderCode = this->_readFile("shaders/frag.spv");

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
    pushConstantRange.size = sizeof(vft::CharacterPushConstants);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &this->_descriptorSetLayout;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    if(vkCreatePipelineLayout(this->_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &this->_pipelineLayout) != VK_SUCCESS) {
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
    graphicsPipelineCreateInfo.layout = this->_pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = this->_renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    if(vkCreateGraphicsPipelines(this->_logicalDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &this->_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan graphics pipeline");
    }

    vkDestroyShaderModule(this->_logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->_logicalDevice, fragmentShaderModule, nullptr);
}

/**
 * @brief Creates vulkan frame buffers
 */
void Scene::_createFramebuffers() {
    this->_framebuffers.resize(this->_swapChainImageViews.size());

    for(size_t i = 0; i < this->_framebuffers.size(); i++) {
        VkImageView attachments[] = { this->_swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = this->_renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = this->_swapChainExtent.width;
        framebufferCreateInfo.height = this->_swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        if(vkCreateFramebuffer(this->_logicalDevice, &framebufferCreateInfo, nullptr, &this->_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error creating vulkan frame buffer");
        }
    }
}

/**
 * @brief Creates vulkan command pool
 */
void Scene::_createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = this->_findQueueFamilies(this->_physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(this->_logicalDevice, &commandPoolCreateInfo, nullptr, &this->_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan command pool");
    }
}

/**
 * @brief Creates vulkan command buffers
 */
void Scene::_createCommandBuffers() {
    this->_commandBuffers.resize(Scene::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = this->_commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(this->_commandBuffers.size());

    if(vkAllocateCommandBuffers(this->_logicalDevice, &commandBufferAllocateInfo, this->_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error creating vulkan command buffer");
    }
}

/**
 * @brief Populates the command buffer with commands
 *
 * @param commandBuffer Command buffer which is being populated with commands
 * @param imageIndex Specifies which frame buffer should be used
 */
void Scene::_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Error while recording vulkan command buffer");
    }

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = this->_renderPass;
    renderPassBeginInfo.framebuffer = this->_framebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = this->_swapChainExtent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->_swapChainExtent.width);
    viewport.height = static_cast<float>(this->_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = this->_swapChainExtent;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout, 0, 1, &(this->_descriptorSets.at(this->_currentFrameIndex)), 0, nullptr);
    this->_renderer.draw(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Error recording vulkan command buffer");
    }
}

/**
 * @brief Creates vulkan synchronization objects
 */
void Scene::_createSynchronizationObjects() {
    this->_imageAvailableSemaphores.resize(Scene::MAX_FRAMES_IN_FLIGHT);
    this->_renderFinishedSemaphores.resize(Scene::MAX_FRAMES_IN_FLIGHT);
    this->_inFlightFences.resize(Scene::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i = 0; i < Scene::MAX_FRAMES_IN_FLIGHT; i++) {
        if(
            vkCreateSemaphore(this->_logicalDevice, &semaphoreCreateInfo, nullptr, &this->_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(this->_logicalDevice, &semaphoreCreateInfo, nullptr, &this->_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(this->_logicalDevice, &fenceCreateInfo, nullptr, &this->_inFlightFences[i]) != VK_SUCCESS
        ) {
            throw std::runtime_error("Error creating vulkan synchronization objects");
        }
    }
}

/**
 * @brief Records and executes the command buffer, presents output to screen
 */
void Scene::_drawFrame() {
    vkWaitForFences(this->_logicalDevice, 1, &this->_inFlightFences[this->_currentFrameIndex], true, UINT64_MAX);

    this->_setUniformBuffers();

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(this->_logicalDevice, this->_swapChain, UINT64_MAX, this->_imageAvailableSemaphores[this->_currentFrameIndex], nullptr, &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        _recreateSwapChain();
        return;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Error acquiring vulkan swap chain image");
    }

    vkResetFences(this->_logicalDevice, 1, &this->_inFlightFences[this->_currentFrameIndex]);

    vkResetCommandBuffer(this->_commandBuffers[this->_currentFrameIndex], 0);
    this->_recordCommandBuffer(this->_commandBuffers[this->_currentFrameIndex], imageIndex);

    VkSemaphore signalSemaphores[] = { this->_renderFinishedSemaphores[this->_currentFrameIndex] };
    VkSemaphore waitSemaphores[] = { this->_imageAvailableSemaphores[this->_currentFrameIndex] };
    VkPipelineStageFlags waitStagees[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStagees;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->_commandBuffers[this->_currentFrameIndex];

    if(vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, this->_inFlightFences[this->_currentFrameIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Error submiting draw command buffer");
    }

    VkSwapchainKHR swapChains[] = { this->_swapChain };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(this->_presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->_window->wasResized()) {
        this->_window->resetResized();
        this->_recreateSwapChain();
        return;
    }
    else if(result != VK_SUCCESS) {
        throw std::runtime_error("Error presenting vulkan swap chain image");
    }

    this->_currentFrameIndex = (this->_currentFrameIndex + 1) % Scene::MAX_FRAMES_IN_FLIGHT;
}
