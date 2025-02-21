#pragma once
#include "vulkan.h"

#include <memory>
#include <span>
#include <string>

class VulkanBuffer;
using VulkanBufferPtr = std::shared_ptr<VulkanBuffer>;

struct VulkanBufferCreateInfo {
    std::string           name;
    uint64_t              sizeInByte;
    VkBufferUsageFlags    usage;
    VkMemoryPropertyFlags memoryProperty;
};

/// @brief
class VulkanBuffer {
public:
    /// @brief
    /// @param createInfo
    /// @return
    [[nodiscard]] static VulkanBufferPtr Create(const VulkanBufferCreateInfo& createInfo);
    [[nodiscard]] static VulkanBufferPtr CreateStagingBuffer(uint64_t sizeInByte, const char* name = nullptr);

    /// @brief
    /// @param createInfo
    VulkanBuffer(const VulkanBufferCreateInfo& createInfo);

    /// @brief Destroy vulkan ressources.
    ~VulkanBuffer();

    /// @brief Return the vulkan buffer..
    [[nodiscard]] VkBuffer getBuffer() const { return mBuffer; };

    /// @brief Return the size of the buffer in bytes.
    [[nodiscard]] uint64_t getSizeInByte() const { return mSiizeInByte; };

    /// @brief Write data into the buffer.
    /// @param data   The data to write.
    /// @param size   The size of the data.
    /// @param offset The offset where to write the data inside the buffer.
    void writeData(const void* data, uint64_t size, uint64_t offset = 0);

    /// @brief
    /// @return
    [[nodiscard]] void* map();

    /// @brief
    void unmap();

private:
    VkBuffer      mBuffer{VK_NULL_HANDLE};
    VmaAllocation mAllocation{VK_NULL_HANDLE};
    uint64_t      mSiizeInByte{0};
};
