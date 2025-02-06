#include "TestLayer.h"

#include "Camera.h"
#include "CameraController.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Spirv/SpirvReflection.h"
#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanDescriptorPool.h"
#include "vulkan/VulkanSwapchain.h"
#include "vulkan/VulkanUtils.h"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <Engine/SDL3/SDL3Window.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spirv_fullscreen_quad_frag_glsl.h>
#include <spirv_fullscreen_quad_vert_glsl.h>
#include <spirv_mesh_frag_glsl.h>
#include <spirv_mesh_vert_glsl.h>
#include <spirv_triangle_frag_glsl.h>
#include <spirv_triangle_tex_frag_glsl.h>
#include <spirv_triangle_tex_vert_glsl.h>
#include <spirv_triangle_vert_glsl.h>
#include <entt/entt.hpp>

#include <array>

struct CTransform {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
};
struct CMesh {
    Mesh mesh;
};
struct CMaterial {
    glm::vec4 color;
};

entt::registry registry;

Texture createCheckBoardTexture() {
    Texture texture =
        VulkanContext::createTexture(10, 10, VK_FORMAT_R8G8B8A8_UNORM,
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkCommandBuffer cmd           = VulkanContext::beginSingleTimeCommands();
    VkDeviceSize    imageSize     = texture.width * texture.height * 4;
    auto            stagingBuffer = VulkanContext::createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* ptr{};
    vmaMapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation, &ptr);
    for (uint32_t row = 0; row < texture.height; row++) {
        for (uint32_t col = 0; col < texture.width; col++) {
            auto* color = (char*)ptr;
            if (col % 2) {
                color[0] = 255;
                color[1] = 0;
                color[2] = 255;
                color[3] = 255;
            } else {
                color[0] = 0;
                color[1] = 0;
                color[2] = 255;
                color[3] = 255;
            }
            ptr = (char*)ptr + 4;
        }
    }
    vmaUnmapMemory(VulkanContext::getVmaAllocator(), stagingBuffer.allocation);

    VulkanUtils::transitionImageLayout(
        cmd, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_2_NONE_KHR, VK_ACCESS_2_NONE_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT);

    VulkanContext::copyBufferToImage(cmd, stagingBuffer.buffer, texture.image,
                                     static_cast<uint32_t>(texture.width),
                                     static_cast<uint32_t>(texture.height));

#if 0
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
#endif

    VulkanUtils::transitionImageLayout(
        cmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_READ_BIT);
    VulkanContext::endSingleTimeCommands(cmd);
    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), stagingBuffer.buffer,
                     stagingBuffer.allocation);

    return texture;
};


struct FrameData {
    VkCommandPool   commandPool;
    VkCommandBuffer commandBuffer;
};
FrameData        frameData{};
VulkanSwapchain* vulkanSwapchain{};
Shader           vertShader;
Shader           fragShader;
Shader          vertMeshShader;
Shader          fragMeshShader;
GraphicPipeline meshPipeline;

GraphicPipeline pipeline;
TestLayer1::TestLayer1(const char* name) : Engine::Layer(name) {}
Texture              texture;
Texture              depthBuffer;
VulkanDescriptorPool descriptorPool;
VkDescriptorSet      meshPipelineDescriptorSet0;
struct PushData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    float     color[4];
};

Engine::CameraController cameraController;
std::vector<Mesh>        meshs;


TestLayer1::~TestLayer1() {}

void TestLayer1::onAttach() {
    VulkanContext::Initialize();
    Renderer::Init();
    descriptorPool.init();

    auto meshCube = Mesh::CreateMeshCube(1.0f);
    auto meshGrid = Mesh::CreateGrid(10.0f, 10.0f, 2.0f, 2.0f);
    auto meshCylinder = Mesh::CreateCylinder(1, 1, 10, 10, 10);
    auto meshSphere = Mesh::CreateSphere(1, 10, 10);
    auto meshGeoSphere = Mesh::CreateGeoSphere(1, 10);

    meshs.push_back(meshCube);
    meshs.push_back(meshGrid);
    meshs.push_back(meshCylinder);
    meshs.push_back(meshSphere);
    meshs.push_back(meshGeoSphere);

    // floor
    auto e = registry.create();
    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshGrid;
    registry.emplace<CMaterial>(e).color = {1.f, 1.f, 1.f, 1.0f};
    registry.emplace<CTransform>(e).position = {0, 0, 0};

    // cubes
    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshCube;
    registry.emplace<CMaterial>(e).color = {1.0f, 0.0f, 0.0f, 1.0f};
    registry.emplace<CTransform>(e).position = {0, 0.5f, 0};
    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshCube;
    registry.emplace<CMaterial>(e).color = {1.0f, 1.0f, 1.0f, 1.0f};
    registry.emplace<CTransform>(e).position = {-5, 0, 5};
    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshCube;
    registry.emplace<CMaterial>(e).color = {.5f, .5f, .5f, 1.0f};
    registry.emplace<CTransform>(e).position = {-15, 0, 10};

    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshCylinder;
    registry.emplace<CMaterial>(e).color = {.5f, .5f, .5f, 1.0f};
    registry.emplace<CTransform>(e).position = {5, 5, 10};

    e = registry.create();
    registry.emplace<CMesh>(e).mesh = meshGeoSphere;
    registry.emplace<CMaterial>(e).color = {.5f, .5f, .5f, 1.0f};
    registry.emplace<CTransform>(e).position = {5, 5, 5};

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

    vertShader = VulkanContext::createShaderModule(spirv_fullscreen_quad_vert_glsl);
    fragShader = VulkanContext::createShaderModule(spirv_fullscreen_quad_frag_glsl);
    pipeline   = VulkanContext::createGraphicPipeline(vertShader, fragShader);

    texture     = createCheckBoardTexture();
    depthBuffer = VulkanContext::createTexture(
        vulkanSwapchain->getSize().width, vulkanSwapchain->getSize().height,
        VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Mesh pipeline
    {
        vertMeshShader = VulkanContext::createShaderModule(spirv_mesh_vert_glsl);
        fragMeshShader = VulkanContext::createShaderModule(spirv_mesh_frag_glsl);
        meshPipeline =
            VulkanContext::createGraphicPipeline(vertMeshShader, fragMeshShader, true, true);
        meshPipelineDescriptorSet0 = descriptorPool.allocate(meshPipeline.descriptorSetLayout[0]);

        VulkanContext::setDebugObjectName((uint64_t)meshPipeline.pipeline, VK_OBJECT_TYPE_PIPELINE,
                                          "meshPipeline");
        VulkanContext::setDebugObjectName((uint64_t)meshPipeline.pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                          "meshpipelineLayout");
        VulkanContext::setDebugObjectName((uint64_t)meshPipelineDescriptorSet0, VK_OBJECT_TYPE_DESCRIPTOR_SET,
                                          "meshPipelineDescriptorSet0");
    }
}

void TestLayer1::onDetach() {
    vkDeviceWaitIdle(VulkanContext::getDevice());

    for (auto& m : meshs) {
        vmaDestroyBuffer(VulkanContext::getVmaAllocator(), m.vertexBuffer.buffer,
                         m.vertexBuffer.allocation);
        vmaDestroyBuffer(VulkanContext::getVmaAllocator(), m.indexBuffer.buffer,
                         m.indexBuffer.allocation);
    }

    descriptorPool.destroy();
    vkDestroyCommandPool(VulkanContext::getDevice(), frameData.commandPool, nullptr);

    vmaDestroyImage(VulkanContext::getVmaAllocator(), texture.image, texture.allocation);
    vkDestroyImageView(VulkanContext::getDevice(), texture.view, nullptr);
    vkDestroySampler(VulkanContext::getDevice(), texture.sampler, nullptr);

    vkDestroyImageView(VulkanContext::getDevice(), depthBuffer.view, nullptr);
    vkDestroySampler(VulkanContext::getDevice(), depthBuffer.sampler, nullptr);

    vkDestroyShaderModule(VulkanContext::getDevice(), vertShader.shaderModule, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), fragShader.shaderModule, nullptr);

    pipeline.destroy();
    delete vulkanSwapchain;
    Renderer::Shutdown();
    VulkanContext::Shutdown();
}

void TestLayer1::onUpdate(float timeStep) {
    cameraController.onUpdate(timeStep);

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
        VkRenderingAttachmentInfo colorAttachmentInfo[1]{};
        colorAttachmentInfo[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo[0].pNext = 0;
        colorAttachmentInfo[0].imageView =
            vulkanSwapchain->getImageViews()[vulkanSwapchain->getCurrentBackImageIndex()];
        colorAttachmentInfo[0].imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo[0].resolveMode        = VK_RESOLVE_MODE_NONE;
        colorAttachmentInfo[0].resolveImageView   = nullptr;
        colorAttachmentInfo[0].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentInfo[0].loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo[0].storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo[0].clearValue.color   = {{1.0f, 0.0f, 1.0f, 1.0f}};

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.pNext              = 0;
        depthAttachmentInfo.imageView          = depthBuffer.view;
        depthAttachmentInfo.imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.resolveMode        = VK_RESOLVE_MODE_NONE;
        depthAttachmentInfo.resolveImageView   = nullptr;
        depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentInfo.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo info{};
        info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.pNext                = nullptr;
        info.flags                = 0;
        info.renderArea           = {0, 0, vulkanSwapchain->getSize().width,
                                     vulkanSwapchain->getSize().height};
        info.layerCount           = 1;
        info.viewMask             = 0;
        info.colorAttachmentCount = 1;
        info.pColorAttachments    = colorAttachmentInfo;
        info.pDepthAttachment     = &depthAttachmentInfo;
        info.pStencilAttachment   = nullptr;
        vkCmdBeginRendering(frameData.commandBuffer, &info);
    }

    // render stuff
    {
        VkViewport viewport;
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = vulkanSwapchain->getSize().width;
        viewport.height   = vulkanSwapchain->getSize().height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
        vkCmdSetViewportWithCount(frameData.commandBuffer, 1, &viewport);

        VkRect2D rect;
        rect.offset.x      = 0;
        rect.offset.y      = 0;
        rect.extent.width  = vulkanSwapchain->getSize().width;
        rect.extent.height = vulkanSwapchain->getSize().height;
        vkCmdSetScissorWithCount(frameData.commandBuffer, 1, &rect);

        vkCmdSetPrimitiveTopology(frameData.commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        // draw back ground
        {
            vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline.pipeline);
            vkCmdDraw(frameData.commandBuffer, 3, 1, 0, 0);

        }

        // render scene
        {
            vkCmdSetPrimitiveTopology(frameData.commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              meshPipeline.pipeline);

            VkDescriptorImageInfo descriptorImageInfo;
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            descriptorImageInfo.imageView   = texture.view;
            descriptorImageInfo.sampler     = texture.sampler;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet          = meshPipelineDescriptorSet0;
            writeDescriptorSet.dstBinding      = 0;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.pImageInfo      = &descriptorImageInfo;

            vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);

            PushData pushData;
            pushData.projection = cameraController.getProjectonMatrix();
            pushData.view       = cameraController.getViewMatrix();
            auto view = registry.view<CTransform, CMesh, CMaterial>();
            for(auto [entity, ctrans, cmesh, cmat]: view.each()) {

                pushData.model    = glm::translate(glm::mat4(1), ctrans.position);
                pushData.color[0] = cmat.color.x;
                pushData.color[1] = cmat.color.y;
                pushData.color[2] = cmat.color.z;
                pushData.color[3] = cmat.color.w;

                vkCmdBindDescriptorSets(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    meshPipeline.pipelineLayout, 0 /*firstSet*/, 1 /*nbSet*/,
                                    &meshPipelineDescriptorSet0, 0, nullptr);

                vkCmdPushConstants(frameData.commandBuffer, meshPipeline.pipelineLayout,
                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                    sizeof(pushData), reinterpret_cast<void*>(&pushData));

                Renderer::DrawMesh(frameData.commandBuffer, cmesh.mesh);
            }
        }
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
    cameraController.onEvent(event);

    event.dispatch<Engine::WindowResizedEvent>([this](const auto& e) {
        vkDeviceWaitIdle(VulkanContext::getDevice());

        vulkanSwapchain->build();
        // rebuild depth buffer.
        vkDestroyImageView(VulkanContext::getDevice(), depthBuffer.view, nullptr);
        vkDestroySampler(VulkanContext::getDevice(), depthBuffer.sampler, nullptr);

        depthBuffer = VulkanContext::createTexture(
            vulkanSwapchain->getSize().width, vulkanSwapchain->getSize().height,
            VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    });
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
