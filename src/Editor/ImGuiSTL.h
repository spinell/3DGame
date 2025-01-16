#pragma once
#ifndef IMGUI_STL_H
#define IMGUI_STL_H

#include <imgui.h>
#include <imgui_internal.h>

#include <format>
#include <string_view>

namespace ImGuiEx {

// \brief Display formated text with ImGui.
//
// This is replacement for ImGui::Text() which used C++ 20 format library
// comparing to ImGui::Text() which use C style variadic paramater.
//
// \param fmt  The formating string.
// \param args The arguments to display inside the string.
template <typename... Args>
IMGUI_API void TextFmt(const std::format_string<Args...>& fmt, const Args&... args) {
    std::string str = std::vformat(fmt.get(), std::make_format_args(args...));
    ImGui::TextUnformatted(str.c_str(), &str[str.size()]);
}

// \brief Display formated tooltips text with ImGui.
//
// This is replacement for ImGui::SetTooltip() which used C++ 20 format library
// comparing to ImGui::Text() which use C style variadic paramater.
//
// \param fmt  The formating string.
// \param args The arguments to display inside the string.
template <typename... Args>
IMGUI_API void SetTooltip(const std::format_string<Args...>& fmt, const Args&... args) {
    if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None)) {
        TextFmt(fmt, args...);
        ImGui::EndTooltip();
    }
}

} // namespace ImGuiEx

#endif // IMGUI_STL_H
