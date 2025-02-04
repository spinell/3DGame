#include "TestLayer.h"

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanSwapchain.h"
#include "vulkan/VulkanUtils.h"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <Engine/SDL3/SDL3Window.h>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <spirv_fullscreen_quad_frag_glsl.h>
#include <spirv_fullscreen_quad_vert_glsl.h>
#include <spirv_triangle_vert_glsl.h>
#include <spirv_triangle_frag_glsl.h>
#include "Spirv/SpirvReflection.h"
#include <array>

struct FrameData {
    VkCommandPool   commandPool;
    VkCommandBuffer commandBuffer;
};
FrameData        frameData{};
VulkanSwapchain* vulkanSwapchain{};
Shader           vertShader;
Shader           fragShader;
Shader           vertTriangleShader;
Shader           fragTriangleShader;
VkPipelineLayout pipelineLayout;
GraphicPipeline  pipeline;
VkPipelineLayout trianglePipelineLayout;
GraphicPipeline  trianglePipeline;
TestLayer1::TestLayer1(const char* name) : Engine::Layer(name) {}

struct PushData{
    float offset[2];
    float size[2];
    float color[4];
};

TestLayer1::~TestLayer1() {}

void TestLayer1::onAttach() {
    VulkanContext::Initialize();
    auto sdlWindow   = Engine::Application::Get().GetWindow().getSDLWindow();
    auto win32Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow),
                                              SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = 0,
        .hinstance = nullptr,
        .hwnd      = (HWND)win32Handle};
    VkSurfaceKHR surface;
    if (VK_SUCCESS != vkCreateWin32SurfaceKHR(VulkanContext::getIntance(), &surfaceCreateInfo,
                                              nullptr, &surface)) {
        ENGINE_CORE_ERROR("vkCreateWin32SurfaceKHR fail");
    }
    vulkanSwapchain =
        new VulkanSwapchain(VulkanContext::getIntance(), VulkanContext::getPhycalDevice(),
                            VulkanContext::getDevice(), surface);
    vulkanSwapchain->build();
    // Create Command pool
    {
        VkCommandPoolCreateFlags flags{};
        // allows any command buffer allocated from a pool to be individually
        // reset to the initial state; either by calling vkResetCommandBuffer,
        // or via the implicit reset when calling vkBeginCommandBuffer.
        flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // specifies that command buffers allocated from the pool will be short-lived,
        // meaning that they will be reset or freed in a relatively short timeframe.
        flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = flags,
            .queueFamilyIndex = VulkanContext::getGraphicQueueFamilyIndex()};
        vkCreateCommandPool(VulkanContext::getDevice(), &commandPoolCreateInfo, nullptr,
                            &frameData.commandPool);
    }

    // Command buffer allocation
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = frameData.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(VulkanContext::getDevice(), &allocInfo, &frameData.commandBuffer);
    }

    vertShader     = VulkanContext::createShaderModule(spirv_fullscreen_quad_vert_glsl,
                                                       VK_SHADER_STAGE_VERTEX_BIT);
    fragShader     = VulkanContext::createShaderModule(spirv_fullscreen_quad_frag_glsl,
                                                       VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineLayout = VulkanContext::createPipelineLayout(0, nullptr, 0, nullptr);
    pipeline       = VulkanContext::createGraphicPipeline(vertShader, fragShader, pipelineLayout);


    SpirvReflection spirvReflection1;
    SpirvReflection spirvReflection2;
    spirvReflection1.reflect(spirv_triangle_vert_glsl);
    spirvReflection2.reflect(spirv_triangle_frag_glsl);


    vertTriangleShader = VulkanContext::createShaderModule(spirv_triangle_vert_glsl,
                                                       spirvReflection1.getShaderStage());
    fragTriangleShader = VulkanContext::createShaderModule(spirv_triangle_frag_glsl,
                                                       spirvReflection2.getShaderStage());

    std::array<VkPushConstantRange, 2> pushConstantRange = {
        spirvReflection1.getPushConstantRange(),
        spirvReflection2.getPushConstantRange()
    };
    trianglePipelineLayout = VulkanContext::createPipelineLayout(0, nullptr, pushConstantRange.size(), pushConstantRange.data());
    trianglePipeline       = VulkanContext::createGraphicPipeline(vertTriangleShader, fragTriangleShader, trianglePipelineLayout);
}

void TestLayer1::onDetach() {
    vkDeviceWaitIdle(VulkanContext::getDevice());

    vkDestroyCommandPool(VulkanContext::getDevice(), frameData.commandPool, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), vertShader.shaderModule, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), fragShader.shaderModule, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), vertTriangleShader.shaderModule, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), fragTriangleShader.shaderModule, nullptr);
    vkDestroyPipelineLayout(VulkanContext::getDevice(), pipelineLayout, nullptr);
    vkDestroyPipeline(VulkanContext::getDevice(), pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(VulkanContext::getDevice(), trianglePipelineLayout, nullptr);
    vkDestroyPipeline(VulkanContext::getDevice(), trianglePipeline.pipeline, nullptr);
    delete vulkanSwapchain;
    VulkanContext::Shutdown();
}

void TestLayer1::onUpdate(float timeStep) {
    // start command buffer
    {
        VkCommandBufferUsageFlags flags{};
        // The command buffer will be rerecorded right after executing it once.
        // flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        // This is a secondary command buffer that will be entirely within a single render pass.
        // flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        // The command buffer can be resubmitted while it is also already pending execution.
        // flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags            = flags;   // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will
        // implicitly reset it.
        vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);
    }

    // transition swapchain image layout
    {
        // Move the swapchain's back image layour from
        // VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        VulkanUtils::transitionImageLayout(
            frameData.commandBuffer,
            vulkanSwapchain->getImages()[vulkanSwapchain->getCurrentBackImageIndex()],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    }

    // start render pass
    {
        VkClearValue clearValue;
        clearValue.color.float32[0] = 1;
        clearValue.color.float32[1] = 0;
        clearValue.color.float32[2] = 0;
        clearValue.color.float32[3] = 1;

        VkRenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.pNext = 0;
        colorAttachmentInfo.imageView =
            vulkanSwapchain->getImageViews()[vulkanSwapchain->getCurrentBackImageIndex()];
        colorAttachmentInfo.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.resolveMode        = VK_RESOLVE_MODE_NONE;
        colorAttachmentInfo.resolveImageView   = nullptr;
        colorAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentInfo.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo.clearValue         = clearValue;

        VkRenderingInfo info{};
        info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.pNext                = nullptr;
        info.flags                = 0;
        info.renderArea           = {0, 0, vulkanSwapchain->getSize().width,
                                     vulkanSwapchain->getSize().height};
        info.layerCount           = 1;
        info.viewMask             = 0;
        info.colorAttachmentCount = 1;
        info.pColorAttachments    = &colorAttachmentInfo;
        info.pDepthAttachment     = nullptr;
        info.pStencilAttachment   = nullptr;
        vkCmdBeginRendering(frameData.commandBuffer, &info);
    }

    // render stuff
    {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = vulkanSwapchain->getSize().width;
        viewport.height = vulkanSwapchain->getSize().height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
        vkCmdSetViewportWithCount(frameData.commandBuffer, 1, &viewport);

        VkRect2D rect;
        rect.offset.x = 0;
        rect.offset.y = 0;
        rect.extent.width= vulkanSwapchain->getSize().width;
        rect.extent.height= vulkanSwapchain->getSize().height;
        vkCmdSetScissorWithCount(frameData.commandBuffer, 1, &rect);

        vkCmdSetPrimitiveTopology(frameData.commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
        vkCmdDraw(frameData.commandBuffer, 3, 1, 0, 0);


        PushData pushData;
        pushData.offset[0]= -.8f;
        pushData.offset[1]= -.4f;
        pushData.size[0]  = .2f;
        pushData.size[1]  = .2f;
        pushData.color[0] = 0;
        pushData.color[1] = 0;
        pushData.color[2] = 1;
        pushData.color[3] = 1;
        vkCmdPushConstants(frameData.commandBuffer, trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,    0, sizeof(float) * 4, reinterpret_cast<void*>(&pushData));
        vkCmdPushConstants(frameData.commandBuffer, trianglePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, sizeof(float) * 4, reinterpret_cast<void*>(&pushData.color));
        vkCmdSetPrimitiveTopology(frameData.commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline.pipeline);
        vkCmdDraw(frameData.commandBuffer, 3, 1, 0, 0);
    }

    // end render pass
    vkCmdEndRendering(frameData.commandBuffer);

    // transition swapchain imagelayout
    {
        // Move the swapchain's back image layour from
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        VulkanUtils::transitionImageLayout(
            frameData.commandBuffer,
            vulkanSwapchain->getImages()[vulkanSwapchain->getCurrentBackImageIndex()],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE);
    }

    // end command buffer
    vkEndCommandBuffer(frameData.commandBuffer);

    // submit command buffer
    {
        VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
        commandBufferSubmitInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.pNext         = nullptr;
        commandBufferSubmitInfo.commandBuffer = frameData.commandBuffer;
        commandBufferSubmitInfo.deviceMask    = 0;

        std::vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfo;
        {
            VkSemaphoreSubmitInfo info;
            info.sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.pNext       = nullptr;
            info.semaphore   = vulkanSwapchain->getImageAvailableSemaphores();
            info.value       = 0;
            info.stageMask   = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            info.deviceIndex = 0;
            waitSemaphoreSubmitInfo.push_back(info);
        }
        std::vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfo;
        {
            VkSemaphoreSubmitInfo info;
            info.sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.pNext       = nullptr;
            info.semaphore   = vulkanSwapchain->getRenderFinishSemaphores();
            info.value       = 0;
            info.stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            info.deviceIndex = 0;
            signalSemaphoreSubmitInfo.push_back(info);
        }

        // Move the command buffer to the Pending states
        VkSubmitInfo2 submitInfo2{};
        submitInfo2.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo2.pNext                    = nullptr;
        submitInfo2.flags                    = 0;
        submitInfo2.waitSemaphoreInfoCount   = waitSemaphoreSubmitInfo.size();
        submitInfo2.pWaitSemaphoreInfos      = waitSemaphoreSubmitInfo.data();
        submitInfo2.commandBufferInfoCount   = 1;
        submitInfo2.pCommandBufferInfos      = &commandBufferSubmitInfo;
        submitInfo2.signalSemaphoreInfoCount = signalSemaphoreSubmitInfo.size();
        submitInfo2.pSignalSemaphoreInfos    = signalSemaphoreSubmitInfo.data();
        VK_CHECK(
            vkQueueSubmit2(VulkanContext::getGraphicQueue(), 1 /*submitCount*/, &submitInfo2, 0));
    }

    vulkanSwapchain->present(VulkanContext::getGraphicQueue(), VK_PRESENT_MODE_MAILBOX_KHR);

    // tempo
    vkDeviceWaitIdle(VulkanContext::getDevice());
}

void TestLayer1::onImGuiRender() {}

bool TestLayer1::onEvent(const Engine::Event& event) {
    event.dispatch<Engine::KeyEvent>([](const Engine::KeyEvent& e) {
        if (e.isPressed()) {
            if (e.getKey() == Engine::KeyCode::Escape) {
                Engine::Application::Get().close();
            }
            if (e.getKey() == Engine::KeyCode::Key1) {
                Engine::Application::Get().GetWindow().setFullScreen(true);
            }
            if (e.getKey() == Engine::KeyCode::Key2) {
                Engine::Application::Get().GetWindow().setFullScreen(false);
            }
            if (e.getKey() == Engine::KeyCode::KeyPad0) {
                Engine::Application::Get().GetWindow().toogleMouseGrab();
            }
            if (e.getKey() == Engine::KeyCode::KeyPad1) {
                Engine::Application::Get().GetWindow().toogleMouseRelativeMode();
            }
        }
    });
    event.dispatch<Engine::MouseButtonEvent>([](const Engine::MouseButtonEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    event.dispatch<Engine::MouseMovedEvent>([](const Engine::MouseMovedEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    return false;
}
