#pragma once
#include "vulkan.h"

class VulkanDescriptorPool {
public:
    VulkanDescriptorPool() = default;
    ~VulkanDescriptorPool() = default;

    VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
    VulkanDescriptorPool(VulkanDescriptorPool&&) = delete;

    VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;
    VulkanDescriptorPool& operator=(VulkanDescriptorPool&&) = delete;

    void init();
    void destroy();
    [[nodiscard]] VkDescriptorSet allocate(VkDescriptorSetLayout setLayout);
private:
    VkDescriptorPool mPool{VK_NULL_HANDLE};
};
