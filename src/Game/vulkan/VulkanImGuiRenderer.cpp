#include "VulkanImGuiRenderer.h"

#include "VulkanContext.h"

#include <Engine/SDL3/SDL3Window.h>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

namespace {

VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

// Callback called by SDL
// this is used to send event to ImGui SDL backend.
// Otherwisem we need to call ImGui_ImplSDL3_ProcessEvent() durent event
// processing.
bool eventWatcher(void* userdata, SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    return true;
}

// The number of extra escriptor ImGui will allocate for us.
// This is used for rendering texture inside ImGui window.
constexpr unsigned nbExtraDecriptorSet = 1000;

// Cache all descriptor set ImGui allocate for us.
// Note: The cache is never cleared for now.
//       Texure not used last frame should release it descriptor ?
std::map<VkImageView, VkDescriptorSet> gDescriptorSetCache;

} // namespace

bool VulkanImGuiRenderer::Init(const Engine::SDL3Window& windows) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup ImGui for SDL
    ImGui_ImplSDL3_InitForVulkan(windows.getSDLWindow());
    SDL_AddEventWatch(eventWatcher, nullptr /*userdata*/);

    //
    // ImGui Vulkan setup with dynamic rendering
    //
    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.pNext                   = nullptr;
    renderingCreateInfo.viewMask                = 0;
    renderingCreateInfo.colorAttachmentCount    = 1;
    renderingCreateInfo.pColorAttachmentFormats = &format;
    renderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_D24_UNORM_S8_UINT;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = VulkanContext::getIntance();
    init_info.PhysicalDevice            = VulkanContext::getPhycalDevice();
    init_info.Device                    = VulkanContext::getDevice();
    init_info.QueueFamily               = VulkanContext::getGraphicQueueFamilyIndex();
    init_info.Queue                     = VulkanContext::getGraphicQueue();
    init_info.PipelineCache             = nullptr; // Optional
    init_info.DescriptorPool            = nullptr; // ignored if using DescriptorPoolSize > 0
    init_info.DescriptorPoolSize =
        IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE + nbExtraDecriptorSet;
    init_info.UseDynamicRendering         = true;
    init_info.PipelineRenderingCreateInfo = renderingCreateInfo;
    init_info.Subpass                     = 0; // Optional
    init_info.MinImageCount               = 2;
    init_info.ImageCount                  = 2;
    init_info.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator                   = nullptr;
    init_info.CheckVkResultFn             = nullptr;
    ImGui_ImplVulkan_Init(&init_info);

    return true;
}

void VulkanImGuiRenderer::Shutdown() {
    SDL_RemoveEventWatch(eventWatcher, nullptr /*userdata*/);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void VulkanImGuiRenderer::BeingFrame() {
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void VulkanImGuiRenderer::EndFrame(VkCommandBuffer cmd) {
    // Tell ImGui to generate rendering data
    // This will fill ImGui::GetDrawData()
    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd, nullptr /*pipeline*/);

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void VulkanImGuiRenderer::AddImage(const VulkanTexturePtr& texture,
                                   const ImVec2&           image_size,
                                   const ImVec2&           uv0,
                                   const ImVec2&           uv1,
                                   const ImVec4&           tint_col,
                                   const ImVec4&           border_col) {
    assert(gDescriptorSetCache.size() < nbExtraDecriptorSet);

    VkDescriptorSet ds{};
    auto            it = gDescriptorSetCache.find(texture->getImageView());
    if (it == gDescriptorSetCache.end()) {
        ds = ImGui_ImplVulkan_AddTexture(texture->getSampler(), texture->getImageView(),
                                         VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
        gDescriptorSetCache.emplace(texture->getImageView(), ds);
    } else {
        ds = it->second;
    }

    ImGui::Image((ImTextureID)ds, image_size, uv0, uv1, tint_col, border_col);
}
