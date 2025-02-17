#pragma once
#include "VulkanUtils.h"
#include "vulkan.h"

#include <memory>
#include <string>

class VulkanGraphicPipeline;
class VulkanShaderProgram;
using VulkanGraphicPipelinePtr = std::shared_ptr<VulkanGraphicPipeline>;

struct VulkanPipelineCreateInfo {
    std::string name;
};

/// @brief
class VulkanGraphicPipeline {
public:
    /// @brief
    /// @return
    static VulkanGraphicPipelinePtr Create(std::shared_ptr<VulkanShaderProgram> shader,
                                           bool enableDepthTest = false,
                                           bool vertexLayout    = false,
                                           bool cull            = false);

    /// @brief
    VulkanGraphicPipeline() = default;
    ~VulkanGraphicPipeline();

    VkPipeline       getPipeline() const { return mPipeline; }
    VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }
    const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const { return mDescriptorSetLayout; }

private:
    VkGraphicsPipelineCreateInfo       mCreateInfo{};
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayout;
    std::vector<VkPushConstantRange>   mPushConstantRanges;

    VkPipeline       mPipeline{};
    VkPipelineLayout mPipelineLayout{};
};
