#pragma once
#include "../vulkan/vulkan.h"
#include <span>
#include <vector>

class SpirvReflection {
public:
    SpirvReflection() = default;
    ~SpirvReflection() = default;

    void reflect(std::span<const uint32_t> spirv);

    VkShaderStageFlagBits getShaderStage() const {
        return mShaderStage;
    }

    const VkPushConstantRange& getPushConstantRange() const {
        return mPushConstantRange;
    }

    const std::vector<VkDescriptorSetLayoutBinding>& getDescriptorSetLayoutBinding() const {
        return mDescriptorSetLayoutBinding;
    }
private:
    VkShaderStageFlagBits mShaderStage{};
    VkPushConstantRange   mPushConstantRange{};
    std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBinding;
};
