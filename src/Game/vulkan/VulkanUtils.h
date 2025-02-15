#pragma once
#include "vulkan.h"
#include <spdlog/spdlog.h>

#include <format>
#include <vector>

#define STRINIFY(x) #x

// Check the return value of a vulkan function.
// If not VK_SUCCESS, log a error and abort.
#define VK_CHECK(func)                                                                     \
    do {                                                                                   \
        auto result = func;                                                                \
        if (result != VK_SUCCESS) {                                                        \
            spdlog::critical("Detected Vulkan error: function {} return {} @{}:{}",        \
                             STRINIFY(func), string_VkResult(result), __FILE__, __LINE__); \
            abort();                                                                       \
        }                                                                                  \
    } while (0)

namespace VulkanUtils {

/// \brief Return Vulkan instance version supported by the Vulkan loader
[[nodiscard]] uint32_t getInstanceVersion() noexcept;

/// \brief Return all the layers available on the system.
/// \return All the layers available on the system.
[[nodiscard]] std::vector<VkLayerProperties> getInstanceLayers() noexcept;

/// \brief Get the extensions required by a layer.
/// \param[in] layerNameThelayer name.
/// \return The extensions required by the layer.
[[nodiscard]] std::vector<VkExtensionProperties> getInstanceExtensions(
    const char* layerName = nullptr) noexcept;

/// \brief Return all physical devices (GPU) on the system.
/// \param[in] instance The vulkan instance used to query the devices.
/// \return All devices on the system.
[[nodiscard]] std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) noexcept;

/// \brief Return all extensions supported for the given physical device.
/// \param[in] physicalDevice The physical device for which the extensions are returned.
/// \return The extensions supported for the given physical device.
[[nodiscard]] std::vector<VkExtensionProperties> getDeviceExtensions(
    VkPhysicalDevice physicalDevice) noexcept;

/// \brief Check if an extension is supported on a physical device.
/// \param[in] physicalDevice The physical device.
/// \param[in] extensionName  The extension name.
/// \return True if the extension is supported, false otherwise.
[[nodiscard]] bool isExtensionAvailable(
    VkPhysicalDevice physicalDevice, const char extensionName[VK_MAX_EXTENSION_NAME_SIZE]) noexcept;

/// \brief Get all queue family of a specific physical device.
/// \param[in] physicalDevice The physical device.
/// \return All all queue family of \p physicalDevice
[[nodiscard]] std::vector<VkQueueFamilyProperties> getDeviceQueueFamily(
    VkPhysicalDevice physicalDevice) noexcept;

/// \brief Find the first family queue that include specific flags and optionally exclude
///        specific flags.
/// \param[in] physicalDevice The physical device.
/// \param[in] include Flags to include.
/// \param[in] exclude Flags to exclude.
/// \return The index of the family queue or -1 if no family queue is found.
[[nodiscard]] int32_t findDeviceQueueFamilyIndex(VkPhysicalDevice physicalDevice,
                                                 VkQueueFlags     include,
                                                 VkQueueFlags     exclude = 0) noexcept;

/// \brief Get all supported presentation modes for a surface.
///
///  This is a thin wrapper on top of vkGetPhysicalDeviceSurfacePresentModesKHR()
///
/// \param[in] physicalDevice the physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        the surface that will be associated with the swapchain.
/// \return All supported presentation mode.
std::vector<VkPresentModeKHR> getSurfacePresentModes(VkPhysicalDevice physicalDevice,
                                                     VkSurfaceKHR     surface) noexcept;

/// \brief Get all supported format for a surface.
///
///  This is a thin wrapper on top of vkGetPhysicalDeviceSurfaceFormatsKHR()
///
/// \param[in] physicalDevice the physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        the surface that will be associated with the swapchain.
/// \return All supported format.
std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkPhysicalDevice physicalDevice,
                                                  VkSurfaceKHR     surface) noexcept;

/// \brief Get the surface capabilities for a specific present mode
///
/// \note required VK_KHR_get_surface_capabilities2
/// \note required VK_EXT_surface_maintenance1
///
/// \param[in] physicalDevice The physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        The surface that will be associated with the swapchain.
/// \param[in] presentMode    The presentation mode for which the capability will be
///                           returned.
/// \return The surface capabilities for the presentation mode.
///
/// \warning presentMode most be a uspported presentMode of the surface.
///
VkSurfaceCapabilitiesKHR getSurfacePresentModeCapabilities(VkPhysicalDevice physicalDevice,
                                                           VkSurfaceKHR     surface,
                                                           VkPresentModeKHR presentMode) noexcept;

/// \brief Get the surface present mode compatibility
///
///  Compatible presentation modes allow switching without swapchain recreation.
///
/// \note required VK_KHR_get_surface_capabilities2
/// \note required VK_EXT_surface_maintenance1
///
/// \param[in] physicalDevice The physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        The surface that will be associated with the swapchain.
/// \param[in] presentMode    The presentation mode for which the compatible presentation
///                           modes  will be query.
/// \return All compatible presentation mode for the given presentation mode.
///
/// \warning presentMode most be a uspported presentMode of the surface.
///
std::vector<VkPresentModeKHR> getSurfacePresentModeCompatibility(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode) noexcept;

/// \brief Get the presentation scaling capabilities of the surface
///
/// \note required VK_KHR_get_surface_capabilities2
/// \note required VK_EXT_surface_maintenance1
///
/// \param[in] physicalDevice The physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        The surface that will be associated with the swapchain.
/// \param[in] presentMode    The presentation mode for which the scaling capability willbe
///                           returned.
/// \return The surface scaling capabilities for the presentation mode.
///
/// \warning presentMode most be a uspported presentMode of the surface.
///
VkSurfacePresentScalingCapabilitiesEXT getSurfacePresentScalingCapabilities(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode) noexcept;

/// \brief Check if a surface is able to use exclusive full-screen on a givin monitor.
///
/// \note required VK_KHR_get_surface_capabilities2
/// \note required VK_EXT_full_screen_exclusive
/// \note required VK_KHR_win32_surface
///
/// \param[in] physicalDevice The physical device that will be associated with the swapchain to be
///                           created
/// \param[in] surface        The surface that will be associated with the swapchain.
/// \param[in] hMonitor       The Win32 HMONITOR handle identifying the display to create the
///                           surface with.
/// \return true if the surface is able to make use of exclusive full-screen access.
bool isSurfaceSupportExclusiveFullscreen(VkPhysicalDevice physicalDevice,
                                         VkSurfaceKHR     surface,
                                         HMONITOR         hMonitor) noexcept;

/// \brief
/// \param[in] cmdBuffer
/// \param[in] image
/// \param[in] oldLayout
/// \param[in] newLayout
/// \param[in] srcStageMask
/// \param[in] srcAccessMask
/// \param[in] dstStageMask
/// \param[in] dstAccessMask
/// \param[in] mipmap
/// \return
void transitionImageLayout(VkCommandBuffer          cmdBuffer,
                           VkImage                  image,
                           VkImageLayout            oldLayout,
                           VkImageLayout            newLayout,
                           VkPipelineStageFlagBits2 srcStageMask,
                           VkAccessFlagBits2        srcAccessMask,
                           VkPipelineStageFlagBits2 dstStageMask,
                           VkAccessFlagBits2        dstAccessMask,
                           uint32_t                 mipmap) noexcept;
} // namespace VulkanUtils

#include <vulkan/vk_enum_string_helper.h>
namespace std {
template <>
struct formatter<VkResult> : formatter<string> {
    format_context::iterator format(VkResult result, format_context& ctx) const;
};

template <>
struct formatter<uint8_t[VK_UUID_SIZE]> {
    constexpr auto parse(const std::format_parse_context& ctx) {
        auto it = ctx.begin();
        return it;
    }

    format_context::iterator format(uint8_t uuid[VK_UUID_SIZE], format_context& ctx) const;
};

template <>
struct formatter<VkLayerProperties> : formatter<string> {
    format_context::iterator format(VkLayerProperties layer, format_context& ctx) const;
};

template <>
struct formatter<VkExtensionProperties> : formatter<string> {
    format_context::iterator format(VkExtensionProperties extension, format_context& ctx) const;
};

template <>
struct formatter<VkConformanceVersion> : formatter<string> {
    format_context::iterator format(VkConformanceVersion version, format_context& ctx) const;
};

template <>
struct formatter<VkDriverId> : formatter<string> {
    format_context::iterator format(VkDriverId id, format_context& ctx) const;
};

template <>
struct formatter<VkExtent2D> : formatter<string> {
    format_context::iterator format(VkExtent2D id, format_context& ctx) const;
};

template <>
struct formatter<VkExtent3D> : formatter<string> {
    format_context::iterator format(VkExtent3D id, format_context& ctx) const;
};

template <>
struct std::formatter<VkSurfaceFormatKHR> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    format_context::iterator format(VkSurfaceFormatKHR format, format_context& ctx) const;
};

template <>
struct formatter<VkPresentModeKHR> : formatter<string> {
    auto format(VkPresentModeKHR mode, format_context& ctx) const {
        return format_to(ctx.out(), "{}", string_VkPresentModeKHR(mode));
    }
};

template <>
struct formatter<VkSurfaceTransformFlagBitsKHR> : formatter<string> {
    format_context::iterator format(VkSurfaceTransformFlagBitsKHR flag, format_context& ctx) const {
        return format_to(ctx.out(), "{}", string_VkSurfaceTransformFlagBitsKHR(flag));
    }
};
} // namespace std
