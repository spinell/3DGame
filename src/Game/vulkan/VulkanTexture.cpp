#include "VulkanTexture.h"

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

} // namespace

VulkanTexturePtr VulkanTexture::Create(std::filesystem::path path, bool sRGB, bool generateMipmap) {
    VulkanTexturePtr vulkanTexture = std::make_shared<VulkanTexture>();

    const std::string pathString = path.string();

    int   width, height, channels;
    auto* data = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
    if (data) {
        ENGINE_INFO("Loading {}", pathString);

        //
        // create the texture
        //
        uint32_t          mipLevels = 1;
        VkFormat          format    = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        VkImageUsageFlags usage     = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (generateMipmap) {
            // This is required to allow generating mipmap with vkCmdBlitImage.
            // vkCmdBlitImage is a transfert operation.
            usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        }
        Texture texture = VulkanContext::createTexture(width, height, format, mipLevels, usage);
        vulkanTexture->mImage      = texture.image;
        vulkanTexture->mView       = texture.view;
        vulkanTexture->mSampler    = texture.sampler;
        vulkanTexture->mAllocation = texture.allocation;
        vulkanTexture->mWidth      = width;
        vulkanTexture->mHeight     = height;
        vulkanTexture->mPath       = path;

        VulkanContext::setDebugObjectName((uint64_t)texture.image, VK_OBJECT_TYPE_IMAGE,
                                          path.string().c_str());
        VulkanContext::setDebugObjectName((uint64_t)texture.view, VK_OBJECT_TYPE_IMAGE_VIEW,
                                          path.string().c_str());
        VulkanContext::setDebugObjectName((uint64_t)texture.sampler, VK_OBJECT_TYPE_SAMPLER,
                                          path.string().c_str());

        //
        // create staging buffer
        //
        VkDeviceSize imageSize     = texture.width * texture.height * 4;
        auto         stagingBuffer = VulkanContext::createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        //
        // Copy data into staging buffer
        //
        void* ptr{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation, &ptr);
        std::memcpy(ptr, data, imageSize);
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation);

        stbi_image_free(data);

        //
        // Copy the staging buffer into the texture
        //
        VkCommandBuffer cmd = VulkanContext::beginSingleTimeCommands();

        VulkanUtils::transitionImageLayout(
            cmd, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE_KHR, VK_ACCESS_2_NONE_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT, mipLevels);

        VulkanContext::copyBufferToImage(cmd, stagingBuffer.buffer, texture.image,
                                         static_cast<uint32_t>(texture.width),
                                         static_cast<uint32_t>(texture.height));

        // Generate mipmap
        if (generateMipmap) {
            VkImageMemoryBarrier barrier{};
            barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image                           = texture.image;
            barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = 1;
            barrier.subresourceRange.levelCount     = 1;

            int32_t mipWidth  = texture.width;
            int32_t mipHeight = texture.height;
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

                vkCmdBlitImage(cmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
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
                cmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT, mipLevels);
        }

        VulkanContext::endSingleTimeCommands(cmd);

        vmaDestroyBuffer(VulkanContext::getVmaAllocator(), stagingBuffer.buffer,
                         stagingBuffer.allocation);
        return vulkanTexture;
    }

    ENGINE_ERROR("Failed to load {}", pathString);
    return vulkanTexture;
}

VulkanTexturePtr VulkanTexture::Create(unsigned width, unsigned height, VkFormat foramt, const void* data) {
    VulkanTexturePtr vulkanTexture = std::make_shared<VulkanTexture>();

    //
    // Create the texture
    //
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    Texture texture = VulkanContext::createTexture(width, height, foramt, 1, usage);
    vulkanTexture->mImage      = texture.image;
    vulkanTexture->mView       = texture.view;
    vulkanTexture->mSampler    = texture.sampler;
    vulkanTexture->mAllocation = texture.allocation;
    vulkanTexture->mWidth      = width;
    vulkanTexture->mHeight     = height;

    if(data) {
        //
        // create staging buffer
        //
        const VkDeviceSize imageSize = width * height * 4; // FIXME: Assume RGBA format.
        const VkMemoryPropertyFlags memoryPropertyFlags =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        auto stagingBuffer = VulkanContext::createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize,
                                                        memoryPropertyFlags);

        //
        // Copy pixels into staging buffer
        //
        void* ptr{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation, &ptr);
        std::memcpy(ptr, data, imageSize);
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation);

        //
        // Copy staging buffer into the image
        //
        VkCommandBuffer cmd = VulkanContext::beginSingleTimeCommands();
        copyBufferToImage(cmd, stagingBuffer.buffer, texture.image, texture.width, texture.height);
        VulkanContext::endSingleTimeCommands(cmd);

        // delete staging buffer
        vmaDestroyBuffer(VulkanContext::getVmaAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
    }

    return vulkanTexture;
}

VulkanTexturePtr VulkanTexture::CreateWhiteTexture() {
    const uint32_t color = 0xFFFFFFFF;
    auto texture = Create(1, 1, VK_FORMAT_R8G8B8A8_UNORM, &color);

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
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr, &texture->mSampler));

    return texture;
}

VulkanTexturePtr VulkanTexture::CreateBlackTexture() {
    const uint32_t color = 0xFF000000;
    auto texture = Create(1, 1, VK_FORMAT_R8G8B8A8_UNORM, &color);

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
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr, &texture->mSampler));

    return texture;
}

VulkanTexturePtr VulkanTexture::CreateCheckBoard() {
    const uint32_t color[4] = {
        0xFF000000, 0xFFFFFFFF, // black, white
        0xFFFFFFFF, 0xFF000000  // white, black
    };
    auto texture = Create(2, 2, VK_FORMAT_R8G8B8A8_UNORM, &color);

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
    VK_CHECK(vkCreateSampler(VulkanContext::getDevice(), &samplerCreateInfo, nullptr, &texture->mSampler));

    return texture;
}

VulkanTexture::~VulkanTexture() {
    ENGINE_CORE_TRACE("Deleting texture: {}", mPath.string());
    vmaDestroyImage(VulkanContext::getVmaAllocator(), mImage, mAllocation);
    vkDestroyImageView(VulkanContext::getDevice(), mView, nullptr);
    vkDestroySampler(VulkanContext::getDevice(), mSampler, nullptr);
}
