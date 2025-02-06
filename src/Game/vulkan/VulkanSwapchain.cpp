#include "VulkanSwapchain.h"

#include "VulkanUtils.h"

#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>

namespace {
void print(VkSurfaceCapabilitiesKHR cap) {
    spdlog::info("\tCurrentExtent:       {}", cap.currentExtent);
    spdlog::info("\tMinImageExtent:      {}", cap.minImageExtent);
    spdlog::info("\tMaxImageExtent:      {}", cap.maxImageExtent);
    spdlog::info("\tMinmageExtent:       {}", cap.minImageCount);
    spdlog::info("\tMaxImageExtent:      {}", cap.maxImageCount);
    spdlog::info("\tCurrentTransform:    {}", cap.currentTransform);
    spdlog::info("\tSupportedTransforms: {}",
                 string_VkSurfaceTransformFlagsKHR(cap.supportedTransforms));
    spdlog::info("\tMaxImageArrayLayers: {}", cap.maxImageArrayLayers);
    spdlog::info("\tMaxImageArrayLayers: {}",
                 string_VkCompositeAlphaFlagsKHR(cap.supportedCompositeAlpha));
    spdlog::info("\tSupportedUsageFlags: {}", string_VkImageUsageFlags(cap.supportedUsageFlags));
}

void print(VkSurfacePresentScalingCapabilitiesEXT cap) {
    spdlog::info("\tSupportedPresentScaling:  {}",
                 string_VkPresentScalingFlagsEXT(cap.supportedPresentScaling));
    spdlog::info("\tSupportedPresentGravityX: {}",
                 string_VkPresentGravityFlagsEXT(cap.supportedPresentGravityX));
    spdlog::info("\tSupportedPresentGravityY: {}",
                 string_VkPresentGravityFlagsEXT(cap.supportedPresentGravityY));
    spdlog::info("\tMinScaledImageExtent:     {}", cap.minScaledImageExtent);
    spdlog::info("\tMaxScaledImageExtent:     {}", cap.maxScaledImageExtent);
}

} // namespace

VulkanSwapchain::VulkanSwapchain(VkInstance       instance,
                                 VkPhysicalDevice physicalDevice,
                                 VkDevice         device,
                                 VkSurfaceKHR     surface)
    : mInstance{instance}, mPhysicalDevice{physicalDevice}, mDevice{device}, mSurface{surface} {}

VulkanSwapchain::~VulkanSwapchain() {
    destroyRessources();

    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
}

void VulkanSwapchain::resize(uint32_t width, uint32_t height) { build(); }

void VulkanSwapchain::acquireNextImage() noexcept {
    mImageAvailableSemaphore = mImageAvailableSemaphores[mCurrentSemaphoreIndex];
    mRenderFinishSemaphor    = mImageAvailableSemaphores[mCurrentSemaphoreIndex];
    mCurrentSemaphoreIndex   = (mCurrentSemaphoreIndex + 1) % mImageCount;
    acquireNextImage(mImageAvailableSemaphore, mCurrentBackImageIndex);
}

void VulkanSwapchain::acquireNextImage(VkSemaphore signalSemaphore, uint32_t& imageIndex) noexcept {
    // - If timeout is zero, then vkAcquireNextImageKHR does not wait, and will either successfully
    //      acquire an image or fail and return VK_NOT_READY if no image is available.
    // - If the specified timeout period expires before an image is acquired, vkAcquireNextImageKHR
    //      returns VK_TIMEOUT.
    // - If timeout is UINT64_MAX, the timeout period is treated as infinite, and
    //      vkAcquireNextImageKHR will block until an image is acquired or an error occurs.
    //
    // If vkAcquireNextImageKHR is used, the device mask is considered to include all physical
    // devices in the logical device.
    VkAcquireNextImageInfoKHR acquireNextImageInfoKHR{};
    acquireNextImageInfoKHR.sType      = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquireNextImageInfoKHR.pNext      = nullptr;
    acquireNextImageInfoKHR.swapchain  = mSwapchain;
    acquireNextImageInfoKHR.timeout    = UINT64_MAX;
    acquireNextImageInfoKHR.semaphore  = signalSemaphore;
    acquireNextImageInfoKHR.fence      = nullptr;
    acquireNextImageInfoKHR.deviceMask = 1;
    VkResult result = vkAcquireNextImage2KHR(mDevice, &acquireNextImageInfoKHR, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // TODO: recreate the swapchain
        spdlog::warn("vkAcquireNextImage2KHR return {}", result);
        build();
    } else if (result == VK_NOT_READY) {
        // VK_NOT_READY is returned if timeout is zero and no image was available.
        spdlog::warn("vkAcquireNextImage2KHR return {}", result);
    } else if (result == VK_TIMEOUT) {
        // VK_TIMEOUT is returned if timeout is greater than zero and less than UINT64_MAX,
        // and no image became available within the time allowed.
        spdlog::warn("vkAcquireNextImage2KHR return {}", result);
    } else if (result == VK_ERROR_SURFACE_LOST_KHR) {
        // VK_ERROR_SURFACE_LOST_KHR is returned if the surface becomes no longer available.
        spdlog::warn("vkAcquireNextImage2KHR return {}", result);
    }
}

void VulkanSwapchain::build() {
    destroyRessources();

    mPresentMode     = selectPresentMode();
    mSwapchainFormat = selectSurfaceFormat();

    spdlog::info("............ Binding swap chain .. ........");
#if 0
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &mSurfaceCapabilities);
    spdlog::info("Surface Capabilities");
    print(mSurfaceCapabilities);
#else
    // VkDisplayNativeHdrSurfaceCapabilitiesAMD;    // Provided by VK_AMD_display_native_hdr
    // VkLatencySurfaceCapabilitiesNV;              // Provided by VK_NV_low_latency2
    // VkSharedPresentSurfaceCapabilitiesKHR;       // Provided by VK_KHR_shared_presentable_image
    // VkSurfaceCapabilitiesPresentBarrierNV;       // Provided by VK_NV_present_barrier
    // VkSurfaceProtectedCapabilitiesKHR;           // Provided by
    // VK_KHR_surface_protected_capabilities

    auto cap1 = VulkanUtils::getSurfacePresentModeCapabilities(mPhysicalDevice, mSurface,
                                                               VK_PRESENT_MODE_FIFO_KHR);
    auto cap2 = VulkanUtils::getSurfacePresentModeCapabilities(mPhysicalDevice, mSurface,
                                                               VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    auto cap3 = VulkanUtils::getSurfacePresentModeCapabilities(mPhysicalDevice, mSurface,
                                                               VK_PRESENT_MODE_MAILBOX_KHR);
    auto cap4 = VulkanUtils::getSurfacePresentModeCapabilities(mPhysicalDevice, mSurface,
                                                               VK_PRESENT_MODE_IMMEDIATE_KHR);

    auto comp1 = VulkanUtils::getSurfacePresentModeCompatibility(mPhysicalDevice, mSurface,
                                                                 VK_PRESENT_MODE_FIFO_KHR);
    auto comp2 = VulkanUtils::getSurfacePresentModeCompatibility(mPhysicalDevice, mSurface,
                                                                 VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    auto comp3 = VulkanUtils::getSurfacePresentModeCompatibility(mPhysicalDevice, mSurface,
                                                                 VK_PRESENT_MODE_MAILBOX_KHR);
    auto comp4 = VulkanUtils::getSurfacePresentModeCompatibility(mPhysicalDevice, mSurface,
                                                                 VK_PRESENT_MODE_IMMEDIATE_KHR);

    auto scap1 = VulkanUtils::getSurfacePresentScalingCapabilities(mPhysicalDevice, mSurface,
                                                                   VK_PRESENT_MODE_FIFO_KHR);
    auto scap2 = VulkanUtils::getSurfacePresentScalingCapabilities(
        mPhysicalDevice, mSurface, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    auto scap3 = VulkanUtils::getSurfacePresentScalingCapabilities(mPhysicalDevice, mSurface,
                                                                   VK_PRESENT_MODE_MAILBOX_KHR);
    auto scap4 = VulkanUtils::getSurfacePresentScalingCapabilities(mPhysicalDevice, mSurface,
                                                                   VK_PRESENT_MODE_IMMEDIATE_KHR);

    auto bFullScreen = VulkanUtils::isSurfaceSupportExclusiveFullscreen(
        mPhysicalDevice, mSurface, MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY));

    spdlog::info("VK_PRESENT_MODE_FIFO_KHR");
    print(scap1);
    spdlog::info("VK_PRESENT_MODE_FIFO_RELAXED_KHR");
    print(scap2);
    spdlog::info("VK_PRESENT_MODE_MAILBOX_KHR");
    print(scap3);
    spdlog::info("VK_PRESENT_MODE_IMMEDIATE_KHR");
    print(scap4);
    VkPhysicalDeviceSurfaceInfo2KHR physicalDeviceSurfaceInfo2KHR;
    physicalDeviceSurfaceInfo2KHR.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    physicalDeviceSurfaceInfo2KHR.pNext   = nullptr;
    physicalDeviceSurfaceInfo2KHR.surface = mSurface;
    VkSurfaceCapabilities2KHR surfaceCapabilities2{};
    surfaceCapabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    auto r                     = vkGetPhysicalDeviceSurfaceCapabilities2KHR(
        mPhysicalDevice, &physicalDeviceSurfaceInfo2KHR, &surfaceCapabilities2);

    mSurfaceCapabilities = surfaceCapabilities2.surfaceCapabilities;
#endif
    // Define the swapchain size.
    // the special value (0xFFFFFFFF, 0xFFFFFFFF) indicating that the surface size
    // will be determined by the extent of a swapchain targeting the surface.
    mSwapchainSize = mSurfaceCapabilities.currentExtent;
    if (mSurfaceCapabilities.currentExtent.width == 0xFFFFFFFF &&
        mSurfaceCapabilities.currentExtent.height == 0xFFFFFFFF) {
        mSwapchainSize = mSurfaceCapabilities.minImageExtent;
    }

    // FIXME: Should maybe be based on present mode ... ?
    // MAILBOX need at least 3 images ....
    // 3 images useless for FIFO ?
    mImageCount = 8;
    if (mImageCount > mSurfaceCapabilities.maxImageCount) {
        mImageCount = mSurfaceCapabilities.maxImageCount;
    } else if (mImageCount < mSurfaceCapabilities.minImageCount) {
        mImageCount = mSurfaceCapabilities.minImageCount;
    }

    // Provided by VK_EXT_swapchain_maintenance1
    // The entries in pPresentModes must be a subset of the present modes returned in
    // VkSurfacePresentModeCompatibilityEXT::pPresentModes, given
    // VkSwapchainCreateInfoKHR::presentMode in VkSurfacePresentModeEXT
    const std::vector<VkPresentModeKHR> presentModeCompatible =
        VulkanUtils::getSurfacePresentModeCompatibility(mPhysicalDevice, mSurface, mPresentMode);
    VkSwapchainPresentModesCreateInfoEXT presentModesCI{};
    presentModesCI.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT;
    presentModesCI.presentModeCount = presentModeCompatible.size();
    presentModesCI.pPresentModes    = presentModeCompatible.data();

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext              = &presentModesCI;
    swapchainCreateInfo.flags              = {};
    swapchainCreateInfo.surface            = mSurface;
    swapchainCreateInfo.minImageCount      = mImageCount;
    swapchainCreateInfo.imageFormat        = mSwapchainFormat.format;
    swapchainCreateInfo.imageColorSpace    = mSwapchainFormat.colorSpace;
    swapchainCreateInfo.imageExtent.width  = mSwapchainSize.width;
    swapchainCreateInfo.imageExtent.height = mSwapchainSize.height;
    swapchainCreateInfo.imageArrayLayers   = 1;
    swapchainCreateInfo.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform       = mSurfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode        = mPresentMode;
    swapchainCreateInfo.clipped            = VK_TRUE;
    swapchainCreateInfo.oldSwapchain       = mSwapchain;
    swapchainCreateInfo.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE; // FIXME: Swapchain sharing mode
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices   = 0;
    VK_CHECK(vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain));

    // delete the old swapchain
    vkDestroySwapchainKHR(mDevice, swapchainCreateInfo.oldSwapchain, nullptr);

    //
    // Get the swap chain images
    //
    uint32_t imageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr));
    mImages.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mImages.data()));

    //
    // Create a image view for each swapchain image.
    // Create the binary semaphores for each swapchain image
    //
    mImageViews.resize(imageCount);
    mImageAvailableSemaphores.resize(imageCount);
    mRenderFinishSemaphores.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo info{};
        info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext                           = 0;
        info.flags                           = 0;
        info.image                           = mImages[i];
        info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        info.format                          = swapchainCreateInfo.imageFormat;
        info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel   = 0;
        info.subresourceRange.levelCount     = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount     = 1;
        VK_CHECK(vkCreateImageView(mDevice, &info, nullptr, &mImageViews[i]));

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
                                   &mImageAvailableSemaphores[i]));
        VK_CHECK(
            vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishSemaphores[i]));

        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.pNext;
        nameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;

        nameInfo.objectHandle = (uint64_t)mImageAvailableSemaphores[i];
        auto name             = std::format("swapchain_image_available_{}", i);
        nameInfo.pObjectName  = name.c_str();
        //vkSetDebugUtilsObjectNameEXT(mDevice, &nameInfo);

        nameInfo.objectHandle = (uint64_t)mRenderFinishSemaphores[i];
        name                  = std::format("swapchain_image_present_ready{}", i);
        nameInfo.pObjectName  = name.c_str();
        //vkSetDebugUtilsObjectNameEXT(mDevice, &nameInfo);
    }

    spdlog::info("............ Swap chain created ...........");
    spdlog::info("\tSize:         {}", mSwapchainSize);
    spdlog::info("\tFormat:       {}", mSwapchainFormat);
    spdlog::info("\tImage Count:  {}", mImageCount);
    spdlog::info("\tSize:         {}", mSwapchainSize);
    spdlog::info("\tPresent Mode: {}", mPresentMode);

    acquireNextImage();
}

void VulkanSwapchain::present(VkQueue queue, VkPresentModeKHR presentModes) {
    const auto        swapchainHandle = getHandle();
    const uint32_t    imageIndex      = getCurrentBackImageIndex();
    const VkSemaphore waitSemaphore   = getRenderFinishSemaphores();

    // Provided by VK_EXT_swapchain_maintenance1
    VkSwapchainPresentModeInfoEXT swapchainPresentModeInfo{};
    swapchainPresentModeInfo.sType          = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT;
    swapchainPresentModeInfo.swapchainCount = 1;
    swapchainPresentModeInfo.pPresentModes  = &presentModes;

    // Queue an image for presentation
    //
    // Note
    //		There is no requirement for an application to present images in the same order
    //      that they were acquired. Applications can arbitrarily present any image that
    //      is currently acquired.
    //
    // If the presentation request is rejected by the presentation engine with an error
    // VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, or
    // VK_ERROR_SURFACE_LOST_KHR, the set of queue operations are still considered to be
    // enqueued and thus any semaphore wait operation specified in VkPresentInfoKHR will execute
    // when the corresponding queue operation is complete.
    //
    // Calls to vkQueuePresentKHR may block, but must return in finite time.
    //
    // Note: Before an application can present an image, the image’s layout must be transitioned
    //       to the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout, or for a shared presentable image
    //       the VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR layout.
    //
    // Note:
    //		When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
    //      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    //      there is no need to delay subsequent processing, or perform any visibility operations
    //      (as vkQueuePresentKHR performs automatic visibility operations). To achieve this, the
    //      dstAccessMask member of the VkImageMemoryBarrier should be set to 0, and the
    //      dstStageMask parameter should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
    VkPresentInfoKHR presentInfoKHR{};
    presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    if (mPresentMode != presentModes) {
        // this look to cause performance issue when goingback to mailbox

        // presentInfoKHR.pNext = &swapchainPresentModeInfo;
        // mPresentMode         = presentModes;
    }

    presentInfoKHR.waitSemaphoreCount = 1;
    presentInfoKHR.pWaitSemaphores    = &waitSemaphore;
    presentInfoKHR.swapchainCount     = 1;
    presentInfoKHR.pSwapchains        = &swapchainHandle;
    presentInfoKHR.pImageIndices      = &imageIndex;
    presentInfoKHR.pResults           = 0;
    VkResult result                   = vkQueuePresentKHR(queue, &presentInfoKHR);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        spdlog::warn("vkQueuePresentKHR ........  {}", result);
        //build();
    } else if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
        spdlog::warn("vkQueuePresentKHR() failed : {}", result);
    } else if (result != VK_SUCCESS) {
        spdlog::critical("vkQueuePresentKHR() failed : {}", result);
    }

    acquireNextImage();
}

VkPresentModeKHR VulkanSwapchain::selectPresentMode() const {
    const std::vector<VkPresentModeKHR> presentModes =
        VulkanUtils::getSurfacePresentModes(mPhysicalDevice, mSurface);
    spdlog::info("Found {} presentation modes.", presentModes.size());
    for (auto& mode : presentModes) {
        spdlog::info("\t {}", mode);
    }

    // From
    // https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-2.html
    //
    // VK_PRESENT_MODE_IMMEDIATE_KHR
    //  Present requests are applied immediately and tearing may be observed (depending on the
    //  frames per second). Internally the presentation engine doesn't use any queue for holding
    //  swap chain images.
    // VK_PRESENT_MODE_FIFO_KHR
    //  This mode is the most similar to OpenGL's buffer swapping with a swap interval set to 1.
    //  The image is displayed (replaces currently displayed image) only on vertical blanking
    //  periods, so no tearing should be visible. Internally, the presentation engine uses FIFO
    //  queue with “numSwapchainImages – 1” elements. Present requests are appended to the end of
    //  this queue. During blanking periods, the image from the beginning of the queue replaces the
    //  currently displayed image, which may become available to application. If all images are in
    //  the queue, the application has to wait until v-sync releases the currently displayed image.
    //  Only after that does it becomes available to the application and program may render image
    //  into it.
    //   ** This mode must always be available in all Vulkan implementations supporting swap chain
    //   extension.
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR
    //   This mode is similar to FIFO, but when the image is displayed longer than one blanking
    //   period it may be released immediately without waiting for another v-sync signal (so if we
    //   are rendering frames with lower frequency than screen's refresh rate, tearing may be
    //   visible)
    // VK_PRESENT_MODE_MAILBOX_KHR
    //   This mode is the most similar to the mentioned triple buffering.
    //   The image is displayed only on vertical blanking periods and no tearing should be visible.
    //   But internally, the presentation engine uses the queue with only a single element.
    //   One image is displayed and one waits in the queue.
    //   If application wants to present another image it is not appended to the end of the queue
    //   but replaces the one that waits. So in the queue there is always the most recently
    //   generated image. This behavior is available if there are more than two images. For two
    //   images MAILBOX mode behaves similarly to FIFO (as we have to wait for the displayed image
    //   to be released, we don't have “spare” image which can be exchanged with the one that waits
    //   in the queue).

    auto isSupported = [&presentModes](VkPresentModeKHR mode) {
        auto it = std::find_if(presentModes.begin(), presentModes.end(),
                               [&mode](VkPresentModeKHR m) { return mode == m; });
        return it != presentModes.end();
    };

    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    if (isSupported(VK_PRESENT_MODE_MAILBOX_KHR)) {
        mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    return mode;
}

VkSurfaceFormatKHR VulkanSwapchain::selectSurfaceFormat() const {
    std::vector<VkSurfaceFormatKHR> surfaceFormats =
        VulkanUtils::getSurfaceFormats(mPhysicalDevice, mSurface);

    spdlog::info("Found {} surface format.", surfaceFormats.size());
    for (auto& format : surfaceFormats) {
        spdlog::info("\t {}", format);
    }

    auto isSupported = [&surfaceFormats](VkFormat format, VkColorSpaceKHR colorSpace) {
        auto it = std::find_if(surfaceFormats.begin(), surfaceFormats.end(),
                               [&format, &colorSpace](VkSurfaceFormatKHR sf) {
                                   return sf.format == format && sf.colorSpace == colorSpace;
                               });
        return it != surfaceFormats.end();
    };

    if (isSupported(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // not found use the first one
    return surfaceFormats[0];
}

void VulkanSwapchain::destroyRessources() {
    vkDeviceWaitIdle(mDevice);

    for (unsigned i = 0; i < getBufferCount(); i++) {
        vkDestroyImageView(mDevice, mImageViews[i], nullptr);
        vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice, mRenderFinishSemaphores[i], nullptr);
    }

    mImageViews.clear();
    mImageAvailableSemaphores.clear();
    mRenderFinishSemaphores.clear();
}
