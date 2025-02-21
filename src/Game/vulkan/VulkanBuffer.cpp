#include "VulkanBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

VulkanBufferPtr VulkanBuffer::Create(const VulkanBufferCreateInfo& createInfo) {
    return std::make_shared<VulkanBuffer>(createInfo);
}

VulkanBufferPtr VulkanBuffer::CreateStagingBuffer(uint64_t sizeInByte, const char* name) {
    VulkanBufferCreateInfo createInfo{};
    createInfo.name = name;
    createInfo.sizeInByte = sizeInByte;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    return VulkanBuffer::Create(createInfo);
}

VulkanBuffer::VulkanBuffer(const VulkanBufferCreateInfo& createInfo) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size  = createInfo.sizeInByte;
    bufferCreateInfo.usage = createInfo.usage;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = VMA_MEMORY_USAGE_UNKNOWN;
    allocInfo.requiredFlags           = createInfo.memoryProperty;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateBuffer(VulkanContext::getVmaAllocator(), &bufferCreateInfo, &allocInfo,
                             &mBuffer, &mAllocation, nullptr /*allocationInfo*/));

    VulkanContext::setDebugObjectName((uint64_t)mBuffer, VK_OBJECT_TYPE_BUFFER,
                                      createInfo.name.c_str());
}

VulkanBuffer::~VulkanBuffer() {
    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), mBuffer, mAllocation);
}

void VulkanBuffer::writeData(const void* data, uint64_t size, uint64_t offset) {
    void* mapped{};
    vmaMapMemory(VulkanContext::getVmaAllocator(), mAllocation, &mapped);
    if(mapped) {
        memcpy(mapped, (uint8_t*)data + offset, size);
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mAllocation);
    }
}

void* VulkanBuffer::map() {
    void* data{};
    vmaMapMemory(VulkanContext::getVmaAllocator(), mAllocation, &data);
    return data;
}

void VulkanBuffer::unmap() { vmaUnmapMemory(VulkanContext::getVmaAllocator(), mAllocation); }
