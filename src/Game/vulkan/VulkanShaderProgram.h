#pragma once
#include "Vulkan/vulkan.h"

#include <filesystem>
#include <initializer_list>
#include <span>
#include <string>

struct ShaderBuffer {
    std::string name;
    uint32_t    size = 0;
};

class VulkanShaderProgram {
public:
    /// @brief
    /// @param spirv
    /// @return
    static [[nodiscard]] std::shared_ptr<VulkanShaderProgram> CreateFromSpirv(
        std::vector<std::vector<uint32_t>> spirv);

    /// @brief Create a shader from a list a multiple spiv binary data.
    /// @param spirv
    /// @return
    static [[nodiscard]] std::shared_ptr<VulkanShaderProgram> CreateFromSpirv(
        std::initializer_list<std::span<const uint32_t>> spirv);

    /// @brief Create a shader from a list a multiple spiv binary files.
    /// @param paths
    /// @return
    static [[nodiscard]] std::shared_ptr<VulkanShaderProgram> CreateFromSpirv(
        std::initializer_list<std::filesystem::path> paths);

    /// @brief
    /// @return
    static [[nodiscard]] std::shared_ptr<VulkanShaderProgram> CreateFromString(std::string source);

    /// @brief
    /// @param paths
    /// @return
    static [[nodiscard]] std::shared_ptr<VulkanShaderProgram> CreateFromFile(
        std::vector<std::filesystem::path> paths);

    VulkanShaderProgram() = default;
    ~VulkanShaderProgram();
    VulkanShaderProgram(const VulkanShaderProgram&)            = delete;
    VulkanShaderProgram(VulkanShaderProgram&&)                 = delete;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&)      = delete;

    [[nodiscard]] bool hasDescriptorBinding(uint32_t setIdx, uint32_t bindingIdx) const;
    [[nodiscard]] bool hasDescriptorSet(uint32_t setIdx) const;
    [[nodiscard]] bool hasPushConstant() const { return mPushConstantRanges.size(); };
    [[nodiscard]] bool hasShaderStage(VkShaderStageFlagBits stage) const { return mStages & stage; }
    [[nodiscard]] VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }
    [[nodiscard]] const std::vector<VkPipelineShaderStageCreateInfo>& getShaderShages() const {
        return mPipelineShaderStageCreateInfos;
    }
    [[nodiscard]] const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const {
        return mDescriptorSetLayout;
    }

private:
    /// @brief Bit mask of all shader stages
    VkShaderStageFlags mStages{};

    /// @brief Use for creating the vulkan pipeline.
    std::vector<VkPipelineShaderStageCreateInfo> mPipelineShaderStageCreateInfos;

    // Reflection data.
    std::vector<VkPushConstantRange> mPushConstantRanges{};

    VkPipelineLayout                   mPipelineLayout{};
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayout{};
};
