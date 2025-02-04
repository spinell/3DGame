#pragma once
#include "vulkan.h"

#include <span>

struct Shader {
    VkShaderModule                  shaderModule{};
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
};
struct GraphicPipeline {
    VkPipeline       pipeline{};
    VkPipelineLayout pipelineLayout{};
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

[[nodiscard]] Shader           createShaderModule(std::span<const uint32_t> spirv,
                                                  VkShaderStageFlagBits     stage);
[[nodiscard]] VkPipelineLayout createPipelineLayout(uint32_t               setLayoutCount,
                                                    VkDescriptorSetLayout* descriptorSetLayout,
                                                    uint32_t               rangeCount,
                                                    VkPushConstantRange*   ranges);
[[nodiscard]] GraphicPipeline  createGraphicPipeline(Shader           vert,
                                                     Shader           frag,
                                                     VkPipelineLayout pipelineLayout);

} // namespace VulkanContext
