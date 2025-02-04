#pragma once
#include <span>
#include "../vulkan/vulkan.h"

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

private:
    VkShaderStageFlagBits mShaderStage{};
    VkPushConstantRange   mPushConstantRange{};
};
