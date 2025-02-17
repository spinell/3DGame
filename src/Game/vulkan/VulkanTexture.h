#pragma once
#include "vulkan.h"

#include <filesystem>
#include <memory>

class VulkanTexture;
using VulkanTexturePtr = std::shared_ptr<VulkanTexture>;

/// @brief
class VulkanTexture {
public:
    /// @brief Create a Vulkan texture from a file.
    /// @param path           The path of the file.
    /// @param sRGB           Should the texture use sRGB format.
    /// @param generateMipmap Should all mipmap been generated.
    /// @return The Vulkan texture.
    static VulkanTexturePtr Create(std::filesystem::path path,
                                   bool                  sRGB           = true,
                                   bool                  generateMipmap = true);

    /// @brief Create a texture from memmery.
    /// @param width  The width of the texture.
    /// @param height The height of the texture.
    /// @param foramt The pixel format of the texture.
    /// @param data   The initial data of the texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr Create(unsigned width, unsigned height, VkFormat foramt, const void* data);

    /// @brief Create a 1x1 white texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateWhiteTexture();

    /// @brief Create a 1x1 black texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateBlackTexture();

    /// @brief Create checkboard texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateCheckBoard();

    ~VulkanTexture();

    VkImage     getImage() const { return mImage; }
    VkImageView getImageView() const { return mView; }
    VkSampler   getSampler() const { return mSampler; }

private:
    /// @brief Width of the texture.
    uint32_t mWidth{};

    /// @brief Height of the texture.
    uint32_t mHeight{};

    /// @brief The vulkan image handle of the texture.
    VkImage mImage{VK_NULL_HANDLE};

    /// @brief The Vulkan image view for the texture.
    VkImageView mView{VK_NULL_HANDLE};

    VmaAllocation mAllocation{VK_NULL_HANDLE};
    VkSampler     mSampler{VK_NULL_HANDLE};

    /// Path of the texture if it was loaded from a file.
    std::filesystem::path mPath;
};
