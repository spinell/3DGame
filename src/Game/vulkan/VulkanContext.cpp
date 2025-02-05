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

    return true;
}

void Shutdown() {
    vmaDestroyAllocator(sVmaAllocator);
    vkDestroyDevice(sDevice, nullptr);
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

VkPipelineLayout createPipelineLayout(Shader vert, Shader frag) {
    std::array<VkPushConstantRange, 2> pushConstantRange;
    uint32_t                           nbPushRange = 0;

    if (vert.pushConstantRange.size > 0) {
        pushConstantRange[nbPushRange] = vert.pushConstantRange;
        nbPushRange++;
    }
    if (frag.pushConstantRange.size > 0) {
        pushConstantRange[nbPushRange] = frag.pushConstantRange;
        nbPushRange++;
    }

    // merge all bingdings
    std::vector<VkDescriptorSetLayoutBinding> mergedBindings;
    mergedBindings.reserve(vert.descriptorSetLayoutBinding.size() +
                     frag.descriptorSetLayoutBinding.size());
    mergedBindings.insert_range(mergedBindings.end(), vert.descriptorSetLayoutBinding);
    mergedBindings.insert_range(mergedBindings.end(), frag.descriptorSetLayoutBinding);

    // remove duplicate binding + merge shader stage
    std::vector<VkDescriptorSetLayoutBinding> uniqueBindings;
    for(auto& binding : mergedBindings) {
            auto it = std::ranges::find_if(uniqueBindings, [&binding](const VkDescriptorSetLayoutBinding& b){ return b.binding == binding.binding; });
            if(it == uniqueBindings.end()) {
                uniqueBindings.push_back(binding);
            }else {
                it->stageFlags |= binding.stageFlags;
            }
        }

    VkDescriptorSetLayout descriptorSetLayout{};
    if (uniqueBindings.size()) {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        descriptorSetLayoutCreateInfo.flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
        descriptorSetLayoutCreateInfo.bindingCount = uniqueBindings.size();
        descriptorSetLayoutCreateInfo.pBindings    = uniqueBindings.data();
            vkCreateDescriptorSetLayout(sDevice, &descriptorSetLayoutCreateInfo, nullptr,
                                        &descriptorSetLayout);
    }

    return createPipelineLayout(descriptorSetLayout != nullptr ? 1 : 0, &descriptorSetLayout,
                                nbPushRange, pushConstantRange.data());
}

GraphicPipeline createGraphicPipeline(Shader vert, Shader frag) {
    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0] = vert.stageCreateInfo;
    shaderStages[1] = frag.stageCreateInfo;

    //
    // dynamic rendering
    //
    std::array<VkFormat, 8> format{VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED,
                                   VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED,
                                   VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};
    format[0] = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.pNext                   = nullptr;
    renderingCreateInfo.viewMask                = 0;
    renderingCreateInfo.colorAttachmentCount    = 1; // FIXME
    renderingCreateInfo.pColorAttachmentFormats = format.data();
    renderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_UNDEFINED;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //============================================================================
    //                            Vertex Input
    //============================================================================
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;

    VkVertexInputBindingDescription   vertexInputBindings[8]{};
    VkVertexInputAttributeDescription vertexInputAttribute[8]{};
    vertexInputInfo.vertexBindingDescriptionCount   = 0;
    vertexInputInfo.pVertexBindingDescriptions      = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions    = 0;

    //============================================================================
    //                           Input Assembly
    //============================================================================
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //============================================================================
    //                           Dynamic States
    //============================================================================
    std::vector<VkDynamicState> dynamicStates = {
        // specifies that the pViewports state in VkPipelineViewportStateCreateInfo will
        // be ignored and must be set dynamically with vkCmdSetViewport() before any drawing
        // commands. The number of viewports used by a pipeline is still specified by the
        // viewportCount member of VkPipelineViewportStateCreateInfo.
        // VK_DYNAMIC_STATE_VIEWPORT,
        // specifies that the pScissors state in VkPipelineViewportStateCreateInfo will be
        // ignored and must be set dynamically with vkCmdSetScissor() before any drawing commands.
        // The number of scissor rectangles used by a pipeline is still specified by the
        // scissorCount member of VkPipelineViewportStateCreateInfo.
        // VK_DYNAMIC_STATE_SCISSOR,              // vkCmdSetScissor
        // VK_DYNAMIC_STATE_LINE_WIDTH,           // vkCmdSetLineWidth
        // VK_DYNAMIC_STATE_DEPTH_BIAS,           // vkCmdSetDepthBiasEnable
        VK_DYNAMIC_STATE_BLEND_CONSTANTS, // vkCmdSetBlendConstants
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,    // vkCmdSetDepthBounds
        // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, // vkCmdSetStencilCompareMask
        // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,   // vkCmdSetStencilWriteMask
        VK_DYNAMIC_STATE_STENCIL_REFERENCE, // vkCmdSetStencilReference
        // =======================================================
        //              Provided by VK_VERSION_1_3
        // =======================================================
        // VK_DYNAMIC_STATE_CULL_MODE,
        // VK_DYNAMIC_STATE_FRONT_FACE,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
        // specifies that the viewportCount and pViewports state in
        // VkPipelineViewportStateCreateInfo will be ignored and must
        // be set dynamically with vkCmdSetViewportWithCount before any
        // draw call.
        VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        // specifies that the scissorCount and pScissors state in
        // VkPipelineViewportStateCreateInfo will be ignored and
        // must be set dynamically with vkCmdSetScissorWithCount before
        // any draw call.
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
        // Specifies that the stride state in VkVertexInputBindingDescription
        // will be ignored and must be set dynamically with vkCmdBindVertexBuffers2
        // before any draw call.
        // VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

    // Used when rasterization is enabled.
    // If the VK_EXT_extended_dynamic_state3 extension is enabled,
    // it can be avoid if the pipeline is created with both VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
    // and VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT dynamic states set.
    //  - Still required when using VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR for the
    //  viewportCount and scissorCount
    //  - Still required when using VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT or
    //  VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT but field are ignored.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    // =================================================================================
    //                              rasterizer states
    // =================================================================================
    // const auto&                            rasterizerStates = createInfo.rasterizerStates;
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_NONE;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    // =================================================================================
    //                                 Multisample
    // =================================================================================
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.minSampleShading      = 0.0f;    // Optional
    multisampling.pSampleMask           = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional

    // ==========================================================================
    //                           Depth stencil states
    //
    // - front and back stencil reference is not set in th pipeline.
    //   We use dynamic state for it (vkCmdSetStencilReference())
    // - minDepthBounds and maxDepthBounds is not set in the pipeline.
    //   We use dynamic state for it (vkCmdSetDepthBounds)
    // ==========================================================================
    VkPipelineDepthStencilStateCreateInfo dsStateCI{};
    dsStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // =================================================================================
    //                                Blend states
    // =================================================================================
    std::array<VkPipelineColorBlendAttachmentState, 8> colorBlendAttachments{};
    colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext           = nullptr;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = colorBlendAttachments.data();

    // =================================================================================
    //                      Create the graphic pipeline
    // =================================================================================
    VkGraphicsPipelineCreateInfo vkPipelineCI{};
    vkPipelineCI.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkPipelineCI.pNext               = &renderingCreateInfo;
    vkPipelineCI.flags               = 0;
    vkPipelineCI.stageCount          = 2;
    vkPipelineCI.pStages             = shaderStages;
    vkPipelineCI.pVertexInputState   = &vertexInputInfo;
    vkPipelineCI.pInputAssemblyState = &inputAssembly;
    vkPipelineCI.pTessellationState  = nullptr;
    vkPipelineCI.pViewportState      = &viewportState;
    vkPipelineCI.pRasterizationState = &rasterizer;
    vkPipelineCI.pMultisampleState   = &multisampling;
    vkPipelineCI.pDepthStencilState  = &dsStateCI;
    vkPipelineCI.pColorBlendState    = &colorBlending;
    vkPipelineCI.pDynamicState       = &dynamicStateCreateInfo;
    vkPipelineCI.layout              = createPipelineLayout(vert, frag);
    vkPipelineCI.renderPass          = VK_NULL_HANDLE;
    vkPipelineCI.subpass             = 0;
    vkPipelineCI.basePipelineHandle  = VK_NULL_HANDLE;
    vkPipelineCI.basePipelineIndex   = 0;
    VkPipeline pipeline;
    VK_CHECK(
        vkCreateGraphicsPipelines(sDevice, VK_NULL_HANDLE, 1, &vkPipelineCI, nullptr, &pipeline));

    // setDebugObjectName(mDevice, (uint64_t)vulkanGraphicPipeline->pipeline,
    // VK_OBJECT_TYPE_PIPELINE, createInfo.debugName);

    GraphicPipeline graphicPipeline;
    graphicPipeline.pipeline       = pipeline;
    graphicPipeline.pipelineLayout = vkPipelineCI.layout;
    return graphicPipeline;
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

Buffer createBuffer(VkBufferUsageFlags usageFlags, uint64_t sizeInByte) noexcept {
    Buffer buffer{};
    buffer.sizeInByte = sizeInByte;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size  = sizeInByte;
    bufferCreateInfo.usage = usageFlags;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = (VmaMemoryUsage)VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    allocInfo.requiredFlags           = 0;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateBuffer(sVmaAllocator, &bufferCreateInfo, &allocInfo, &buffer.buffer,
                             &buffer.allocation, nullptr /*allocationInfo*/
                             ));

#if 0
    if (createInfo.debugName) {
        setDebugObjectName(mDevice, (uint64_t)internalBuffer->buffer,
                           VK_OBJECT_TYPE_BUFFER, createInfo.debugName);
    }

    if (createInfo.initialData) {
        void* pData{};
        vmaMapMemory(mVmaAllocator, internalBuffer->allocation, &pData);
        std::memcpy(pData, createInfo.initialData, createInfo.sizeInByte);
        vmaUnmapMemory(mVmaAllocator, internalBuffer->allocation);
    }
#endif
    return buffer;
}

Texture createTexture() noexcept {
    Texture texture;

    // TODO: Make this dynamic
    const VkFormat              format    = VK_FORMAT_R8G8B8A8_UNORM;
    const VkSampleCountFlagBits nbSamples = VK_SAMPLE_COUNT_1_BIT;
    const VkExtent3D            extent    = {800, 600, 1};

    //
    // Create the image
    //
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext       = nullptr;
    imageCreateInfo.flags       = 0;
    imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format      = format;
    imageCreateInfo.extent      = extent;
    imageCreateInfo.mipLevels   = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples     = nbSamples;
    imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices   = nullptr;
    imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags                   = 0;
    allocInfo.usage                   = (VmaMemoryUsage)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.requiredFlags           = 0;
    allocInfo.preferredFlags          = 0;
    allocInfo.memoryTypeBits          = 0;
    allocInfo.pool                    = nullptr;
    allocInfo.pUserData               = nullptr;
    allocInfo.priority                = 0;
    VK_CHECK(vmaCreateImage(sVmaAllocator, &imageCreateInfo, &allocInfo, &texture.image,
                            &texture.allocation, nullptr /*allocationInfo*/
                            ));

    //
    // Create the image view
    //
    VkImageViewCreateInfo ivCreateInfo           = {};
    ivCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.pNext                           = nullptr;
    ivCreateInfo.flags                           = 0;
    ivCreateInfo.image                           = texture.image;
    ivCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    ivCreateInfo.format                          = format;
    ivCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    ivCreateInfo.subresourceRange.baseMipLevel   = 0;
    ivCreateInfo.subresourceRange.levelCount     = 1;
    ivCreateInfo.subresourceRange.baseArrayLayer = 0;
    ivCreateInfo.subresourceRange.layerCount     = 1;
    vkCreateImageView(sDevice, &ivCreateInfo, nullptr, &texture.view);

    // setDebugObjectName(mDevice, (uint64_t)vulkanTexture->image, VK_OBJECT_TYPE_IMAGE,
    // "Texture2D"); setDebugObjectName(mDevice, (uint64_t)vulkanTexture->view,
    // VK_OBJECT_TYPE_IMAGE_VIEW, "Texture2DView");

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext;
    samplerCreateInfo.flags;
    samplerCreateInfo.magFilter;
    samplerCreateInfo.minFilter;
    samplerCreateInfo.mipmapMode;
    samplerCreateInfo.addressModeU;
    samplerCreateInfo.addressModeV;
    samplerCreateInfo.addressModeW;
    samplerCreateInfo.mipLodBias;
    samplerCreateInfo.anisotropyEnable;
    samplerCreateInfo.maxAnisotropy;
    samplerCreateInfo.compareEnable;
    samplerCreateInfo.compareOp;
    samplerCreateInfo.minLod;
    samplerCreateInfo.maxLod;
    samplerCreateInfo.borderColor;
    samplerCreateInfo.unnormalizedCoordinates;
    VK_CHECK(vkCreateSampler(sDevice, &samplerCreateInfo, nullptr, &texture.sampler));

    VkCommandPool   transferBufferPool{};
    VkCommandBuffer transferBuffer{};
    if (!transferBufferPool) {
        transferBufferPool =
            createCommandPool(sGraphicQueueFamilyIndex, false /*transient*/, true /*reset*/);
        VkCommandBufferAllocateInfo cmd_buf_info{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = transferBufferPool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};
        VK_CHECK(vkAllocateCommandBuffers(sDevice, &cmd_buf_info, &transferBuffer));
    }
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext            = 0;
    commandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(transferBuffer, &commandBufferBeginInfo);
    VkCommandBuffer cmd = transferBuffer;

    VulkanUtils::transitionImageLayout(
        cmd, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    VkImageSubresourceRange sr{};
    sr.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    sr.baseMipLevel   = 0;
    sr.levelCount     = 1;
    sr.baseArrayLayer = 0;
    sr.layerCount     = 1;
    VkClearColorValue cc;
    cc.float32[0] = 0;
    cc.float32[1] = 1;
    cc.float32[2] = 0;
    cc.float32[3] = 1;
    vkCmdClearColorImage(cmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cc, 1, &sr);

    VulkanUtils::transitionImageLayout(
        cmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    vkEndCommandBuffer(cmd);

    // flush
    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.pNext         = nullptr;
    commandBufferSubmitInfo.commandBuffer = cmd;
    commandBufferSubmitInfo.deviceMask    = 0;

    VkSubmitInfo2 submitInfo2{};
    submitInfo2.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo2.pNext                    = nullptr;
    submitInfo2.flags                    = 0;
    submitInfo2.waitSemaphoreInfoCount   = 0;
    submitInfo2.pWaitSemaphoreInfos      = nullptr;
    submitInfo2.commandBufferInfoCount   = 1;
    submitInfo2.pCommandBufferInfos      = &commandBufferSubmitInfo;
    submitInfo2.signalSemaphoreInfoCount = 0;
    submitInfo2.pSignalSemaphoreInfos    = nullptr;
    VK_CHECK(vkQueueSubmit2(sGraphicsQueue, 1 /*submitCount*/, &submitInfo2, nullptr /*fence*/));
    vkDeviceWaitIdle(sDevice);

    return texture;
}

} // namespace VulkanContext
