#include "glfw/platform_factory.hpp"
#include "win32/platform_factory.hpp"

#include "platform_manager.hpp"
#include "core_manager.hpp"

#include "screen.hpp"
#include "file.hpp"

#ifdef USE_OPENGL

#include "gl/shader.hpp"
#include "gl/buffer.hpp"
#include "gl/vertex_array.hpp"

#else

#include "vk/instance.hpp"
#include "vk/surface.hpp"

#endif

#include <iostream>
#include <set>
#include <algorithm>

enum Action
{
    Exit
};

typedef struct Vertex
{
    glm::vec2 pos;
    glm::vec3 col;
} Vertex;

std::vector<Vertex> vertices
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

std::vector<uint32_t> indices
{
    0, 1, 2
};

#ifdef USE_OPENGL

#else

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> deviceExtensions =
{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices_queue;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices_queue.presentFamily = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices_queue.graphicsFamily = i;
        }

        if (indices_queue.graphicsFamily.has_value() &&
            indices_queue.presentFamily.has_value())
        {
            break;
        }

        i++;
    }

    return indices_queue;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    int width  = Screen::width();
    int height = Screen::height();


    VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };

    actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

#endif

int main()
{
    CoreManager      core_manager;
    PlatformManager& platform_manager = PlatformManager::instance();

    #ifdef USE_OPENGL
    auto platform_factory = new glfw::PlatformFactory();
    #else
    auto platform_factory = new win32::PlatformFactory();
    #endif

    core_manager.init();
    platform_manager.init(platform_factory, { "Template", 800, 600 });

    gainput::InputManager input_manager;
    input_manager.SetDisplaySize(800, 600);

    const gainput::DeviceId keyboard_id = input_manager.CreateDevice<gainput::InputDeviceKeyboard>();

    gainput::InputMap input_map(input_manager);
    input_map.MapBool(Action::Exit, keyboard_id, gainput::KeyEscape);

    #ifdef USE_OPENGL

    auto vert_source = File::read<char>("../simple_vert.glsl");
    auto frag_source = File::read<char>("../simple_frag.glsl");

    gl::ShaderStage vert_stage;
    vert_stage.create(GL_VERTEX_SHADER);
    vert_stage.source(vert_source);

    gl::ShaderStage frag_stage;
    frag_stage.create(GL_FRAGMENT_SHADER);
    frag_stage.source(frag_source);

    gl::Shader shader;
    shader.create();
    shader.attach(vert_stage);
    shader.attach(frag_stage);
    shader.compile();

    shader.detach(vert_stage);
    shader.detach(frag_stage);

    vert_stage.release();
    frag_stage.release();

    const GLint vpos_location = 0;
    const GLint vcol_location = 1;
    const GLint mvp_location  = 0;

    gl::VertexArray vao;
    vao.create();
    vao.bind();

    gl::Buffer vbo {GL_ARRAY_BUFFER };
    vbo.create();
    vbo.bind();
    vbo.source(base::Buffer::make_data(vertices));

    gl::Buffer ibo { GL_ELEMENT_ARRAY_BUFFER };
    ibo.create();
    ibo.bind();
    ibo.source(base::Buffer::make_data(indices));

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));

    gl::Buffer uniform { GL_UNIFORM_BUFFER };
    uniform.create();
    uniform.bind(0);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    #else

    vk::Instance vk_instance;
    vk_instance.create();

    vk::Surface vk_surface;
    vk_surface.create(vk_instance, platform_manager.win32_handle());

    VkDevice device;

    VkPhysicalDevice physicalDevice;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vk_instance.handle(), &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vk_instance.handle(), &deviceCount, devices.data());

    for (const auto& pd : devices)
    {
        if (isDeviceSuitable(pd))
        {
            physicalDevice = pd;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    VkQueue graphicQueue;
    VkQueue presentQueue;

    float queuePriority = 1.0f;
    QueueFamilyIndices indices_queue = findQueueFamilies(physicalDevice, vk_surface.handle());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices_queue.graphicsFamily.value(), indices_queue.presentFamily.value() };

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo { };
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo { };
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());;
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.enabledLayerCount       = 0;
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device))
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices_queue.graphicsFamily.value(), 0, &graphicQueue);
    vkGetDeviceQueue(device, indices_queue.presentFamily.value(), 0, &presentQueue);

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, vk_surface.handle());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapcreateInfo{};
    swapcreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapcreateInfo.surface = vk_surface.handle();
    swapcreateInfo.minImageCount = imageCount;
    swapcreateInfo.imageFormat = surfaceFormat.format;
    swapcreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapcreateInfo.imageExtent = extent;
    swapcreateInfo.imageArrayLayers = 1;
    swapcreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {indices_queue.graphicsFamily.value(), indices_queue.presentFamily.value()};

    if (indices_queue.graphicsFamily != indices_queue.presentFamily)
    {
        swapcreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapcreateInfo.queueFamilyIndexCount = 2;
        swapcreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapcreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapcreateInfo.queueFamilyIndexCount = 0; // Optional
        swapcreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    swapcreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapcreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapcreateInfo.presentMode = presentMode;
    swapcreateInfo.clipped = VK_TRUE;
    swapcreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;

    if (vkCreateSwapchainKHR(device, &swapcreateInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    std::vector<VkImageView> swapChainImageViews;
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    #endif

    auto angle = 0.0f;
    auto speed = 60.0f;

    while (platform_manager.is_active())
    {
        core_manager.update();
        input_manager.Update();

        if (input_map.GetBoolWasDown(Action::Exit))
        {
            platform_manager.shutdown();
        }

        auto width  = Screen::width();
        auto height = Screen::height();
        auto ratio  = Screen::ratio();

        angle += speed * Time::delta_time();

        glm::vec3 a   = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::mat4 m   = glm::rotate(glm::mat4(1.0f), glm::radians(angle), a);
        glm::mat4 p   = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
        glm::mat4 mvp = p * m;

        #ifdef USE_OPENGL

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        uniform.bind();
        uniform.source(base::Buffer::make_data(&mvp));

        shader.bind();
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);

        vao.bind();
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        #else



        #endif

        platform_manager.update();
    }

    #ifdef USE_OPENGL

    #else

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);

    vk_surface.release(vk_instance);
    vk_instance.release();

    #endif

    core_manager.release();
    platform_manager.release();

    exit(EXIT_SUCCESS);
}