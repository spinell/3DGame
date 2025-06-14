#include "VulkanTexture.h"

#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <Engine/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace {

/// @brief
/// @param cmd
/// @param buffer
/// @param image
/// @param width
/// @param height
void copyBufferToImage(
    VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VulkanUtils::transitionImageLayout(
        cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_2_NONE_KHR, VK_ACCESS_2_NONE_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, 1);

    VulkanContext::copyBufferToImage(cmd, buffer, image, width, height);

    VulkanUtils::transitionImageLayout(
        cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1);
}

void copyBufferToImage2(VkCommandBuffer commandBuffer,
                        VkBuffer        buffer,
                        VkImage         image,
                        uint32_t        width,
                        uint32_t        height,
                        uint32_t        layer) {
    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
}

} // namespace

VulkanTexturePtr VulkanTexture::Create(std::filesystem::path path, bool sRGB, bool generateMipmap) {
    VulkanTexturePtr texture;

    const std::string pathString = path.string();

    int   width, height, channels;
    auto* data = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
    if (data) {
        ENGINE_INFO("Loading {}", pathString);

        //
        // create the texture
        //
        uint32_t mipLevels = 1;
        VkFormat format    = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        // VkImageUsageFlags usage     = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        // VK_IMAGE_USAGE_SAMPLED_BIT;
        if (generateMipmap) {
            // This is required to allow generating mipmap with vkCmdBlitImage.
            // vkCmdBlitImage is a transfert operation.
            // usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        }
        VulkanTexture2DCreateInfo createInfo{};
        createInfo.name   = path.string();
        createInfo.width  = width;
        createInfo.height = height;
        createInfo.format = format;
        createInfo.mipmap = mipLevels;
        texture           = std::make_shared<VulkanTexture>(createInfo);
        texture->mPath    = path;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(VulkanContext::getPhycalDevice(), &properties);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext;
        samplerCreateInfo.flags;
        // Magnification concerns the oversampling
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        // minification concerns undersampling
        samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias              = 0.0f;
        samplerCreateInfo.anisotropyEnable        = VK_TRUE;
        samplerCreateInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerCreateInfo.compareEnable           = VK_FALSE;
        samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod                  = 0;
        samplerCreateInfo.maxLod                  = mipLevels;
        samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                                 &texture->mSampler));

        //
        // create staging buffer
        //
        VkDeviceSize imageSize     = width * height * 4;
        auto         stagingBuffer = VulkanBuffer::CreateStagingBuffer(imageSize, "Staging");
        stagingBuffer->writeData(data, imageSize);

        stbi_image_free(data);

        //
        // Copy the staging buffer into the texture
        //
        VkCommandBuffer cmd = VulkanContext::beginSingleTimeCommands();

        VulkanUtils::transitionImageLayout(
            cmd, texture->mImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE_KHR, VK_ACCESS_2_NONE_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT, mipLevels);

        VulkanContext::copyBufferToImage(cmd, stagingBuffer->getBuffer(), texture->mImage,
                                         texture->mWidth, texture->mHeight);

        // Generate mipmap
        if (generateMipmap) {
            VkImageMemoryBarrier barrier{};
            barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image                           = texture->mImage;
            barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = 1;
            barrier.subresourceRange.levelCount     = 1;

            int32_t mipWidth  = texture->mWidth;
            int32_t mipHeight = texture->mHeight;
            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                     &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0]                 = {0, 0, 0};
                blit.srcOffsets[1]                 = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel       = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount     = 1;
                blit.dstOffsets[0]                 = {0, 0, 0};
                blit.dstOffsets[1]                 = {mipWidth > 1 ? mipWidth / 2 : 1,
                                      mipHeight > 1 ? mipHeight / 2 : 1, 1};
                blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel       = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount     = 1;

                vkCmdBlitImage(cmd, texture->mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               texture->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                               VK_FILTER_LINEAR);

                barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                                     nullptr, 1, &barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,
                                 1, &barrier);

        } else {
            VulkanUtils::transitionImageLayout(
                cmd, texture->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT, mipLevels);
        }

        VulkanContext::endSingleTimeCommands(cmd);

        return texture;
    }

    ENGINE_ERROR("Failed to load {}", pathString);
    return texture;
}

VulkanTexturePtr VulkanTexture::CreateCubeMap(std::filesystem::path paths[6], bool sRGB) {
    VulkanTexturePtr vulkanTexture;

    bool imageAreValide = true;

    int width      = 0;
    int height     = 0;
    int nbChannels = 0;
    for (unsigned i = 0; i < 6; i++) {
        const std::string path = paths[i].string();
        if (!stbi_info(path.c_str(), &width, &height, &nbChannels)) {
            ENGINE_ERROR("Failed to query image info fo {}, reason {}", path,
                         stbi_failure_reason());
            imageAreValide = false;
        }
    }

    stbi_uc* data[6] = {};
    if (imageAreValide) {
        for (unsigned i = 0; i < 6; i++) {
            const std::string path = paths[i].string();
            data[i]                = stbi_load(path.c_str(), &width, &height, &nbChannels, 4);
            if (!data[i]) {
                ENGINE_ERROR("Failed to query image info fo {}, reason {}", path,
                             stbi_failure_reason());
                imageAreValide = false;
            }
        }
    }

    if (imageAreValide) {
        //
        // Create the cube map
        //
        VulkanTextureCubeMapCreateInfo cubeMapCreateInfo;
        cubeMapCreateInfo.width  = width;
        cubeMapCreateInfo.height = height;
        cubeMapCreateInfo.format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        vulkanTexture            = VulkanTexture::CreateCubeMap(cubeMapCreateInfo);

        VkCommandBuffer cmd = VulkanContext::beginSingleTimeCommands();
        VulkanUtils::transitionImageLayout(cmd, vulkanTexture->mImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_PIPELINE_STAGE_2_NONE_KHR, VK_ACCESS_2_NONE_KHR,
                                           VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                           VK_ACCESS_2_TRANSFER_WRITE_BIT, 1, 6);

        VulkanBufferPtr    stagingBuffers[6];
        const VkDeviceSize imageSize = width * height * 4;
        for (unsigned i = 0; i < 6; i++) {
            stagingBuffers[i] = VulkanBuffer::CreateStagingBuffer(imageSize, "Staging");
            stagingBuffers[i]->writeData(data[i], imageSize);

            copyBufferToImage2(cmd, stagingBuffers[i]->getBuffer(), vulkanTexture->mImage, width,
                               height, i);
        }

        VulkanUtils::transitionImageLayout(
            cmd, vulkanTexture->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT, 1, 6);

        VulkanContext::endSingleTimeCommands(cmd);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext;
        samplerCreateInfo.flags;
        samplerCreateInfo.magFilter               = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter               = VK_FILTER_NEAREST;
        samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias              = 0.0f;
        samplerCreateInfo.anisotropyEnable        = VK_FALSE;
        samplerCreateInfo.maxAnisotropy           = 0;
        samplerCreateInfo.compareEnable           = VK_FALSE;
        samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod                  = 0;
        samplerCreateInfo.maxLod                  = 0;
        samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                                 &vulkanTexture->mSampler));
    }

    // clean up
    for (unsigned i = 0; i < 6; i++) {
        if (data[i]) {
            stbi_image_free(data[i]);
        }
    }

    return vulkanTexture;
}

VulkanTexturePtr VulkanTexture::CreateCubeMap(const VulkanTextureCubeMapCreateInfo& createInfo) {
    VulkanTexturePtr texture = std::make_shared<VulkanTexture>(createInfo);
    return texture;
}

VulkanTexturePtr VulkanTexture::Create(const VulkanTexture2DCreateInfo& createInfo,
                                       const void*                      data) {
    VulkanTexturePtr texture = std::make_shared<VulkanTexture>(createInfo);

    if (data) {
        //
        // create staging buffer
        //
        VkDeviceSize imageSize = createInfo.width * createInfo.height;
        if(createInfo.format == VK_FORMAT_R8G8B8A8_UNORM || createInfo.format == VK_FORMAT_R32_SFLOAT)
            imageSize *= 4; // FIXME: Assume RGBA format.

        auto stagingBuffer = VulkanBuffer::CreateStagingBuffer(imageSize, "Staging");
        stagingBuffer->writeData(data, imageSize);

        //
        // Copy staging buffer into the image
        //
        VkCommandBuffer cmd = VulkanContext::beginSingleTimeCommands();
        copyBufferToImage(cmd, stagingBuffer->getBuffer(), texture->mImage, texture->mWidth,
                          texture->mHeight);
        VulkanContext::endSingleTimeCommands(cmd);
    }

    return texture;
}

VulkanTexturePtr VulkanTexture::CreateDepthBuffer(uint32_t width, uint32_t height) {
    VulkanTextureDepthCreateInfo createInfo{};
    createInfo.name   = "DepthBuffer";
    createInfo.width  = width;
    createInfo.height = height;
    return std::make_shared<VulkanTexture>(createInfo);
}

VulkanTexturePtr VulkanTexture::CreateWhiteTexture() {
    const uint32_t            color = 0xFFFFFFFF;
    VulkanTexture2DCreateInfo createInfo{};
    createInfo.name   = "WhiteTexture";
    createInfo.width  = 1;
    createInfo.height = 1;
    createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    auto texture      = VulkanTexture::Create(createInfo, &color);

    // tempo
    vkDestroySampler(VulkanContext::getDevice(), texture->getSampler(), nullptr);
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext;
    samplerCreateInfo.flags;
    samplerCreateInfo.magFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.anisotropyEnable        = VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = 0;
    samplerCreateInfo.compareEnable           = VK_FALSE;
    samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod                  = 0;
    samplerCreateInfo.maxLod                  = 0;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                             &texture->mSampler));

    return texture;
}

VulkanTexturePtr VulkanTexture::CreateBlackTexture() {
    const uint32_t            color = 0xFF000000;
    VulkanTexture2DCreateInfo createInfo{};
    createInfo.name   = "BlackTexture";
    createInfo.width  = 1;
    createInfo.height = 1;
    createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    auto texture      = VulkanTexture::Create(createInfo, &color);

    // tempo
    vkDestroySampler(VulkanContext::getDevice(), texture->getSampler(), nullptr);
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext;
    samplerCreateInfo.flags;
    samplerCreateInfo.magFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.anisotropyEnable        = VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = 0;
    samplerCreateInfo.compareEnable           = VK_FALSE;
    samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod                  = 0;
    samplerCreateInfo.maxLod                  = 0;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                             &texture->mSampler));

    return texture;
}

VulkanTexturePtr VulkanTexture::CreateCheckBoard() {
    const uint32_t color[4] = {
        0xFF000000, 0xFFFFFFFF, // black, white
        0xFFFFFFFF, 0xFF000000  // white, black
    };
    VulkanTexture2DCreateInfo createInfo{};
    createInfo.name   = "CheckBoardTexture";
    createInfo.width  = 2;
    createInfo.height = 2;
    createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    auto texture      = VulkanTexture::Create(createInfo, &color);

    // tempo
    vkDestroySampler(VulkanContext::getDevice(), texture->getSampler(), nullptr);
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext;
    samplerCreateInfo.flags;
    samplerCreateInfo.magFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter               = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.anisotropyEnable        = VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = 0;
    samplerCreateInfo.compareEnable           = VK_FALSE;
    samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod                  = 0;
    samplerCreateInfo.maxLod                  = 0;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                             &texture->mSampler));

    return texture;
}

VulkanTexture::~VulkanTexture() {
    ENGINE_CORE_TRACE("Deleting texture: {}", mPath.string());
    vmaDestroyImage(VulkanContext::getVmaAllocator(), mImage, mAllocation);
    vkDestroyImageView(VulkanContext::getDevice(), mView, nullptr);
    vkDestroySampler(VulkanContext::getDevice(), mSampler, nullptr);
}

VulkanTexture::VulkanTexture(const VulkanTexture2DCreateInfo& createInfo)
    : mWidth(createInfo.width), mHeight(createInfo.height) {
    const VkSampleCountFlagBits nbSamples = VK_SAMPLE_COUNT_1_BIT;
    const VkExtent3D            extent    = {createInfo.width, createInfo.height, 1};
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    //
    // Create the image
    //
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                 = nullptr;
    imageCreateInfo.flags                 = 0;
    imageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format                = createInfo.format;
    imageCreateInfo.extent                = extent;
    imageCreateInfo.mipLevels             = createInfo.mipmap;
    imageCreateInfo.arrayLayers           = 1;
    imageCreateInfo.samples               = nbSamples;
    imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage                 = usage;
    imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices   = nullptr;
    imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = (VmaMemoryUsage)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.requiredFlags           = 0;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateImage(VulkanContext::getVmaAllocator(), &imageCreateInfo, &allocInfo, &mImage,
                            &mAllocation, nullptr /*allocationInfo*/));

    //
    // Create the image view
    //
    VkImageViewCreateInfo ivCreateInfo           = {};
    ivCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.pNext                           = nullptr;
    ivCreateInfo.flags                           = 0;
    ivCreateInfo.image                           = mImage;
    ivCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    ivCreateInfo.format                          = createInfo.format;
    ivCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    ivCreateInfo.subresourceRange.baseMipLevel   = 0;
    ivCreateInfo.subresourceRange.levelCount     = createInfo.mipmap;
    ivCreateInfo.subresourceRange.baseArrayLayer = 0;
    ivCreateInfo.subresourceRange.layerCount     = 1;
    VK_CHECK(vkCreateImageView(VulkanContext::getDevice(), &ivCreateInfo, nullptr, &mView));

    VulkanContext::setDebugObjectName((uint64_t)mImage, VK_OBJECT_TYPE_IMAGE,
                                      createInfo.name.c_str());
    VulkanContext::setDebugObjectName((uint64_t)mView, VK_OBJECT_TYPE_IMAGE_VIEW,
                                      createInfo.name.c_str());

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext;
    samplerCreateInfo.flags;
    samplerCreateInfo.magFilter               = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.anisotropyEnable        = VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = 0;
    samplerCreateInfo.compareEnable           = VK_FALSE;
    samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod                  = 0;
    samplerCreateInfo.maxLod                  = 0;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr,
                             &mSampler));
}

VulkanTexture::VulkanTexture(const VulkanTextureCubeMapCreateInfo& createInfo)
    : mWidth(createInfo.width), mHeight(createInfo.height) {
    const VkSampleCountFlagBits nbSamples = VK_SAMPLE_COUNT_1_BIT;
    const VkExtent3D            extent    = {createInfo.width, createInfo.height, 1};
    const uint32_t              mipLevels = 1;
    const VkFormat              format    = createInfo.format;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    //
    // Create the image
    //
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                 = nullptr;
    imageCreateInfo.flags                 = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format                = format;
    imageCreateInfo.extent                = extent;
    imageCreateInfo.mipLevels             = 1;
    imageCreateInfo.arrayLayers           = 6; // cubemap required 6 faces
    imageCreateInfo.samples               = nbSamples;
    imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage                 = usage;
    imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices   = nullptr;
    imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = (VmaMemoryUsage)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.requiredFlags           = 0;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateImage(VulkanContext::getVmaAllocator(), &imageCreateInfo, &allocInfo, &mImage,
                            &mAllocation, nullptr /*allocationInfo*/));

    //
    // Create the image view
    //
    VkImageViewCreateInfo ivCreateInfo           = {};
    ivCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.pNext                           = nullptr;
    ivCreateInfo.flags                           = 0;
    ivCreateInfo.image                           = mImage;
    ivCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
    ivCreateInfo.format                          = format;
    ivCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    ivCreateInfo.subresourceRange.baseMipLevel   = 0;
    ivCreateInfo.subresourceRange.levelCount     = mipLevels;
    ivCreateInfo.subresourceRange.baseArrayLayer = 0;
    ivCreateInfo.subresourceRange.layerCount     = 6; // cube map required 6 layers
    VK_CHECK(vkCreateImageView(VulkanContext::getDevice(), &ivCreateInfo, nullptr, &mView));

    VulkanContext::setDebugObjectName((uint64_t)mImage, VK_OBJECT_TYPE_IMAGE,
                                      createInfo.name.c_str());
    VulkanContext::setDebugObjectName((uint64_t)mView, VK_OBJECT_TYPE_IMAGE_VIEW,
                                      createInfo.name.c_str());
}

VulkanTexture::VulkanTexture(const VulkanTextureDepthCreateInfo& createInfo)
    : mWidth(createInfo.width), mHeight(createInfo.height) {
    const VkSampleCountFlagBits nbSamples = VK_SAMPLE_COUNT_1_BIT;
    const VkExtent3D            extent    = {createInfo.width, createInfo.height, 1};
    const VkImageUsageFlags     usage     = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    //
    // Create the image
    //
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                 = nullptr;
    imageCreateInfo.flags                 = 0;
    imageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format                = VK_FORMAT_D24_UNORM_S8_UINT;
    imageCreateInfo.extent                = extent;
    imageCreateInfo.mipLevels             = 1;
    imageCreateInfo.arrayLayers           = 1;
    imageCreateInfo.samples               = nbSamples;
    imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage                 = usage;
    imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices   = nullptr;
    imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = (VmaMemoryUsage)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.requiredFlags           = 0;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateImage(VulkanContext::getVmaAllocator(), &imageCreateInfo, &allocInfo, &mImage,
                            &mAllocation, nullptr /*allocationInfo*/));

    //
    // Create the image view
    //
    VkImageViewCreateInfo ivCreateInfo = {};
    ivCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.pNext                 = nullptr;
    ivCreateInfo.flags                 = 0;
    ivCreateInfo.image                 = mImage;
    ivCreateInfo.viewType              = VK_IMAGE_VIEW_TYPE_2D;
    ivCreateInfo.format                = VK_FORMAT_D24_UNORM_S8_UINT;
    ivCreateInfo.components.r          = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.g          = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.b          = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.a          = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    ivCreateInfo.subresourceRange.baseMipLevel   = 0;
    ivCreateInfo.subresourceRange.levelCount     = 1;
    ivCreateInfo.subresourceRange.baseArrayLayer = 0;
    ivCreateInfo.subresourceRange.layerCount     = 1;
    VK_CHECK(vkCreateImageView(VulkanContext::getDevice(), &ivCreateInfo, nullptr, &mView));

    VulkanContext::setDebugObjectName((uint64_t)mImage, VK_OBJECT_TYPE_IMAGE,
                                      createInfo.name.c_str());
    VulkanContext::setDebugObjectName((uint64_t)mView, VK_OBJECT_TYPE_IMAGE_VIEW,
                                      createInfo.name.c_str());
}
