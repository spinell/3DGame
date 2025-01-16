#pragma once
#ifndef IMGUI_UTILS_H
#define IMGUI_UTILS_H

#include <imgui.h>

namespace ImGuiUtils {

/*
 *
 */
class ScopedStyle {
public:
    template <typename T>
    ScopedStyle(ImGuiStyleVar styleVar, T value) {
        ImGui::PushStyleVar(styleVar, value);
    }

    ScopedStyle(const ScopedStyle&)           = delete;
    ScopedStyle operator=(const ScopedStyle&) = delete;

    ScopedStyle(ScopedStyle&&)           = delete;
    ScopedStyle operator=(ScopedStyle&&) = delete;

    ~ScopedStyle() { ImGui::PopStyleVar(); }
};

/*
 *
 */
class ScopedColor {
public:
    ScopedColor() = default;

    template <typename T>
    ScopedColor(ImGuiCol idx, const T& value) {
        push(idx, value);
    }

    ~ScopedColor() {
        if (mNbPush > 0) {
            ImGui::PopStyleColor(mNbPush);
        }
    }

    ScopedColor(const ScopedColor&)           = delete;
    ScopedColor operator=(const ScopedColor&) = delete;

    ScopedColor(ScopedColor&&)           = delete;
    ScopedColor operator=(ScopedColor&&) = delete;

    template <typename T>
    void push(ImGuiCol idx, const T& value) {
        mNbPush++;
        ImGui::PushStyleColor(idx, value);
    }

private:
    int mNbPush = 0;
};
}; // namespace ImGuiUtils

#endif // !IMGUI_UTILS_H
