#pragma once
#include "vulkan.h"

#include <filesystem>
#include <memory>

class VulkanTexture;
using VulkanTexturePtr = std::shared_ptr<VulkanTexture>;

/// @brief
struct VulkanTextureCubeMapCreateInfo {
    std::string name;
    uint32_t    width  = 1;
    uint32_t    height = 1;
    VkFormat    format = VK_FORMAT_R8G8B8_UNORM;
};

/// @brief
struct VulkanTexture2DCreateInfo {
    std::string name;
    uint32_t    width  = 1;
    uint32_t    height = 1;
    uint32_t    mipmap = 1;
    VkFormat    format = VK_FORMAT_R8G8B8_UNORM;
};

struct VulkanTextureDepthCreateInfo {
    std::string name;
    uint32_t    width  = 1;
    uint32_t    height = 1;
};

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

    /// @brief Create a cube map from 6 images files.
    ///        All image must be the same size.
    /// @param paths Array of 6 paths.
    /// @param sRGB Indicate if the cubemap will use sRGB color.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateCubeMap(std::filesystem::path paths[6], bool sRGB = true);

    /// @brief Create a cube map texture.
    /// @param createInfo Texture creation paramater.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateCubeMap(const VulkanTextureCubeMapCreateInfo& createInfo);

    /// @brief Create a texture from memmery.
    /// @param createInfo Texture creation paramater.
    /// @param data       The initial data of the texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr Create(const VulkanTexture2DCreateInfo& createInfo, const void* data);

    /// @brief Create a texture with depth format.
    /// @param width  The width of the texture.
    /// @param height The height of the texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateDepthBuffer(uint32_t width, uint32_t height);

    /// @brief Helper fucniton to create a 1x1 RGBA white texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateWhiteTexture();

    /// @brief Helper fucniton to create a 1x1 RGBA black texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateBlackTexture();

    /// @brief Helper fucniton to create a 2x2 RGBA white and black checkboard texture.
    /// @return The Vulkan texture.
    static VulkanTexturePtr CreateCheckBoard();

    ~VulkanTexture();

    VkImage     getImage() const { return mImage; }
    VkImageView getImageView() const { return mView; }
    VkSampler   getSampler() const { return mSampler; }

    VulkanTexture() = default; // tempo

    VulkanTexture(const VulkanTexture2DCreateInfo& createInfo);
    VulkanTexture(const VulkanTextureCubeMapCreateInfo& createInfo);
    VulkanTexture(const VulkanTextureDepthCreateInfo& createInfo);
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
