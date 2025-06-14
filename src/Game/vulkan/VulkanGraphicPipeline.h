#pragma once
#include "VulkanUtils.h"
#include "vulkan.h"

#include <memory>
#include <string>

class VulkanGraphicPipeline;
class VulkanShaderProgram;
using VulkanGraphicPipelinePtr = std::shared_ptr<VulkanGraphicPipeline>;

struct VulkanGraphicPipelineCreateInfo {
    std::string name;
    std::shared_ptr<VulkanShaderProgram> shader;
    VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    bool            enableDepthTest = true;
    VkCompareOp     depthCompareOp  = VK_COMPARE_OP_LESS;
    std::vector<VkVertexInputAttributeDescription> vertexInput = {};
    uint32_t vertexStride = 0;
};

/// @brief
class VulkanGraphicPipeline {
public:
    /// @brief
    /// @param createInfo
    /// @return
    static VulkanGraphicPipelinePtr Create(const VulkanGraphicPipelineCreateInfo& createInfo);

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
