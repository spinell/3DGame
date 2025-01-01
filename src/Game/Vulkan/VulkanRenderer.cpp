#include "VulkanRenderer.h"
#include "VulkanUtils.h"

#include <vulkan/vk_enum_string_helper.h>

import std;

#include <volk.h>

namespace {
    VkInstance               instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    // TODO: check layers are available (VK_LAYER_PATH) ....
    const std::vector<const char*> requiredLayers = {
        //"VK_LAYER_KHRONOS_profiles",
        //"VK_LAYER_LUNARG_screenshot",
        "VK_LAYER_LUNARG_monitor", "VK_LAYER_KHRONOS_validation",
        //"VK_LAYER_KHRONOS_synchronization2",
        //"VK_LAYER_LUNARG_gfxreconstruct",
        //"VK_LAYER_LUNARG_api_dump",
        //"VK_LAYER_RENDERDOC_Capture",
        //"VK_LAYER_VALVE_steam_fossilize",
        //"VK_LAYER_VALVE_steam_overlay",
        //"VK_LAYER_NV_optimus"
    };

    const std::vector<const char*> requiredInstanceExtensions = {
        // used for platform independent surface manipulation
        //  - vkDestroySurfaceKHR
        //  - vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        //  - vkGetPhysicalDeviceSurfaceFormatsKHR
        //  - vkGetPhysicalDeviceSurfacePresentModesKHR
        //  - vkGetPhysicalDeviceSurfaceSupportKHR
        VK_KHR_SURFACE_EXTENSION_NAME,
        // Provides new queries for device surface capabilities that can be easily extended by other
        // extensions
        // - vkGetPhysicalDeviceSurfaceCapabilities2KHR
        // - vkGetPhysicalDeviceSurfaceFormats2KHR
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        //
        // Adds a collection of window system integration features that were intentionally
        // left out or overlooked in the original VK_KHR_surface extension.
        //  - Allow querying number of min/max images from a surface for a particular presentation
        //    mode.
        //  - Allow querying a surface�s scaled presentation capabilities.
        //  - Allow querying a surface for the set of presentation modes which can be easily
        //    switched between without requiring swapchain recreation.
        //  Dependencies: VK_KHR_surface and VK_KHR_get_surface_capabilities2
        VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
        //
        // expands VkColorSpaceKHR to add support for most standard color spaces beyond
        // VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        // This extension also adds support for VK_COLOR_SPACE_PASS_THROUGH_EXT which allows
        // applications to use color
        // spaces not explicitly enumerated in VkColorSpaceKHR.
        // VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME, // TODO: debug only
#ifdef WIN32
        // TODO: windows only
        //  - vkCreateWin32SurfaceKHR
        //  - vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        // - vkCreateWaylandSurfaceKHR
        // - vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex,
        // display)
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
        // - vkCreateXcbSurfaceKHR
        // - vkGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex,
        // connection, visual_id)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
        // - vkCreateXlibSurfaceKHR
        // - vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy,
        // visualID)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        // - vkCreateDirectFBSurfaceEXT
        // - vkGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex,
        // dfb)
        VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME
#endif
    };

    VkBool32 VKAPI_PTR
    instanceDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                        VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                        void*                                       pUserData) {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            spdlog::error("{} - {}: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::warn("{} - {}: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            spdlog::info("{} - {}: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            spdlog::trace("{} - {}: {}", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
        }
        return true;
    }
};


bool VulkanRenderer::initialize() {
    spdlog::info("Init Valkan ....");

    // Initialise volk.
    // This must be done before calling any Vulkan functions because we are not
    // linking staticly with the Vulkan loader (vulkan-1.dll)
    // This will dynamicly load (vulkan-1.dll / libvulkan.so.1)and Vulkan
    // extension.
    if (volkInitialize() != VK_SUCCESS) {
        spdlog::critical("Failed to load vulkan.");
        return false;
    }

    if (auto vkLoaderVersion = volkGetInstanceVersion(); vkLoaderVersion == 0) {
        spdlog::critical("volkGetInstanceVersion failed.");
        return false;
    }
    else {
        spdlog::info("Using vulkan instance version {}.{}.{} (Variant {})", VK_API_VERSION_MAJOR(vkLoaderVersion), VK_API_VERSION_MINOR(vkLoaderVersion), VK_API_VERSION_PATCH(vkLoaderVersion), VK_API_VERSION_VARIANT(vkLoaderVersion));
    }

    // get all available instance layers.
    const auto availableInstanceLayers = VulkanUtils::getInstanceLayers();
    spdlog::info("Found {} instance layer(s).", availableInstanceLayers.size());
    for (const auto& layer : availableInstanceLayers) {
        constexpr std::string_view nameFmt       = " - {}";
        constexpr std::string_view descFmt       = "   Description : {}";
        constexpr std::string_view specVerFmt    = "   SpecVersion : {}.{}.{}.{}";
        constexpr std::string_view implVerFmt    = "   Implementation Version: {}";
        constexpr std::string_view nbExtFmt      = "   Provide {} extensions";
        constexpr std::string_view extFmt        = "    - {}";

        spdlog::info(nameFmt, layer.layerName);
        spdlog::info(descFmt, layer.description);
        spdlog::info(specVerFmt, VK_API_VERSION_VARIANT(layer.specVersion), VK_API_VERSION_MAJOR(layer.specVersion), VK_API_VERSION_MINOR(layer.specVersion), VK_API_VERSION_PATCH(layer.specVersion));
        spdlog::info(implVerFmt, layer.implementationVersion);

        const auto& layerExtensions = VulkanUtils::getInstanceExtensions(layer.layerName);
        spdlog::info(nbExtFmt, layerExtensions.size());
        for (const auto& extension : layerExtensions) {
            spdlog::info(extFmt, extension);
        }
    }

    // get all available instance extensions.
    const auto availableInstanceExtensions = VulkanUtils::getInstanceExtensions();
    spdlog::info("Found {} instance extension(s).", availableInstanceExtensions.size());
    for (const auto& extension : availableInstanceExtensions) {
        spdlog::info(" - {}", extension);
    }

    // be sure our required extension is supported.
    for (auto& name : requiredInstanceExtensions) {
        if (std::find_if(availableInstanceExtensions.begin(), availableInstanceExtensions.end(),
                         [name](VkExtensionProperties ext) {
                             return std::strcmp(ext.extensionName, name) == 0;
                         }) == availableInstanceExtensions.end()) {
            spdlog::critical("Can't initialize vulkan, Instance extension {} is not supported.",
                             name);
            return false;
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMsgCreateInfo{};
    debugMsgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMsgCreateInfo.pNext = nullptr;
    debugMsgCreateInfo.flags = 0;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugMsgCreateInfo.pfnUserCallback = instanceDebugUtilsMessengerCallback;
    debugMsgCreateInfo.pUserData       = nullptr;

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext              = nullptr;
    applicationInfo.pApplicationName   = "SpyApp";                        // FIXME: Config by app
    applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // FIXME: Config by app
    applicationInfo.pEngineName        = "SpyEngine";
    applicationInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
    applicationInfo.apiVersion         = VK_MAKE_API_VERSION(0,1,3,0); // FIXME: Config by app ???

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext                   = &debugMsgCreateInfo;
    instanceCreateInfo.pApplicationInfo        = &applicationInfo;
    instanceCreateInfo.enabledLayerCount       = requiredLayers.size();
    instanceCreateInfo.ppEnabledLayerNames     = requiredLayers.data();
    instanceCreateInfo.enabledExtensionCount   = requiredInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

    auto result = vkCreateInstance(&instanceCreateInfo, nullptr /*allocator*/, &instance);
    if (result != VK_SUCCESS) {
        spdlog::critical("vkCreateInstance() failed : {}", string_VkResult(result));
        return false;
    }

    // load all vulkan functions
    volkLoadInstance(instance);

    const auto physicalDevices = VulkanUtils::getPhysicalDevices(instance);
    if (physicalDevices.empty()) {
        spdlog::critical("Can't find a GPU.", physicalDevices.size());
        // todo destroy instance
        return false;
    }

    spdlog::info("Found {} physical device.", physicalDevices.size());
    for (auto& device : physicalDevices) {
        VkPhysicalDeviceDriverProperties driverProperties{}; // Provided by VK_VERSION_1_2
        driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

        VkPhysicalDeviceVulkan11Properties properties11{}; // Vulkan 1.1 functionality.
        properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        properties11.pNext = &driverProperties;

        VkPhysicalDeviceVulkan12Properties properties12{}; // Vulkan 1.2 functionality.
        properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        properties12.pNext = &properties11;

        VkPhysicalDeviceVulkan13Properties properties13{}; // Vulkan 1.3 functionality.
        properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        properties13.pNext = &properties12;

        VkPhysicalDeviceProperties2 physicalDeviceProperties{};
        physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProperties.pNext = &properties13;

        VkPhysicalDeviceProperties& properties = physicalDeviceProperties.properties;

        vkGetPhysicalDeviceProperties2(device, &physicalDeviceProperties);
        spdlog::info("\t {}", properties.deviceName);
        spdlog::info("\t\tApiVersion: {}.{}.{}.{}", VK_API_VERSION_VARIANT(properties.apiVersion),
                     VK_API_VERSION_MAJOR(properties.apiVersion),
                     VK_API_VERSION_MINOR(properties.apiVersion),
                     VK_API_VERSION_PATCH(properties.apiVersion));
        spdlog::info(
            "\t\tDriverVersion: {}.{}.{}.{}", // FIXME: driver version
            ((properties.driverVersion >> 22) & 0x3ff), ((properties.driverVersion >> 14) & 0x0ff),
            ((properties.driverVersion >> 6) & 0x0ff), ((properties.driverVersion) & 0x003f));
        spdlog::info("\t\tVendorID: {}", properties.vendorID);
        spdlog::info("\t\tDeviceID: {}", properties.deviceID);
        spdlog::info("\t\tDeviceType: {}", string_VkPhysicalDeviceType(properties.deviceType));

        spdlog::info("\t\tPipelineCacheUUID: {}", properties.pipelineCacheUUID);

        spdlog::info("\t\tdriverID           : {}", properties12.driverID);
        spdlog::info("\t\tdriverName         : {}", properties12.driverName);
        spdlog::info("\t\tdriverInfo         : {}", properties12.driverInfo);
        spdlog::info("\t\tconformanceVersion : {}", properties12.conformanceVersion);

        VkPhysicalDeviceMemoryProperties2 physicalDeviceMemoryProperties2{};
        physicalDeviceMemoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(device, &physicalDeviceMemoryProperties2);
        for (unsigned i = 0; i < physicalDeviceMemoryProperties2.memoryProperties.memoryHeapCount; i++) {
            spdlog::info("\t\tMemory Heap#{} size {} flags {}", i, physicalDeviceMemoryProperties2.memoryProperties.memoryHeaps[i].size, string_VkMemoryHeapFlags(physicalDeviceMemoryProperties2.memoryProperties.memoryHeaps[i].flags));
        }

        for (unsigned i = 0; i < physicalDeviceMemoryProperties2.memoryProperties.memoryTypeCount; i++) {
            spdlog::info("\t\tMemory type #{} heap {} flags {}", i, physicalDeviceMemoryProperties2.memoryProperties.memoryTypes[i].heapIndex, string_VkMemoryPropertyFlags(physicalDeviceMemoryProperties2.memoryProperties.memoryTypes[i].propertyFlags));
        }
    }

    return true;
}
