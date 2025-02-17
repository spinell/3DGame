#pragma once
#include "VulkanContext.h"
#include "vulkan.h"

#include <imgui.h>

namespace Engine {
class SDL3Window;
}

/// @brief Wrapper around ImGui Vulkan backend.
///
/// Call Init() to init.
/// In the render loop
///
///   setup command buffer + render target
///
///   BeingFrame()
///
///   do some ImGui call
///
///   EndFrame(commandBuffer)
///
///   submit the command buffer
///
/// Call Shutdown() after the render loop.
///
/// @note All ImGui call must bedone between BeingFrame() and EndFrame()
///
namespace VulkanImGuiRenderer {

/// @brief Iniialise ImGui for Vulan and SDL.
/// @param windows The window ImGui wil use as main window.
/// @return true on success.
bool Init(const Engine::SDL3Window& windows);

/// @brief Destroy all ressource used by ImGui.
void Shutdown();

/// @brief Start a ImGui frame.
void BeingFrame();

/// @brief End a ImGui frame and register all draw command.
/// @param cmd The command buffer use to register the draw call.
void EndFrame(VkCommandBuffer cmd);

/// @brief Thin wrapper around ImGui::Image().
///        This function will allocate a descriptor set and cache it.
///
/// @param texture
/// @param image_size
/// @param uv0
/// @param uv1
/// @param tint_col
/// @param border_col
void AddImage(Texture       texture,
              const ImVec2& image_size,
              const ImVec2& uv0        = ImVec2(0, 0),
              const ImVec2& uv1        = ImVec2(1, 1),
              const ImVec4& tint_col   = ImVec4(1, 1, 1, 1),
              const ImVec4& border_col = ImVec4(0, 0, 0, 0));

} // namespace VulkanImGuiRenderer
