#pragma once
#include "vulkan.h"

#include <span>
#include <vector>

struct Shader {
    VkShaderModule                  shaderModule{};
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    VkPushConstantRange             pushConstantRange{};
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding;
};
struct GraphicPipeline {
    VkPipeline       pipeline{};
    VkPipelineLayout pipelineLayout{};
};

struct Texture {
    VkImage       image{VK_NULL_HANDLE};
    VkImageView   view{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
};

namespace VulkanContext {

bool Initialize();
void Shutdown();

VkInstance       getIntance();
VkPhysicalDevice getPhycalDevice();
VkDevice         getDevice();
uint32_t         getGraphicQueueFamilyIndex();
VkQueue          getGraphicQueue();

bool isLayerSupported();
bool isInstanceExtensionSupported();
bool isDeviceExtensionSupported();

[[nodiscard]] Shader           createShaderModule(std::span<const uint32_t> spirv);
[[nodiscard]] VkPipelineLayout createPipelineLayout(uint32_t               setLayoutCount,
                                                    VkDescriptorSetLayout* descriptorSetLayout,
                                                    uint32_t               rangeCount,
                                                    VkPushConstantRange*   ranges);
[[nodiscard]] VkPipelineLayout createPipelineLayout(Shader vert, Shader frag);
[[nodiscard]] GraphicPipeline  createGraphicPipeline(Shader vert, Shader frag);

[[nodiscard]] Texture createTexture() noexcept;

} // namespace VulkanContext
