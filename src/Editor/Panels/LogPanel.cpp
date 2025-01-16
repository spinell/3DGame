#include "LogPanel.h"

#include "imgui.h"

#include "../ImGuiSTL.h"
#include "../ImGuiUtils.h"
#include "../fonts/IconsMaterialDesignIcons.h"

#include <cstddef>
#include <format>

namespace {
// return the color used to display a message for a specific level.
ImVec4 GetRenderColor(LogPanel::LogLevel level) {
    switch (level) {
            // clang-format off
        case LogPanel::LogLevel::Trace:    return {0.75F, 0.75F, 0.75F, 1.00F}; // White-ish gray
        case LogPanel::LogLevel::Info:     return {0.40F, 0.70F, 1.00F, 1.00F}; // Blue
        case LogPanel::LogLevel::Debug:    return {0.00F, 0.50F, 0.50F, 1.00F}; // Cyan
        case LogPanel::LogLevel::Warning:  return {1.00F, 1.00F, 0.00F, 1.00F}; // Yellow
        case LogPanel::LogLevel::Error:    return {1.00F, 0.25F, 0.25F, 1.00F}; // Red
        case LogPanel::LogLevel::Critical: return {0.6F, 0.2F, 0.8F, 1.00F};    // Purple
        default: return {1.00F, 1.00F, 1.00F, 1.00F};
            // clang-format on
    }
}

// return the Icon used to for a specific level.
const char* GetLevelIcon(LogPanel::LogLevel level) {
    switch (level) {
            // clang-format off
        case LogPanel::LogLevel::Trace:   return ICON_MDI_MESSAGE_TEXT;
        case LogPanel::LogLevel::Info:    return ICON_MDI_INFORMATION;
        case LogPanel::LogLevel::Debug:   return ICON_MDI_BUG;
        case LogPanel::LogLevel::Warning: return ICON_MDI_ALERT;
        case LogPanel::LogLevel::Error:   return ICON_MDI_CLOSE_OCTAGON;
        case LogPanel::LogLevel::Critical:return ICON_MDI_ALERT_OCTAGRAM;
        default: return "Unknown name";
            // clang-format on
    }
}

// return the Icon + name used to for a specific level.
const char* GetLevelName(LogPanel::LogLevel level) {
    switch (level) {
            // clang-format off
        case LogPanel::LogLevel::Trace:    return ICON_MDI_MESSAGE_TEXT " Trace";
        case LogPanel::LogLevel::Info:     return ICON_MDI_INFORMATION " Info";
        case LogPanel::LogLevel::Debug:    return ICON_MDI_BUG " Debug";
        case LogPanel::LogLevel::Warning:  return ICON_MDI_ALERT " Warning";
        case LogPanel::LogLevel::Error:    return ICON_MDI_CLOSE_OCTAGON " Error";
        case LogPanel::LogLevel::Critical: return ICON_MDI_ALERT_OCTAGRAM " Critical";
        default:                           return "Unknown name";
            // clang-format on
    }
}

} // namespace

LogPanel::LogPanel() { mMessage.reserve(1000); }

void LogPanel::onImGui(bool& isOpen) {
    if (ImGui::Begin(ICON_MDI_VIEW_LIST " Console###Console", &isOpen)) {
        renderHeader();
        ImGui::Separator();
        renderMessages();
    }
    ImGui::End();
}

void LogPanel::renderHeader() {
    // Button for advanced settings
    {
        const ImGuiUtils::ScopedColor buttonColorScope(ImGuiCol_Button,
                                                       ImVec4(0.0F, 0.0F, 0.0F, 0.0F));
        if (ImGui::Button(ICON_MDI_COGS)) {
            ImGui::OpenPopup("SettingsPopup");
        }
    }
    if (ImGui::BeginPopup("SettingsPopup")) {
        // Checkbox for scrolling lock
        ImGui::Checkbox("Scroll to bottom", &mScrollToBottom);

        // Button to clear the console
        if (ImGui::Button("Clear console")) {
            clear();
            if (ImGui::IsItemHovered()) {
                ImGuiEx::SetTooltip("Clear all logged message");
            }
        }

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    // const float spacing             = ImGui::GetStyle().ItemSpacing.x;
    ImGui::GetStyle().ItemSpacing.x = 2;
    const float levelButtonWidth    = (ImGui::CalcTextSize(GetLevelIcon(LogLevel(1))).x +
                                    (ImGui::GetStyle().FramePadding.x * 2.0F));
    const float levelButtonWidths   = (levelButtonWidth + ImGui::GetStyle().ItemSpacing.x) * 6;
    {
        ImGui::TextUnformatted(ICON_MDI_MAGNIFY);

        ImGui::SameLine();
        mTextFilter.Draw("###ConsoleFilter",
                         ImGui::GetContentRegionAvail().x - (levelButtonWidths));
    }

    ImGui::SameLine();

    ImGui::BeginGroup();
    {
        filterButton(LogLevel::Info);
        ImGui::SameLine();
        filterButton(LogLevel::Warning);
        ImGui::SameLine();
        filterButton(LogLevel::Error);
        ImGui::SameLine();
        filterButton(LogLevel::Trace);
        ImGui::SameLine();
        filterButton(LogLevel::Debug);
        ImGui::SameLine();
        filterButton(LogLevel::Critical);

        ImGuiEx::TextFmt(
            "Info{} Warning{}, Error{}, Debug{}, Trace{}, Critical{}",
            mLogLevelCounter[(int)LogLevel::Info], mLogLevelCounter[(int)LogLevel::Warning],
            mLogLevelCounter[(int)LogLevel::Error], mLogLevelCounter[(int)LogLevel::Debug],
            mLogLevelCounter[(int)LogLevel::Trace], mLogLevelCounter[(int)LogLevel::Critical]);
    }
    ImGui::EndGroup();

    if (!mTextFilter.IsActive()) {
        ImGui::SameLine();
        // ImGuiUtilities::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0F);
        // ImGuiUtilities::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f,
        // ImGui::GetStyle().FramePadding.y));
        ImGui::TextUnformatted("Search...");
    }
}

void LogPanel::renderMessages() {
    // ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;

    ImGuiTableFlags msgTableflags = 0;
    msgTableflags |= ImGuiTableFlags_NoSavedSettings;
    msgTableflags |= ImGuiTableFlags_Borders;
    msgTableflags |= ImGuiTableFlags_SizingStretchProp;
    msgTableflags |= ImGuiTableFlags_ScrollY;
    msgTableflags |= ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("Messages", 3, msgTableflags)) {
        ImGui::TableSetupColumn("Type",
                                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed,
                                0.0F, 0 /*id*/);
        ImGui::TableSetupColumn("Time",
                                ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed,
                                0.0F, 1 /*id*/);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_NoSort, 0.0F, 2 /*id*/);
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableHeadersRow();
        ImGui::TableNextRow();

        // =============================================
        //			Render all the messages
        // =============================================

        auto renderMessage = [this](const Entry& msg) {
            // filter by log level
            if (mFilterLogLevel.at((int)msg.level)) {
                return;
            }

            if (!mTextFilter.PassFilter(msg.message.c_str(), nullptr /*textEnd*/)) {
                return;
            }

            // col 1
            ImGui::TableNextColumn();
            {
                const ImGuiUtils::ScopedColor colorScope(ImGuiCol_Text, GetRenderColor(msg.level));
                ImGui::TextUnformatted(GetLevelIcon(msg.level));
                if (ImGui::IsItemHovered()) {
                    ImGuiEx::SetTooltip("{}", GetLevelName(msg.level));
                }
            }

            // col 2
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(msg.time.c_str());

            // col 3
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(msg.message.c_str());

            ImGui::TableNextRow();
        };

        //
        // render old messages
        //
        for (std::size_t i = mFirstMessageIndex; i < mMessage.size(); i++) {
            const Entry& entry = mMessage[i];
            renderMessage(entry);
        }

        //
        // render new messages
        //
        for (unsigned i = 0; i < mFirstMessageIndex; i++) {
            const Entry& entry = mMessage[i];
            renderMessage(entry);
        }

        //
        // Scroll to the last message if required
        //
        if (mScrollToBottom && ImGui::GetScrollY() < ImGui::GetScrollMaxY()) {
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        }
        ImGui::EndTable();
    }
}

void LogPanel::addMessage(LogLevel logLevel, const std::string& msg, const std::string& time) {
    if (mMessage.size() < mMessage.capacity()) {
        mMessage.emplace_back();
        mMessage.back().level   = logLevel;
        mMessage.back().message = msg;
        mMessage.back().time    = time;
        mFirstMessageIndex++;
    } else {
        if (mFirstMessageIndex == mMessage.size()) {
            mFirstMessageIndex = 0;
        }

        mLogLevelCounter.at(to_underlying(mMessage.at(mFirstMessageIndex).level))--;

        mMessage[mFirstMessageIndex].level   = logLevel;
        mMessage[mFirstMessageIndex].message = msg;
        mFirstMessageIndex++;
    }

    mLogLevelCounter.at(to_underlying(logLevel))++;
    mScrollToBottom = mAutoScrollToBottom;
}

void LogPanel::clear() {
    mMessage.clear();
    for (unsigned int& i : mLogLevelCounter) {
        i = 0;
    }
    mFirstMessageIndex = 0;
}

void LogPanel::filterButton(LogLevel level) {
    // render the button
    {
        const ImVec4 buttonBackGroundColor = ImVec4(0.0F, 0.0F, 0.0F, 0.0F);
        const ImVec4 buttonTextColorPushed = ImVec4(0.5F, 0.5, 0.5F, 0.5F);
        const ImVec4 buttonTextColor =
            isFilterLevelEnable(level) ? buttonTextColorPushed : GetRenderColor(level);

        ImGuiUtils::ScopedColor buttonColorScope;
        buttonColorScope.push(ImGuiCol_Button, buttonBackGroundColor);
        buttonColorScope.push(ImGuiCol_Text, buttonTextColor);
        if (ImGui::Button(GetLevelIcon(level))) {
            toogleFilterLevel(level);
        }
    }

    // render tooltips when move over the button.
    if (ImGui::IsItemHovered()) {
        if (isFilterLevelEnable(level)) {
            ImGuiEx::SetTooltip("Show {} messages", GetLevelName(level));
        } else {
            ImGuiEx::SetTooltip("Hide {} messages", GetLevelName(level));
        }
    }
}
