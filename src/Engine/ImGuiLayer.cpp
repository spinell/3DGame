#include <Engine/Application.h>
#include <Engine/ImGuiLayer.h>
#include <Engine/Log.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

namespace {
bool eventWatcher(void* userdata, SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    return true;
}
} // namespace

namespace Engine {

ImGuiLayer::ImGuiLayer() : Engine::Layer("ImGuiLayer") {}

void ImGuiLayer::begin() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    // Rendering
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window*   backup_current_window  = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
}

void ImGuiLayer::onAttach() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GLContext gl_context =
        SDL_GL_CreateContext(Engine::Application::Get().GetWindow().getSDLWindow());
    assert(gl_context);

    if (!SDL_GL_MakeCurrent(Engine::Application::Get().GetWindow().getSDLWindow(), gl_context)) {
        ENGINE_CORE_ERROR("SDL_GL_MakeCurrent fail !");
    }

    if (!ImGui_ImplSDL3_InitForOpenGL(Engine::Application::Get().GetWindow().getSDLWindow(),
                                      gl_context)) {
        ENGINE_CORE_ERROR("ImGui_ImplSDL3_InitForOpenGL fail !");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        ENGINE_CORE_ERROR("ImGui_ImplOpenGL3_Init fail !");
    }

    SDL_AddEventWatch(eventWatcher, nullptr /*userdata*/);
}

void ImGuiLayer::onDetach() {
    SDL_RemoveEventWatch(eventWatcher, nullptr /*userdata*/);

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

} // namespace Engine
