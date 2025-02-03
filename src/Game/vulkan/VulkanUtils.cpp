#include "VulkanUtils.h"

#include <vulkan/vk_enum_string_helper.h>

/// \brief Return Vulkan instance version supported by the Vulkan loader
[[nodiscard]] uint32_t getInstanceVersion() noexcept {
    uint32_t apiVersion = VK_API_VERSION_1_0;
    if (VkResult result = vkEnumerateInstanceVersion(&apiVersion); result != VK_SUCCESS) {
        spdlog::critical("vkEnumerateInstanceVersion() failed : {}", string_VkResult(result));
        return 0;
    }
    return apiVersion;
}

std::vector<VkLayerProperties> VulkanUtils::getInstanceLayers() noexcept {
    // From the spec:
    // The list of available layers may change at any time due to actions outside of the Vulkan
    // implementation, so two calls to vkEnumerateInstanceLayerProperties with the same parameters
    // may return different results, or retrieve different pPropertyCount values or pProperties
    // contents. Once an instance has been created, the layers enabled for that instance will
    // continue to be enabled and valid for the lifetime of that instance, even if some of them
    // become unavailable for future instances.
    uint32_t count{};
    VkResult result{VK_INCOMPLETE};
    while (result == VK_INCOMPLETE) {
        // If pProperties is NULL, then the number of layer properties available is returned in
        // pPropertyCount.
        result = vkEnumerateInstanceLayerProperties(&count, nullptr /*pProperties*/);
        if (result != VK_SUCCESS) {
            return {};
        }
        std::vector<VkLayerProperties> layers(count);
        result = vkEnumerateInstanceLayerProperties(&count, layers.data());
        if (result == VK_SUCCESS) {
            return layers;
        }
    }
    return {};
}

std::vector<VkExtensionProperties> VulkanUtils::getInstanceExtensions(
    const char* layerName /*= nullptr*/) noexcept {
    uint32_t count{};
    VkResult result{VK_INCOMPLETE};
    while (result == VK_INCOMPLETE) {
        // When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation
        // or by implicitly enabled layers are returned. When pLayerName is the name of a layer, the
        // instance extensions provided by that layer are returned.
        result = vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr /*pProperties*/);
        if (result != VK_SUCCESS) {
            return {};
        }
        std::vector<VkExtensionProperties> extensions(count);
        result = vkEnumerateInstanceExtensionProperties(layerName, &count, extensions.data());
        if (result == VK_SUCCESS) {
            return extensions;
        }
    }
    return {};
}

std::vector<VkPhysicalDevice> VulkanUtils::getPhysicalDevices(VkInstance instance) noexcept {
    uint32_t count{};
    VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (result != VK_SUCCESS) return {};

    std::vector<VkPhysicalDevice> devices{count};
    result = vkEnumeratePhysicalDevices(instance, &count, devices.data());
    if (result != VK_SUCCESS) return {};

    return devices;
}

std::vector<VkExtensionProperties> VulkanUtils::getDeviceExtensions(
    VkPhysicalDevice physicalDevice) noexcept {
    uint32_t count  = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr /*pLayerName*/,
                                                           &count, nullptr);
    if (result != VK_SUCCESS) return {};

    std::vector<VkExtensionProperties> extensions(count);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr /*pLayerName*/, &count,
                                                  extensions.data());
    if (result != VK_SUCCESS) return {};

    return extensions;
}

bool VulkanUtils::isExtensionAvailable(
    VkPhysicalDevice physicalDevice,
    const char       extensionName[VK_MAX_EXTENSION_NAME_SIZE]) noexcept {
    const auto deviceExtensionAvailable = getDeviceExtensions(physicalDevice);
    if (std::find_if(deviceExtensionAvailable.begin(), deviceExtensionAvailable.end(),
                     [&extensionName](VkExtensionProperties ext) {
                         return std::strcmp(ext.extensionName, extensionName) == 0;
                     }) != deviceExtensionAvailable.end()) {
        return true;
    }
    return false;
}

std::vector<VkQueueFamilyProperties> VulkanUtils::getDeviceQueueFamily(
    VkPhysicalDevice physicalDevice) noexcept {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, properties.data());

    return properties;
}

int32_t VulkanUtils::findDeviceQueueFamilyIndex(VkPhysicalDevice physicalDevice,
                                                VkQueueFlags     include,
                                                VkQueueFlags     exclude) noexcept {
    const auto deviceQueueFamily = getDeviceQueueFamily(physicalDevice);

    auto it = std::find_if(deviceQueueFamily.begin(), deviceQueueFamily.end(),
                           [include, exclude](VkQueueFamilyProperties queueFamilyProperties) {
                               return (queueFamilyProperties.queueFlags & include) &&
                                      !(queueFamilyProperties.queueFlags & exclude);
                           });
    if (it == deviceQueueFamily.end()) {
        return -1;
    } else {
        return it - deviceQueueFamily.begin();
    }
}

std::vector<VkPresentModeKHR> VulkanUtils::getSurfacePresentModes(VkPhysicalDevice physicalDevice,
                                                                  VkSurfaceKHR surface) noexcept {
    uint32_t surfacePresentModeCount{};
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                                       &surfacePresentModeCount, nullptr));
    std::vector<VkPresentModeKHR> presentModes(surfacePresentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &surfacePresentModeCount, presentModes.data()));
    return presentModes;
}

std::vector<VkSurfaceFormatKHR> VulkanUtils::getSurfaceFormats(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR     surface) noexcept {
    uint32_t surfaceFormatCount{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount,
                                                  nullptr));

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount,
                                                  surfaceFormats.data()));
    return surfaceFormats;
}

VkSurfaceCapabilitiesKHR VulkanUtils::getSurfacePresentModeCapabilities(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode) noexcept {
    // VK_EXT_surface_maintenance1
    VkSurfacePresentModeEXT surfacePresentModeEXT{};
    surfacePresentModeEXT.sType       = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT;
    surfacePresentModeEXT.pNext       = nullptr;
    surfacePresentModeEXT.presentMode = presentMode;

    // Provided by VK_KHR_get_surface_capabilities2
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.pNext   = &surfacePresentModeEXT;
    surfaceInfo.surface = surface;

    // Provided by VK_KHR_get_surface_capabilities2
    VkSurfaceCapabilities2KHR surfaceCapabilities{};
    surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

    // Provided by VK_KHR_get_surface_capabilities2
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo, &surfaceCapabilities);
    return surfaceCapabilities.surfaceCapabilities;
}

std::vector<VkPresentModeKHR> VulkanUtils::getSurfacePresentModeCompatibility(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode) noexcept {
    // VK_EXT_surface_maintenance1
    VkSurfacePresentModeEXT surfacePresentModeEXT{};
    surfacePresentModeEXT.sType       = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT;
    surfacePresentModeEXT.pNext       = nullptr;
    surfacePresentModeEXT.presentMode = presentMode;

    // Provided by VK_KHR_get_surface_capabilities2
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.pNext   = &surfacePresentModeEXT;
    surfaceInfo.surface = surface;

    // VK_EXT_surface_maintenance1
    VkSurfacePresentModeCompatibilityEXT presentModeCompatibility{};
    presentModeCompatibility.sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT;
    presentModeCompatibility.pNext = nullptr;
    presentModeCompatibility.presentModeCount = 0;
    presentModeCompatibility.pPresentModes    = nullptr;

    // Provided by VK_KHR_get_surface_capabilities2
    VkSurfaceCapabilities2KHR surfaceCapabilities{};
    surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    surfaceCapabilities.pNext = &presentModeCompatibility;

    // Provided by VK_KHR_get_surface_capabilities2
    // query the number of present mode capatible first.
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo, &surfaceCapabilities);

    // query all the present mode capatible.
    const uint32_t                presentModeCount = presentModeCompatibility.presentModeCount;
    std::vector<VkPresentModeKHR> presentModeKHR(presentModeCount);
    presentModeCompatibility.presentModeCount = presentModeCount;
    presentModeCompatibility.pPresentModes    = presentModeKHR.data();
    vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo, &surfaceCapabilities);

    return presentModeKHR;
}

VkSurfacePresentScalingCapabilitiesEXT VulkanUtils::getSurfacePresentScalingCapabilities(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode) noexcept {
    VkSurfacePresentModeEXT surfacePresentModeEXT{};
    surfacePresentModeEXT.sType       = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT;
    surfacePresentModeEXT.presentMode = presentMode;

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.pNext   = &surfacePresentModeEXT;
    surfaceInfo.surface = surface;

    VkSurfacePresentScalingCapabilitiesEXT scalingCapabilities{};
    scalingCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT;

    VkSurfaceCapabilities2KHR surfaceCapabilities{};
    surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    surfaceCapabilities.pNext = &scalingCapabilities;

    vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo, &surfaceCapabilities);
    return scalingCapabilities;
}

bool VulkanUtils::isSurfaceSupportExclusiveFullscreen(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR     surface,
                                                      HMONITOR         hMonitor) noexcept {
    // Provided by VK_KHR_win32_surface with VK_EXT_full_screen_exclusive
    VkSurfaceFullScreenExclusiveWin32InfoEXT fullsceenInfo{};
    fullsceenInfo.sType    = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
    fullsceenInfo.hmonitor = hMonitor;

    // Provided by VK_KHR_get_surface_capabilities2
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.pNext   = &fullsceenInfo;
    surfaceInfo.surface = surface;

    // Provided by VK_EXT_full_screen_exclusive
    VkSurfaceCapabilitiesFullScreenExclusiveEXT fullsceenCapability{};
    fullsceenCapability.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT;

    VkSurfaceCapabilities2KHR surfaceCapabilities{};
    surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    surfaceCapabilities.pNext = &fullsceenCapability;

    vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo, &surfaceCapabilities);
    return fullsceenCapability.fullScreenExclusiveSupported;
}

void VulkanUtils::transitionImageLayout(VkCommandBuffer          cmdBuffer,
                                        VkImage                  image,
                                        VkImageLayout            oldLayout,
                                        VkImageLayout            newLayout,
                                        VkPipelineStageFlagBits2 srcStageMask,
                                        VkAccessFlagBits2        srcAccessMask,
                                        VkPipelineStageFlagBits2 dstStageMask,
                                        VkAccessFlagBits2        dstAccessMask) noexcept {
    VkImageMemoryBarrier2 memoryBarrier2{};
    memoryBarrier2.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    memoryBarrier2.pNext                           = 0;
    memoryBarrier2.srcStageMask                    = srcStageMask;
    memoryBarrier2.srcAccessMask                   = srcAccessMask;
    memoryBarrier2.dstStageMask                    = dstStageMask;
    memoryBarrier2.dstAccessMask                   = dstAccessMask;
    memoryBarrier2.oldLayout                       = oldLayout;
    memoryBarrier2.newLayout                       = newLayout;
    memoryBarrier2.srcQueueFamilyIndex             = 0;
    memoryBarrier2.dstQueueFamilyIndex             = 0;
    memoryBarrier2.image                           = image;
    memoryBarrier2.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    memoryBarrier2.subresourceRange.baseMipLevel   = 0;
    memoryBarrier2.subresourceRange.levelCount     = 1;
    memoryBarrier2.subresourceRange.baseArrayLayer = 0;
    memoryBarrier2.subresourceRange.layerCount     = 1;

    // Provided by VK_VERSION_1_3 or VK_KHR_synchronization2
    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext                    = nullptr;
    dependencyInfo.dependencyFlags          = 0;
    dependencyInfo.memoryBarrierCount       = 0;
    dependencyInfo.pMemoryBarriers          = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pBufferMemoryBarriers    = nullptr;
    dependencyInfo.imageMemoryBarrierCount  = 1;
    dependencyInfo.pImageMemoryBarriers     = &memoryBarrier2;
    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
}

// ===========================================================================
//
//                     std formater specialization
//
// ===========================================================================

auto std::formatter<VkResult>::format(VkResult        result,
                                      format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{}", string_VkResult(result));
}

auto std::formatter<uint8_t[VK_UUID_SIZE]>::format(
    uint8_t uuid[VK_UUID_SIZE], format_context& ctx) const -> decltype(ctx.out()) {
    auto&& out = ctx.out();
    return std::format_to(
        out, "{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}-{:X}",
        (int)uuid[0], (int)uuid[1], (int)uuid[2], (int)uuid[3], (int)uuid[4], (int)uuid[5],
        (int)uuid[6], (int)uuid[7], (int)uuid[8], (int)uuid[9], (int)uuid[10], (int)uuid[11],
        (int)uuid[12], (int)uuid[13], (int)uuid[14], (int)uuid[15]);
    return out;
}

auto std::formatter<VkLayerProperties>::format(VkLayerProperties layer,
                                               format_context&   ctx) const -> decltype(ctx.out()) {
    return std::format_to(
        ctx.out(), "{} SpecVersion: {}.{}.{}.{} ImplVersion: {} {}", layer.layerName,
        VK_API_VERSION_VARIANT(layer.specVersion), VK_API_VERSION_MAJOR(layer.specVersion),
        VK_API_VERSION_MINOR(layer.specVersion), VK_API_VERSION_PATCH(layer.specVersion),
        layer.implementationVersion, layer.description);
}

auto std::formatter<VkExtensionProperties>::format(
    VkExtensionProperties extension, format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{} [Version: {}]", extension.extensionName,
                          extension.specVersion);
}

auto std::formatter<VkConformanceVersion>::format(
    VkConformanceVersion version, format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{}.{}.{}.{}", version.major, version.minor, version.subminor,
                          version.patch);
}

auto std::formatter<VkDriverId>::format(VkDriverId      id,
                                        format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{}", string_VkDriverId(id));
}

auto std::formatter<VkExtent2D>::format(VkExtent2D      extent,
                                        format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{}x{}", extent.width, extent.height);
}
auto std::formatter<VkExtent3D>::format(VkExtent3D      extent,
                                        format_context& ctx) const -> decltype(ctx.out()) {
    return std::format_to(ctx.out(), "{}x{}x{}", extent.width, extent.height, extent.depth);
}

auto std::formatter<VkSurfaceFormatKHR>::format(VkSurfaceFormatKHR format,
                                                format_context& ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "{} {}", string_VkFormat(format.format),
                     string_VkColorSpaceKHR(format.colorSpace));
}
