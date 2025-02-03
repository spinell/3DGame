#pragma once
#include "vulkan.h"

#include <vector>

/**
 * @brief
 * TODO: VkSwapchainPresentScalingCreateInfoEXT
 * TODO: VK_EXT_swapchain_maintenance1
 */
class VulkanSwapchain {
public:
    /**
     * @brief
     *
     * @param [in] physicalDevice
     * @param [in] device
     * @param [in] surface The surface used to create the swapchain.
     */
    VulkanSwapchain(VkInstance       instance,
                    VkPhysicalDevice physicalDevice,
                    VkDevice         device,
                    VkSurfaceKHR     surface);

    ~VulkanSwapchain();

    VulkanSwapchain(VulkanSwapchain&)             = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&)  = delete;
    VulkanSwapchain(VulkanSwapchain&&)            = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

    /**
     * @brief Resize the swapchain to the specific width hand height.
     *        The new width and height will be clamped by the swapchain' surface capacity.
     * @param width The new width of the swapchain.
     * @param height The new height of the swapchain.
     */
    void resize(uint32_t width, uint32_t height);

    /**
     * @brief Build / Rebuild the swapchain.
     */
    void build();

    /**
     * @brief Get the of the swapchaine images.
     * @return The of the swapchaine images.
     */
    [[nodiscard]] VkExtent2D getSize() const noexcept { return mSwapchainSize; }

    /**
     * @brief Get the swapchain handle.
     * @return The swapchain handle.
     * @warning Caller must not delete the handle.
     */
    [[nodiscard]] const VkSwapchainKHR& getHandle() const noexcept { return mSwapchain; }

    /**
     * @brief Get the number of images the swapchain contain.
     * @return The number of images the swapchain contain.
     */
    [[nodiscard]] uint32_t getBufferCount() const noexcept { return mImageCount; }

    /**
     * @brief Get the image views of the swap chain.
     * @return Tthe image views of the swap chain.
     * @warning Caller must not delete the images.
     */
    [[nodiscard]] const std::vector<VkImage>& getImages() const noexcept { return mImages; }

    /**
     * @brief Get the image Views of the swap chain.
     * @return The Image Views of the swap chain.
     * @warning Caller must not delete the views.
     */
    [[nodiscard]] const std::vector<VkImageView>& getImageViews() const noexcept {
        return mImageViews;
    }
    [[nodiscard]] VkSemaphore getImageAvailableSemaphores() const noexcept {
        return mImageAvailableSemaphore;
    }
    [[nodiscard]] VkSemaphore getRenderFinishSemaphores() const noexcept {
        return mRenderFinishSemaphor;
    }

    /**
     * @brief Get the Surface Capabilities.
     * @return The Surface Capabilities.
     */
    [[nodiscard]] const VkSurfaceCapabilitiesKHR& getSurfaceCapabilities() const noexcept {
        return mSurfaceCapabilities;
    }

    /**
     * @brief Get the pixel format used by the swapchaine.
     * @return The VkFormat of the swapchain.
     */
    [[nodiscard]] VkFormat getFormat() const noexcept { return mSwapchainFormat.format; }

    void acquireNextImage() noexcept;
    void acquireNextImage(VkSemaphore signalSemaphore, uint32_t& imageIndex) noexcept;

    [[nodiscard]] uint32_t getCurrentBackImageIndex() const noexcept {
        return mCurrentBackImageIndex;
    }

    void present(VkQueue queue, VkPresentModeKHR presentModes);

private:
    /**
     * @brief Choose a present mode base on the swapchain's surface supported present mode
     *        For now, always return VK_PRESENT_MODE_MAILBOX_KHR if available.
     *        Otherwise, VK_PRESENT_MODE_FIFO_KHR.
     * @return The selected present mode.
     *
     * TODO: Add support for VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR and SHARED_DEMAND_REFRESH_KHR
     * ?
     */
    VkPresentModeKHR selectPresentMode() const;

    /**
     * @brief Chose the swapchain pixel format and color space based on format surpported by the
     * surface. For now, always return VK_FORMAT_B8G8R8A8_SRGB and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
     * if available. Otherwise, return the first available surface format.
     * @return The chosen format.
     */
    VkSurfaceFormatKHR selectSurfaceFormat() const;

    void destroyRessources();

private:
    VkInstance         mInstance{};
    VkPhysicalDevice   mPhysicalDevice{};  /**< */
    VkDevice           mDevice{};          /**< */
    VkSurfaceKHR       mSurface{};         /**< The vulkan surface used by the swapchain.*/
    VkSwapchainKHR     mSwapchain{};       /**< The swapchain vulkan handle.*/
    VkPresentModeKHR   mPresentMode{};     /**< The presentation mode used by the swapchain.*/
    VkExtent2D         mSwapchainSize{};   /**< The swapchain size.*/
    VkSurfaceFormatKHR mSwapchainFormat{}; /**< The swapchain pixel format and color space.*/

    /**< The capabilities of the surface used by the swapchain.*/
    VkSurfaceCapabilitiesKHR mSurfaceCapabilities{};
    uint32_t                 mImageCount{}; /**< The number of image in the swapchain.*/

    /**< The swapchain' images. Do not destroy them.*/
    std::vector<VkImage> mImages{};

    /**< The image views corresponding to each swapchain' images.*/
    std::vector<VkImageView> mImageViews{};

    // binary semaphore signaled when swapchain is ready
    //  - signaling by acquireNextImage
    //  - waiting   by vkQueueSubmit2
    std::vector<VkSemaphore> mImageAvailableSemaphores;

    // binary semaphore signaled when the rendering is finish
    //  - signaling by vkQueueSubmit2
    //  - waiting   by present
    std::vector<VkSemaphore> mRenderFinishSemaphores;

    VkSemaphore mImageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore mRenderFinishSemaphor{VK_NULL_HANDLE};
    uint32_t    mCurrentSemaphoreIndex{};

    uint32_t mCurrentBackImageIndex{};
};
