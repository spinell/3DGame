#pragma once
#include "../vulkan/vulkan.h"
#include <span>
#include <vector>
#include <map>

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

    const std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>>& getDescriptorSetLayoutBinding() const {
        return mDescriptorSetLayoutBinding;
    }
private:
    VkShaderStageFlagBits mShaderStage{};
    VkPushConstantRange   mPushConstantRange{};
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> mDescriptorSetLayoutBinding;
};
