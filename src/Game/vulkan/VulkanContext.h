#pragma once
#include "vulkan.h"

#include <span>
#include <vector>

struct Shader {
    VkShaderModule                            shaderModule{};
    VkPipelineShaderStageCreateInfo           stageCreateInfo{};
    VkPushConstantRange                       pushConstantRange{};
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding;
};
struct GraphicPipeline {
    VkPipeline                         pipeline{};
    VkPipelineLayout                   pipelineLayout{};
    std::vector<VkDescriptorSetLayout> descriptorSetLayout;
    std::vector<VkPushConstantRange>   pushConstantRanges;

    void destroy();
};
struct Buffer {
    VkBuffer      buffer{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    uint64_t      sizeInByte{0};
};
struct Texture {
    VkImage       image{VK_NULL_HANDLE};
    VkImageView   view{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkSampler     sampler{VK_NULL_HANDLE};
    uint32_t width;
    uint32_t height;
};

namespace VulkanContext {

bool Initialize();
void Shutdown();

VkInstance       getIntance();
VkPhysicalDevice getPhycalDevice();
VkDevice         getDevice();
uint32_t         getGraphicQueueFamilyIndex();
VkQueue          getGraphicQueue();
VmaAllocator     getVmaAllocator();

bool isLayerSupported();
bool isInstanceExtensionSupported();
bool isDeviceExtensionSupported();

[[nodiscard]] VkCommandBuffer beginSingleTimeCommands();
void                          endSingleTimeCommands(VkCommandBuffer commandBuffer);
void                          copyBufferToImage(
                             VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

[[nodiscard]] Shader           createShaderModule(std::span<const uint32_t> spirv);
[[nodiscard]] VkPipelineLayout createPipelineLayout(uint32_t               setLayoutCount,
                                                    VkDescriptorSetLayout* descriptorSetLayout,
                                                    uint32_t               rangeCount,
                                                    VkPushConstantRange*   ranges);
[[nodiscard]] VkPipelineLayout createPipelineLayout(Shader vert, Shader frag);
[[nodiscard]] GraphicPipeline  createGraphicPipeline(Shader vert, Shader frag);

[[nodiscard]] Buffer  createBuffer(VkBufferUsageFlags    usageFlags,
                                   uint64_t              sizeInByte,
                                   VkMemoryPropertyFlags memoryPropertyFlags) noexcept;
[[nodiscard]] Texture createTexture(uint32_t width, uint32_t height, VkFormat format) noexcept;

} // namespace VulkanContext
