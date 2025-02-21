#pragma once
#include "vulkan.h"
#include "VulkanShaderProgram.h"

#include <span>
#include <string>
#include <vector>
#include <map>

struct Shader {
    VkShaderModule                            shaderModule{};
    VkPipelineShaderStageCreateInfo           stageCreateInfo{};
    VkPushConstantRange                       pushConstantRange{};
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> descriptorSetLayoutBinding;
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

// Debugging
void CmdBeginsLabel(VkCommandBuffer cmd, std::string_view label);
void CmdEndLabel(VkCommandBuffer cmd);
void CmdInsertLabel(VkCommandBuffer cmd, std::string_view label);
void setDebugObjectName(uint64_t objectHandle, VkObjectType objectType, std::string name);

template<class T>
void setDebugObjectName(uint64_t objectHandle, VkObjectType objectType, std::string name) {

}

} // namespace VulkanContext
