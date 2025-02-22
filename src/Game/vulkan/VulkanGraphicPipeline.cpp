#include "VulkanGraphicPipeline.h"

#include "VulkanContext.h"
#include "VulkanShaderProgram.h"

#include <array>

VulkanGraphicPipelinePtr VulkanGraphicPipeline::Create(
    const VulkanGraphicPipelineCreateInfo& createInfo) {
    //============================================================================
    //                            Vertex Input
    //============================================================================
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;

    VkVertexInputBindingDescription   vertexInputBindings[8]{};
    VkVertexInputAttributeDescription vertexInputAttribute[8]{};
    VkVertexInputBindingDescription   inputBinding = {0, createInfo.vertexStride,
                                                      VK_VERTEX_INPUT_RATE_VERTEX};
    if (createInfo.vertexInput.size()) {
        vertexInputInfo.vertexBindingDescriptionCount   = 1;
        vertexInputInfo.pVertexBindingDescriptions      = &inputBinding;
        vertexInputInfo.vertexAttributeDescriptionCount = createInfo.vertexInput.size();
        vertexInputInfo.pVertexAttributeDescriptions    = createInfo.vertexInput.data();
    } else {
        vertexInputInfo.vertexBindingDescriptionCount   = 0;
        vertexInputInfo.pVertexBindingDescriptions      = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions    = 0;
    }

    //============================================================================
    //                           Input Assembly
    //============================================================================
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE
    inputAssembly.topology               = createInfo.primitiveTopology;

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
        //VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
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
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = createInfo.cullMode;
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
    dsStateCI.depthTestEnable  = createInfo.enableDepthTest;
    dsStateCI.depthCompareOp   = createInfo.depthCompareOp;
    dsStateCI.depthWriteEnable = VK_TRUE;

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
    //                          Dynamic rendering
    // =================================================================================
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
    renderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_D24_UNORM_S8_UINT;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    // =================================================================================
    //                      Create the graphic pipeline
    // =================================================================================
    VulkanGraphicPipelinePtr vulkanPipeline = std::make_shared<VulkanGraphicPipeline>();
    vulkanPipeline->mDescriptorSetLayout    = createInfo.shader->getDescriptorSetLayouts();
    vulkanPipeline->mPipelineLayout         = createInfo.shader->getPipelineLayout();

    VkGraphicsPipelineCreateInfo& vkcreateInfo = vulkanPipeline->mCreateInfo;
    vkcreateInfo.sType                         = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkcreateInfo.pNext                         = &renderingCreateInfo;
    vkcreateInfo.flags                         = 0;
    vkcreateInfo.stageCount                    = createInfo.shader->getShaderShages().size();
    vkcreateInfo.pStages                       = createInfo.shader->getShaderShages().data();
    vkcreateInfo.pVertexInputState             = &vertexInputInfo;
    vkcreateInfo.pInputAssemblyState           = &inputAssembly;
    vkcreateInfo.pTessellationState            = nullptr;
    vkcreateInfo.pViewportState                = &viewportState;
    vkcreateInfo.pRasterizationState           = &rasterizer;
    vkcreateInfo.pMultisampleState             = &multisampling;
    vkcreateInfo.pDepthStencilState            = &dsStateCI;
    vkcreateInfo.pColorBlendState              = &colorBlending;
    vkcreateInfo.pDynamicState                 = &dynamicStateCreateInfo;
    vkcreateInfo.layout                        = createInfo.shader->getPipelineLayout();
    vkcreateInfo.renderPass                    = VK_NULL_HANDLE;
    vkcreateInfo.subpass                       = 0;
    vkcreateInfo.basePipelineHandle            = VK_NULL_HANDLE;
    vkcreateInfo.basePipelineIndex             = 0;

    assert(vulkanPipeline);
    VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::getDevice(), VK_NULL_HANDLE, 1,
                                       &vulkanPipeline->mCreateInfo, nullptr,
                                       &vulkanPipeline->mPipeline));

    VulkanContext::setDebugObjectName((uint64_t)vulkanPipeline->mPipeline, VK_OBJECT_TYPE_PIPELINE, createInfo.name.c_str());
    return vulkanPipeline;
}


VulkanGraphicPipeline::~VulkanGraphicPipeline() {
    vkDestroyPipeline(VulkanContext::getDevice(), mPipeline, nullptr);
}
