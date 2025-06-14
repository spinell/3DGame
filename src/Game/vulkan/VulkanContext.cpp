#include "VulkanContext.h"

#include "VulkanUtils.h"
#include "vk_mem_alloc.h"

#include "../spirv/SpirvReflection.h"

#include <Engine/Log.h>

#include <algorithm>
#include <array>
#include <vector>

VkBool32 VKAPI_PTR debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                 VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                 void*                                       pUserData) {
    const auto* msgSeverityStr  = string_VkDebugUtilsMessageSeverityFlagBitsEXT(messageSeverity);
    const auto  messageTypesStr = string_VkDebugUtilsMessageTypeFlagsEXT(messageTypes);

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        ENGINE_CORE_ERROR("{}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        ENGINE_CORE_WARNING("{}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        ENGINE_CORE_INFO("{}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        ENGINE_CORE_TRACE("{}", pCallbackData->pMessage);
    }

    return true;
}

namespace VulkanContext {
VkInstance               sInstance{VK_NULL_HANDLE};
VkPhysicalDevice         sPhysicalDevice{VK_NULL_HANDLE};
VkDevice                 sDevice{VK_NULL_HANDLE};
uint32_t                 sGraphicQueueFamilyIndex{0};
VkQueue                  sGraphicsQueue{VK_NULL_HANDLE};
VmaAllocator             sVmaAllocator{VK_NULL_HANDLE};
VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
VkCommandPool            sSingleTimeCommandPool{VK_NULL_HANDLE};

bool Initialize() {
    const uint32_t desiredVulkanVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    ENGINE_CORE_INFO("Initialisation vulkan ...");

    // If the vkGetInstanceProcAddr returns NULL for vkEnumerateInstanceVersion,
    // it is a Vulkan 1.0 implementation.
    if (!vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion")) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("Vulkan loader version found {}.{}.{}", 1, 0, 0);
        ENGINE_ERROR("Vulkan loader version required {}.{}.{}",
                     VK_VERSION_MAJOR(desiredVulkanVersion), VK_VERSION_MINOR(desiredVulkanVersion),
                     VK_VERSION_PATCH(desiredVulkanVersion));
        return false;
    }

    uint32_t vulkanLoaderVersion{};
    if (vkEnumerateInstanceVersion(&vulkanLoaderVersion) != VK_SUCCESS) {
        ENGINE_ERROR("Failed to get vulkan loader verson.");
        return false;
    }

    if (vulkanLoaderVersion < desiredVulkanVersion) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("Vulkan loader version found {}.{}.{}", VK_VERSION_MAJOR(vulkanLoaderVersion),
                     VK_VERSION_MINOR(vulkanLoaderVersion), VK_VERSION_PATCH(vulkanLoaderVersion));
        ENGINE_ERROR("Vulkan loader version required {}.{}.{}",
                     VK_VERSION_MAJOR(desiredVulkanVersion), VK_VERSION_MINOR(desiredVulkanVersion),
                     VK_VERSION_PATCH(desiredVulkanVersion));
        return false;
    }

    // =================================================
    //  Create vulkan instance
    // =================================================
    VkDebugUtilsMessengerCreateInfoEXT debugMsgCreateInfo{};
    debugMsgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMsgCreateInfo.pNext = nullptr;
    debugMsgCreateInfo.flags = 0;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugMsgCreateInfo.pfnUserCallback = debugCallback;
    debugMsgCreateInfo.pUserData       = nullptr;

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName   = "Engine";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName        = "Engine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.apiVersion         = desiredVulkanVersion;

    std::vector<const char*> instanceExtension;
    instanceExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtension.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    instanceExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    instanceExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    instanceExtension.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    VkInstanceCreateInfo instanceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = &debugMsgCreateInfo,
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = (uint32_t)instanceExtension.size(),
        .ppEnabledExtensionNames = instanceExtension.data()};

    if (VK_SUCCESS != vkCreateInstance(&instanceCreateInfo, nullptr, &sInstance)) {
        ENGINE_ERROR("Failed to create vulkan instance.");
    }

    // ====================================================
    //  Setup debug callback
    // ====================================================

    //
    // Register debug callback
    //
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(sInstance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    debugMsgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMsgCreateInfo.pNext = nullptr;
    debugMsgCreateInfo.flags = 0;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugMsgCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMsgCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugMsgCreateInfo.pfnUserCallback = debugCallback;
    debugMsgCreateInfo.pUserData       = nullptr;
    vkCreateDebugUtilsMessengerEXT(sInstance, &debugMsgCreateInfo, nullptr, &debugMessenger);

    // ====================================================
    //   Find Physical Device
    // ====================================================
    auto physicalDevice = VulkanUtils::getPhysicalDevices(sInstance);
    if (physicalDevice.empty()) {
        ENGINE_ERROR("Failed to initialise vulkan.");
        ENGINE_ERROR("No GPU which support vulkan found.");
        return false;
    }

    // FIXME: If there is more than one GPU, chose the best one
    sPhysicalDevice = physicalDevice[0];

    // ====================================================
    //   Create Device
    //
    // Fow now, only use 1 queue that support graphic and compute.
    // ====================================================

    sGraphicQueueFamilyIndex = VulkanUtils::findDeviceQueueFamilyIndex(
        sPhysicalDevice, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    float                   pQueuePriorities = 0.0;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext;
    queueCreateInfo.flags;
    queueCreateInfo.queueFamilyIndex = sGraphicQueueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &pQueuePriorities;

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.features.imageCubeArray     = true;
    deviceFeatures.features.geometryShader     = true;
    deviceFeatures.features.tessellationShader = true;
    deviceFeatures.features.multiDrawIndirect  = true;
    deviceFeatures.features.drawIndirectFirstInstance = true;
    deviceFeatures.features.samplerAnisotropy  = true;

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

    VkPhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.sType              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.dynamicRendering   = true;
    vulkan13Features.synchronization2   = true;
    vulkan13Features.inlineUniformBlock = true;

    // chain features
    deviceFeatures.pNext   = &vulkan12Features;
    vulkan12Features.pNext = &vulkan13Features;

    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    deviceExtensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    deviceExtensions.push_back(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

    // require for ImGui Vulkan backend even if using Vulkan 1.3
    // Otherwise ImGui Vulkan backend crash when moving a windows outside the
    // main windows because it dynamic rendering function pointer will be null.
    deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    // deviceExtensions.push_back(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &deviceFeatures;
    createInfo.flags                   = 0;
    createInfo.queueCreateInfoCount    = 1;
    createInfo.pQueueCreateInfos       = &queueCreateInfo;
    createInfo.enabledLayerCount       = 0;       // deprecated and ignored.
    createInfo.ppEnabledLayerNames     = nullptr; // deprecated and ignored.
    createInfo.enabledExtensionCount   = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pEnabledFeatures        = nullptr;
    if (VK_SUCCESS != vkCreateDevice(sPhysicalDevice, &createInfo, nullptr, &sDevice)) {
        ENGINE_ERROR("Failed to create vulkan device.");
    }

    vkGetDeviceQueue(sDevice, sGraphicQueueFamilyIndex, 0, &sGraphicsQueue);

    // ==============================================================
    //   Setup memory allocation
    // ==============================================================
    VmaVulkanFunctions vmaVulkanFunctions{};
#ifdef RHI_WANT_VOLK
    /// TODO: Fill vmaVulkanFunctions table if using volk loader
#endif
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion       = desiredVulkanVersion;
    // allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    // allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;   //
    // Enables usage of VK_KHR_dedicated_allocation extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;           // Enables usage of VK_KHR_bind_memory2
    // extension. allocatorInfo.flags           |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT; //
    // Enables usage of VK_EXT_memory_budget extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT; // Enables usage of
    // VK_AMD_device_coherent_memory extension. allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;      // Enables usage of "buffer device
    // address" feature allocatorInfo.flags           |=
    // VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;        // Enables usage of
    // VK_EXT_memory_priority extension in the library.
    allocatorInfo.instance         = sInstance;
    allocatorInfo.physicalDevice   = sPhysicalDevice;
    allocatorInfo.device           = sDevice;
    allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
    if (VK_SUCCESS != vmaCreateAllocator(&allocatorInfo, &sVmaAllocator)) {
        ENGINE_ERROR("Failed to create vma allocator.");
        return false;
    }

    //
    // create Single Time Command Pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = sGraphicQueueFamilyIndex;
    vkCreateCommandPool(sDevice, &poolInfo, nullptr, &sSingleTimeCommandPool);

    return true;
}

void Shutdown() {
    vkDestroyCommandPool(sDevice, sSingleTimeCommandPool, nullptr);
    vmaDestroyAllocator(sVmaAllocator);
    vkDestroyDevice(sDevice, nullptr);

    auto vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            sInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (vkDestroyDebugUtilsMessengerEXT) {
        vkDestroyDebugUtilsMessengerEXT(sInstance, debugMessenger, nullptr);
    }
    vkDestroyInstance(sInstance, nullptr);
    ENGINE_CORE_INFO("Shutdown vulkan ...");
}

VkInstance getIntance() { return sInstance; }

VkPhysicalDevice getPhycalDevice() { return sPhysicalDevice; }

VkDevice getDevice() { return sDevice; }

VmaAllocator getVmaAllocator() { return sVmaAllocator; }

bool isLayerSupported() { return true; }

bool isInstanceExtensionSupported() { return true; }

bool isDeviceExtensionSupported() { return true; }

uint32_t getGraphicQueueFamilyIndex() { return sGraphicQueueFamilyIndex; }
VkQueue  getGraphicQueue() { return sGraphicsQueue; }

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = sSingleTimeCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(sDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(sGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(sGraphicsQueue);

    vkFreeCommandBuffers(sDevice, sSingleTimeCommandPool, 1, &commandBuffer);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    auto cmd = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size      = size;
    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(cmd);
}

void copyBufferToImage(VkCommandBuffer commandBuffer,
                       VkBuffer        buffer,
                       VkImage         image,
                       uint32_t        width,
                       uint32_t        height) {
    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
}

Shader createShaderModule(std::span<const uint32_t> spirv) {
    SpirvReflection spirvReflection;
    spirvReflection.reflect(spirv);

    // create the shader module
    VkShaderModuleCreateInfo pCreateInfo{};
    pCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    pCreateInfo.pNext    = nullptr;
    pCreateInfo.flags    = 0;
    pCreateInfo.codeSize = spirv.size_bytes();
    pCreateInfo.pCode    = spirv.data();

    VkShaderModule shaderModule{};
    VK_CHECK(vkCreateShaderModule(sDevice, &pCreateInfo, nullptr, &shaderModule));
    // if (createInfo.debugName) {
    //     setDebugObjectName(mDevice, (uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
    //                        createInfo.debugName);
    // }

    Shader shader;
    shader.shaderModule           = shaderModule;
    shader.stageCreateInfo.pNext  = nullptr;
    shader.stageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader.stageCreateInfo.flags  = 0;
    shader.stageCreateInfo.module = shaderModule;
    shader.stageCreateInfo.pName  = "main";
    shader.stageCreateInfo.pSpecializationInfo = nullptr;
    shader.stageCreateInfo.stage               = spirvReflection.getShaderStage();
    shader.pushConstantRange                   = spirvReflection.getPushConstantRange();
    shader.descriptorSetLayoutBinding          = spirvReflection.getDescriptorSetLayoutBinding();
    return shader;
}

std::vector<VkDescriptorSetLayout> createDescriptorSetLayout(Shader vert, Shader frag) {
    //
    // merge all bingdings
    //
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> mergedBindings;

    std::initializer_list<Shader> shaders = {vert, frag};
    for(const auto& shader : shaders) {
        for(auto& [setIdx, bindings] : shader.descriptorSetLayoutBinding) {
            for(auto& [bindingIdx, binding] : bindings) {
                auto it = mergedBindings[setIdx].emplace(bindingIdx, binding);
                if(!it.second) {
                    it.first->second.stageFlags |= binding.stageFlags;
                }
            }
        }
    }
#if 0
    // remove duplicate binding + merge shader stage
    std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> uniqueBindings;
    for (auto& [setIdx, bindings] : mergedBindings) {
        uniqueBindings.emplace(setIdx, std::vector<VkDescriptorSetLayoutBinding>{});

        for (auto& binding : bindings) {
            auto it =
                std::ranges::find_if(bindings, [&binding](const VkDescriptorSetLayoutBinding& b) {
                    return b.binding == binding.binding;
                });
            if (it == uniqueBindings[setIdx].end()) {
                uniqueBindings[setIdx].push_back(binding);
            } else {
                it->stageFlags |= binding.stageFlags;
            }
        }
    }
#endif

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
#if 1
    for (const auto& [setIdx, bindings] : mergedBindings) {
        std::vector<VkDescriptorSetLayoutBinding> b;

        for (const auto& [bindingIdx, binding] : bindings) {
            b.push_back(binding);
        }
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        descriptorSetLayoutCreateInfo.flags = 0; // VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
        descriptorSetLayoutCreateInfo.bindingCount = b.size();
        descriptorSetLayoutCreateInfo.pBindings    = b.data();

        VkDescriptorSetLayout descriptorSetLayout{};
        vkCreateDescriptorSetLayout(sDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
        descriptorSetLayouts.push_back(descriptorSetLayout);
    }
#endif
    return descriptorSetLayouts;
}

std::vector<VkPushConstantRange> createPushConstantRange(Shader vert, Shader frag) {
    std::vector<VkPushConstantRange> ranges;

    if (vert.pushConstantRange.size > 0) {
        ranges.emplace_back(vert.pushConstantRange);
    }
    if (frag.pushConstantRange.size > 0) {
        ranges.emplace_back(frag.pushConstantRange);
    }

    return ranges;
}

VkPipelineLayout createPipelineLayout(uint32_t               setLayoutCount,
                                      VkDescriptorSetLayout* descriptorSetLayout,
                                      uint32_t               rangeCount,
                                      VkPushConstantRange*   ranges) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = setLayoutCount;      // Optional
    pipelineLayoutInfo.pSetLayouts            = descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = rangeCount;          // Optional
    pipelineLayoutInfo.pPushConstantRanges    = ranges;              // Optional

    VkPipelineLayout layout{};
    vkCreatePipelineLayout(sDevice, &pipelineLayoutInfo, nullptr, &layout);
    return layout;
}

VkCommandPool createCommandPool(uint32_t queueFamilyIndex, bool transient, bool reset) noexcept {
    //
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    //	specifies that command buffers allocated from the pool will be short-lived,
    //  meaning that they will be reset or freed in a relatively short timeframe.
    //  This flag may be used by the implementation to control memory allocation behavior within the
    //  pool.
    //
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    //	allows any command buffer allocated from a pool to be individually reset to the initial
    // state; either by calling vkResetCommandBuffer,
    //  or via the implicit reset when calling vkBeginCommandBuffer.
    //  If this flag is not set on a pool, then vkResetCommandBuffer must not be called for any
    //  command buffer allocated from that pool.
    //
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = 0;
    if (transient) {
        commandPoolCreateInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }
    if (reset) {
        commandPoolCreateInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool{};
    VK_CHECK(vkCreateCommandPool(sDevice, &commandPoolCreateInfo, nullptr, &commandPool));
    return commandPool;
}

void setDebugObjectName(uint64_t     objectHandle,
                        VkObjectType objectType,
                        std::string  name) {
    static auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(sDevice, "vkSetDebugUtilsObjectNameEXT");
    if(func){
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.pNext        = nullptr;
        nameInfo.objectHandle = objectHandle;
        nameInfo.objectType   = objectType;
        nameInfo.pObjectName  = name.c_str();
        func(sDevice, &nameInfo);
    }
}

void CmdBeginsLabel(VkCommandBuffer cmd, std::string_view label) {
    auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(sInstance, "vkCmdBeginDebugUtilsLabelEXT");
    if(func){
        VkDebugUtilsLabelEXT info{};
        info.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        info.pNext        = nullptr;
        info.pLabelName   = label.data();
        info.color[0]     = 0;
        info.color[1]     = 0;
        info.color[2]     = 0;
        info.color[3]     = 0;
        func(cmd, &info);
    }
}

void CmdEndLabel(VkCommandBuffer cmd) {
    static auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(sInstance, "vkCmdEndDebugUtilsLabelEXT");
    if(func){
        func(cmd);
    }
}

void CmdInsertLabel(VkCommandBuffer cmd, std::string_view label) {
    static auto func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(sInstance, "vkCmdInsertDebugUtilsLabelEXT");
    if(func){
        VkDebugUtilsLabelEXT info{};
        info.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        info.pNext        = nullptr;
        info.pLabelName   = label.data();
        info.color[0]     = 0;
        info.color[1]     = 0;
        info.color[2]     = 0;
        info.color[3]     = 0;
        func(cmd, &info);
    }
}

} // namespace VulkanContext
