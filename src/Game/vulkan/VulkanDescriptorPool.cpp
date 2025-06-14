#include "VulkanDescriptorPool.h"

#include "VulkanContext.h"

#include <Engine/Log.h>

#include <array>

void VulkanDescriptorPool::init() {
    VkDescriptorPoolInlineUniformBlockCreateInfo inlineUniformBlockCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO,
        .pNext = nullptr,
        .maxInlineUniformBlockBindings = 10 // number of inline uniform block bindings to allocate
    };

    std::array<VkDescriptorPoolSize, 7> poolSize = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 100},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
        // when inline uniforn block is used, we need to include
        // VkDescriptorPoolInlineUniformBlockCreateInfo in VkDescriptorPoolCreateInfo.pNext
        // Note: for  VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, descriptorCount is
        //       the size in byte of the uniform data capacity.
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 100}};

    VkDescriptorPoolCreateFlags flags = 0;
    // pecifies that descriptor sets can return their individual allocations to the pool
    // all of vkAllocateDescriptorSets, vkFreeDescriptorSets, and vkResetDescriptorPool are allowed.
    // Otherwise, descriptor sets allocated from the pool must not be individually freed back to the
    // pool, i.e. only vkAllocateDescriptorSets and vkResetDescriptorPool are allowed.
    // flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    // VK_VERSION_1_2, specifies that descriptor sets allocated from this pool can include bindings
    // with the VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT bit set. It is valid to allocate
    // descriptor sets that have bindings that do not set the
    // VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT bit from a pool that has
    // VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT set.
    // flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    // VK_EXT_mutable_descriptor_type
    // specifies that this descriptor pool and the descriptor sets allocated from it reside entirely
    // in host memory and cannot be bound.
    // flags |= VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;

    // VK_NV_descriptor_pool_overallocation
    // specifies that the implementation should allow the application to allocate more than
    // VkDescriptorPoolCreateInfo::maxSets descriptor set objects from the descriptor pool as
    // available resources allow.
    // flags |= VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV;

    // VK_NV_descriptor_pool_overallocation
    // flags |= VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_POOLS_BIT_NV;
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = &inlineUniformBlockCreateInfo,
        .flags         = flags,
        .maxSets       = 100,
        .poolSizeCount = poolSize.size(),
        .pPoolSizes    = poolSize.data()};
    vkCreateDescriptorPool(VulkanContext::getDevice(), &descriptorPoolCreateInfo, nullptr, &mPool);
}

void VulkanDescriptorPool::destroy() {
    vkDestroyDescriptorPool(VulkanContext::getDevice(), mPool, nullptr);
}

VkDescriptorSet VulkanDescriptorPool::allocate(VkDescriptorSetLayout setLayout) {
    VkDescriptorSetAllocateInfo allocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = mPool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &setLayout};
    VkDescriptorSet descriptorSet{};
    auto            result =
        vkAllocateDescriptorSets(VulkanContext::getDevice(), &allocateInfo, &descriptorSet);
    // VK_CHECK(result);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
        ENGINE_CORE_WARNING("VK_ERROR_OUT_OF_POOL_MEMORY");
        /// TODO Create a new pool ?
    }
    if (result == VK_ERROR_FRAGMENTED_POOL) {
        ENGINE_CORE_WARNING("VK_ERROR_FRAGMENTED_POOL");
        /// TODO Create a new pool ?
    }
    return descriptorSet;
}
