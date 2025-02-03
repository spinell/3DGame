#include "EditorLayer.h"

#include "imgui_impl_opengl3.h"

#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/LogPanel.h"
#include "fonts/IconsMaterialDesignIcons.h"
#include "fonts/embeded/IconsMaterialDesignIcons.h_materialdesignicons-webfont.ttf.h"
#include "fonts/embeded/RobotoRegular.inl"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/sinks/callback_sink.h>

namespace {
bool showDemoWindow        = false;
bool showMetricsWindow     = false;
bool showDebugLogWindow    = false;
bool showIDStackToolWindow = false;

const std::array<ImWchar, 3> mdi_icons_ranges = {ICON_MIN_MDI, ICON_MAX_MDI, 0};

void loadFonts() {
    const float fontSize = 14.0F;

    ImFontConfig fontConfig{};

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        static_cast<const void*>(RobotoRegular_compressed_data), RobotoRegular_compressed_size,
        fontSize, &fontConfig, ImGui::GetIO().Fonts->GetGlyphRangesDefault());

    // merge icons

    fontConfig.FontDataOwnedByAtlas = false;
    fontConfig.MergeMode            = true;
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)(s_materialdesignicons_webfont_ttf),
                                               sizeof(s_materialdesignicons_webfont_ttf), fontSize,
                                               &fontConfig, mdi_icons_ranges.data());
}
}; // namespace

EditorLayer::EditorLayer() : Engine::Layer("EditorLayer") {}

EditorLayer::~EditorLayer() {}

void EditorLayer::onAttach() {
    loadFonts();

    ImGui::GetIO().ConfigDebugIsDebuggerPresent = true;

    ImGui_ImplOpenGL3_CreateFontsTexture();
    setImGuiTheme(4);

    LogPanel* logPanel = mPanelManager.addPanel<LogPanel>("Console");
    mPanelManager.addPanel<HierarchyPanel>("Hierarchy");
    mPanelManager.addPanel<InspectorPanel>("Inspector");

    Engine::Log::getCoreLogger()->sinks().push_back(
        std::make_shared<spdlog::sinks::callback_sink_mt>([&logPanel = logPanel](
                                                              const spdlog::details::log_msg& msg) {
            auto getMessageLevel = [](const spdlog::level::level_enum level) -> LogPanel::LogLevel {
                switch (level) {
                        // clang-format off
                    case spdlog::level::level_enum::trace:    return LogPanel::LogLevel::Trace;
                    case spdlog::level::level_enum::debug:    return LogPanel::LogLevel::Debug;
                    case spdlog::level::level_enum::info:     return LogPanel::LogLevel::Info;
                    case spdlog::level::level_enum::warn:     return LogPanel::LogLevel::Warning;
                    case spdlog::level::level_enum::err:      return LogPanel::LogLevel::Error;
                    case spdlog::level::level_enum::critical: return LogPanel::LogLevel::Critical;
                    default:                                  return LogPanel::LogLevel::Trace;
                        // clang-format on
                }
            };

            const std::string message(msg.payload.data(), msg.payload.size());

#if __cpp_lib_chrono >= 201907L
            auto const        localTime = std::chrono::current_zone()->to_local(msg.time);
            const std::string timeStr   = std::format("{:%X}", localTime);
#else
            // FIXME: std::chrono::current_zone() is missing on Mingw (CLANG64)
            time_t            time       = std::chrono::system_clock::to_time_t(msg.time);
            tm*               timeBuffer = localtime(&time);
            std::stringstream timeString;
            timeString << std::put_time(timeBuffer, "%T");
            const std::string timeStr = timeString.str();
#endif
            logPanel->addMessage(getMessageLevel(msg.level), message, timeStr);
        }));
}

void EditorLayer::onDetach() {}

void EditorLayer::onUpdate(float timeStep) {}

void EditorLayer::onImGuiRender() {
    ImGuiDockNodeFlags dockspaceFlags = 0;
    dockspaceFlags |= ImGuiDockNodeFlags_NoWindowMenuButton; // internal flag
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);

    drawMainMenu();

    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (showMetricsWindow) ImGui::ShowMetricsWindow(&showMetricsWindow);
    if (showDebugLogWindow) ImGui::ShowDebugLogWindow(&showDebugLogWindow);
    if (showIDStackToolWindow) ImGui::ShowIDStackToolWindow(&showIDStackToolWindow);

    if (ImGui::Begin("Viewport")) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(),
            ImGui::ColorConvertFloat4ToU32(ImVec4{0.6f, 0.6f, 0.6F, 1}));
    }
    ImGui::End();

    mPanelManager.onImGui();
}

bool EditorLayer::onEvent(const Engine::Event& event) { return false; }

void EditorLayer::drawMainMenu() {
    const bool isVisible = ImGui::BeginMainMenuBar();
    if (!isVisible) {
        return;
    }

    //
    // File menu
    //
    if (ImGui::BeginMenu("File", true /*enabled*/)) {
        if (ImGui::MenuItem("New Scene")) {
            ENGINE_CORE_WARNING("New Scene not implemented.");
        }
        if (ImGui::MenuItem("Save Scene")) {
            ENGINE_CORE_WARNING("Save Scene not implemented.");
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Alt + F4")) {
            Engine::Application::Get().close();
        }

        ImGui::EndMenu();
    }

    //
    // Edit menu
    //
    if (ImGui::BeginMenu("Edit", true /*enabled*/)) {
        if (ImGui::MenuItem("Undo", nullptr, nullptr)) {
        }
        if (ImGui::MenuItem("Redo", nullptr, nullptr)) {
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Clear Selection")) {
        }
        ImGui::EndMenu();
    }

    //
    // Theme menu
    //
    if (ImGui::BeginMenu("Theme")) {
        if (ImGui::MenuItem("ImGui Dark")) {
            setImGuiTheme(0);
        }
        if (ImGui::MenuItem("ImGui Light")) {
            setImGuiTheme(1);
        }
        if (ImGui::MenuItem("ImGui Classic")) {
            setImGuiTheme(2);
        }
        if (ImGui::MenuItem("Custom 1")) {
            setImGuiTheme(3);
        }
        if (ImGui::MenuItem("Custom 2")) {
            setImGuiTheme(4);
        }
        ImGui::EndMenu();
    }

    //
    // Windows Menu
    //
    if (ImGui::BeginMenu("Windows")) {
        for (auto& [id, data] : mPanelManager.getPanels()) {
            if (ImGui::MenuItem(id.c_str(), nullptr, &data.isOpen)) {
            }
        }
        ImGui::EndMenu();
    }

    //
    // Tools menu
    //
    if (ImGui::BeginMenu("Tools")) {
        showDemoWindow        = ImGui::MenuItem("ImGui Demo");
        showMetricsWindow     = ImGui::MenuItem("ImGui Metric");
        showDebugLogWindow    = ImGui::MenuItem("ImGui Debug Log");
        showIDStackToolWindow = ImGui::MenuItem("ImGui stack editor");
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("About")) {
    }

    ImGui::EndMainMenuBar();
}

void EditorLayer::setImGuiTheme(int index) {
    if (index == 0) {
        ImGui::StyleColorsDark();
    } else if (index == 1) {
        ImGui::StyleColorsLight();
    } else if (index == 2) {
        ImGui::StyleColorsClassic();
    } else if (index == 3) {
        struct HVSSetup {
            float col_main_hue = 0.0F / 255.0F;
            float col_main_sat = 200.0F / 255.0F;
            float col_main_val = 170.0F / 255.0F;

            float col_area_hue = 0.0F / 255.0F;
            float col_area_sat = 0.0F / 255.0F;
            float col_area_val = 60.0F / 255.0F;

            float col_back_hue = 0.0F / 255.0F;
            float col_back_sat = 0.0F / 255.0F;
            float col_back_val = 35.0F / 255.0F;

            float col_text_hue   = 0.0F / 255.0F;
            float col_text_sat   = 0.0F / 255.0F;
            float col_text_val   = 255.0F / 255.0F;
            float frame_rounding = 0.0F;
        };
        const HVSSetup setup;

        const ImVec4 col_text =
            ImColor::HSV(setup.col_text_hue, setup.col_text_sat, setup.col_text_val);
        const ImVec4 col_main =
            ImColor::HSV(setup.col_main_hue, setup.col_main_sat, setup.col_main_val);
        const ImVec4 col_back =
            ImColor::HSV(setup.col_back_hue, setup.col_back_sat, setup.col_back_val);
        const ImVec4 col_area =
            ImColor::HSV(setup.col_area_hue, setup.col_area_sat, setup.col_area_val);
        const float rounding = setup.frame_rounding;

        ImGuiStyle& style                   = ImGui::GetStyle();
        style.FrameRounding                 = rounding;
        style.WindowRounding                = rounding;
        style.Colors[ImGuiCol_Text]         = ImVec4(col_text.x, col_text.y, col_text.z, 1.00F);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(col_text.x, col_text.y, col_text.z, 0.58F);
        style.Colors[ImGuiCol_WindowBg]     = ImVec4(col_back.x, col_back.y, col_back.z, 1.00F);
        style.Colors[ImGuiCol_ChildBg]      = ImVec4(col_area.x, col_area.y, col_area.z, 1.00F);
        style.Colors[ImGuiCol_PopupBg] =
            ImVec4(col_area.x * 0.8F, col_area.y * 0.8F, col_area.z * 0.8F, 1.00F);
        style.Colors[ImGuiCol_Border]           = ImVec4(col_text.x, col_text.y, col_text.z, 0.30F);
        style.Colors[ImGuiCol_BorderShadow]     = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
        style.Colors[ImGuiCol_FrameBg]          = ImVec4(col_back.x, col_back.y, col_back.z, 1.00F);
        style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(col_main.x, col_main.y, col_main.z, 0.68F);
        style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_TitleBg]          = ImVec4(col_main.x, col_main.y, col_main.z, 1.0F);
        style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(col_main.x, col_main.y, col_main.z, 1.0F);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0F);
        style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(col_area.x, col_area.y, col_area.z, 1.0F);
        style.Colors[ImGuiCol_ScrollbarBg]      = ImVec4(col_area.x, col_area.y, col_area.z, 1.00F);
        style.Colors[ImGuiCol_ScrollbarGrab]    = ImVec4(col_main.x, col_main.y, col_main.z, 0.31F);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(col_main.x, col_main.y, col_main.z, 0.78F);
        style.Colors[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_CheckMark]        = ImVec4(col_text.x, col_text.y, col_text.z, 0.80F);
        style.Colors[ImGuiCol_SliderGrab]       = ImVec4(col_main.x, col_main.y, col_main.z, 0.54F);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_Button]           = ImVec4(col_main.x, col_main.y, col_main.z, 0.44F);
        style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(col_main.x, col_main.y, col_main.z, 0.86F);
        style.Colors[ImGuiCol_ButtonActive]     = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_Header]           = ImVec4(col_main.x, col_main.y, col_main.z, 0.46F);
        style.Colors[ImGuiCol_HeaderActive]     = ImVec4(col_main.x, col_main.y, col_main.z, 0.86F);
        style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_Separator]        = ImVec4(col_main.x, col_main.y, col_main.z, 0.44F);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.86F);
        style.Colors[ImGuiCol_SeparatorActive]  = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_Tab]              = style.Colors[ImGuiCol_Header];
        style.Colors[ImGuiCol_TabHovered]       = style.Colors[ImGuiCol_HeaderHovered];
        style.Colors[ImGuiCol_TabActive]        = style.Colors[ImGuiCol_HeaderActive];
        style.Colors[ImGuiCol_TabUnfocused]     = style.Colors[ImGuiCol_Tab];
        style.Colors[ImGuiCol_TabUnfocusedActive] = style.Colors[ImGuiCol_TabActive];
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(col_main.x, col_main.y, col_main.z, 0.20F);
        style.Colors[ImGuiCol_ResizeGripHovered] =
            ImVec4(col_main.x, col_main.y, col_main.z, 0.78F);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_PlotLines]        = ImVec4(col_text.x, col_text.y, col_text.z, 0.63F);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_PlotHistogram]    = ImVec4(col_text.x, col_text.y, col_text.z, 0.63F);
        style.Colors[ImGuiCol_PlotHistogramHovered] =
            ImVec4(col_main.x, col_main.y, col_main.z, 1.00F);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.43F);
        style.Colors[ImGuiCol_NavHighlight]   = style.Colors[ImGuiCol_HeaderHovered];
    } else if (index == 4) {
        auto& style                            = ImGui::GetStyle();
        style.FrameBorderSize                  = 0;
        style.WindowBorderSize                 = 1;
        style.ItemSpacing                      = {4, 4};
        ImVec4* colors                         = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_Border]                = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.78f, 0.88f, 1.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.44f, 0.44f, 0.44f, 0.35f);
    }
}
