#include "VulkanContext.h"

#include "VulkanUtils.h"
#include "vk_mem_alloc.h"

#include <Engine/Log.h>

#include <vector>

namespace VulkanContext {
VkInstance       sInstance{VK_NULL_HANDLE};
VkPhysicalDevice sPhysicalDevice{VK_NULL_HANDLE};
VkDevice         sDevice{VK_NULL_HANDLE};
uint32_t         sGraphicQueueFamilyIndex{0};
VkQueue          sGraphicsQueue{VK_NULL_HANDLE};
VmaAllocator     sVmaAllocator{VK_NULL_HANDLE};

bool Initialize() {
    const uint32_t desiredVulkanVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

    ENGINE_CORE_INFO("Initialisation vulkan ...");

    // If the vkGetInstanceProcAddr returns NULL for vkEnumerateInstanceVersion,
    // it is a Vulkan 1.0 implementation.
    if (!vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion")) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("Vulkan loader version found {}.{}.{}", 1, 0, 0);
        ENGINE_ERROR("Vulkan loader version required {}.{}.{}",
                     VK_VERSION_MAJOR(desiredVulkanVersion), VK_VERSION_MINOR(desiredVulkanVersion),
                     VK_VERSION_PATCH(desiredVulkanVersion));
        return false;
    }

    uint32_t vulkanLoaderVersion{};
    if (vkEnumerateInstanceVersion(&vulkanLoaderVersion) != VK_SUCCESS) {
        ENGINE_ERROR("Failed to get vulkan loader verson.");
        return false;
    }

    if (vulkanLoaderVersion < desiredVulkanVersion) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("Vulkan loader version found {}.{}.{}", VK_VERSION_MAJOR(vulkanLoaderVersion),
                     VK_VERSION_MINOR(vulkanLoaderVersion), VK_VERSION_PATCH(vulkanLoaderVersion));
        ENGINE_ERROR("Vulkan loader version required {}.{}.{}",
                     VK_VERSION_MAJOR(desiredVulkanVersion), VK_VERSION_MINOR(desiredVulkanVersion),
                     VK_VERSION_PATCH(desiredVulkanVersion));
        return false;
    }

    // =================================================
    //  Create vulkan instance
    // =================================================
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName   = "Engine";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName        = "Engine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.apiVersion         = desiredVulkanVersion;

    std::vector<const char*> instanceExtension;
    instanceExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtension.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    instanceExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    instanceExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    VkInstanceCreateInfo instanceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = (uint32_t)instanceExtension.size(),
        .ppEnabledExtensionNames = instanceExtension.data()};

    if (VK_SUCCESS != vkCreateInstance(&instanceCreateInfo, nullptr, &sInstance)) {
        ENGINE_ERROR("Failed to create vulkan instance.");
    }

    // ====================================================
    //  Setup debug callback
    // ====================================================

    /// TODO

    // ====================================================
    //   Find Physical Device
    // ====================================================
    auto physicalDevice = VulkanUtils::getPhysicalDevices(sInstance);
    if (physicalDevice.empty()) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("No GPU which support vulkan found.");
        return false;
    }

    // FIXME: If there is more than one GPU, chose the best one
    sPhysicalDevice = physicalDevice[0];

    // ====================================================
    //   Create Device
    //
    // Fow now, only use 1 queue that support graphic and compute.
    // ====================================================

    sGraphicQueueFamilyIndex = VulkanUtils::findDeviceQueueFamilyIndex(
        sPhysicalDevice, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    float                   pQueuePriorities = 0.0;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext;
    queueCreateInfo.flags;
    queueCreateInfo.queueFamilyIndex = sGraphicQueueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &pQueuePriorities;

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.features.imageCubeArray     = true;
    deviceFeatures.features.geometryShader     = true;
    deviceFeatures.features.tessellationShader = true;
    deviceFeatures.features.multiDrawIndirect  = true;
    deviceFeatures.features.drawIndirectFirstInstance = true;

    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    deviceExtensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &deviceFeatures;
    createInfo.flags                   = 0;
    createInfo.queueCreateInfoCount    = 1;
    createInfo.pQueueCreateInfos       = &queueCreateInfo;
    createInfo.enabledLayerCount       = 0;       // deprecated and ignored.
    createInfo.ppEnabledLayerNames     = nullptr; // deprecated and ignored.
    createInfo.enabledExtensionCount   = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pEnabledFeatures        = nullptr;
    if (VK_SUCCESS != vkCreateDevice(sPhysicalDevice, &createInfo, nullptr, &sDevice)) {
        ENGINE_ERROR("Failed to create vulkan device.");
    }

    vkGetDeviceQueue(sDevice, sGraphicQueueFamilyIndex, 0, &sGraphicsQueue);

    // ==============================================================
    //   Setup memory allocation
    // ==============================================================
    VmaVulkanFunctions vmaVulkanFunctions{};
#ifdef RHI_WANT_VOLK
    /// TODO: Fill vmaVulkanFunctions table if using volk loader
#endif
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion       = desiredVulkanVersion;
    // allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    // allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;   //
    // Enables usage of VK_KHR_dedicated_allocation extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;           // Enables usage of VK_KHR_bind_memory2
    // extension. allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT; //
    // Enables usage of VK_EXT_memory_budget extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT; // Enables usage of
    // VK_AMD_device_coherent_memory extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;      // Enables usage of "buffer device
    // address" feature allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;        // Enables usage of
    // VK_EXT_memory_priority extension in the library.
    allocatorInfo.instance         = sInstance;
    allocatorInfo.physicalDevice   = sPhysicalDevice;
    allocatorInfo.device           = sDevice;
    allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
    if (VK_SUCCESS != vmaCreateAllocator(&allocatorInfo, &sVmaAllocator)) {
        ENGINE_ERROR("Failed to create vma allocator.");
        return false;
    }

    return true;
}

void Shutdown() {
    vmaDestroyAllocator(sVmaAllocator);
    vkDestroyDevice(sDevice, nullptr);
    vkDestroyInstance(sInstance, nullptr);
    ENGINE_CORE_INFO("Shutdown vulkan ...");
}

VkInstance getIntance() { return sInstance; }

VkPhysicalDevice getPhycalDevice() { return sPhysicalDevice; }

VkDevice getDevice() { return sDevice; }

bool isLayerSupported() { return true; }

bool isInstanceExtensionSupported() { return true; }

bool isDeviceExtensionSupported() { return true; }
} // namespace VulkanContext
