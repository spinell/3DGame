#pragma once

#include "EditorPanel.h"

#include <imgui.h>

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

/// TODO move it somewhere else
/// Usefulle to cast enum class to int
template <typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

/*
 * \brief
 */
class LogPanel : public EditorPanel {
public:
    enum class LogLevel : std::uint8_t { Trace, Debug, Info, Warning, Error, Critical, Count };

    LogPanel();
    ~LogPanel() = default;

    LogPanel(const LogPanel&)              = delete;
    LogPanel& operator=(const LogPanel&)   = delete;
    LogPanel(const LogPanel&&)             = delete;
    LogPanel&& operator=(const LogPanel&&) = delete;

    void onImGui(bool& isOpen) override;

    void addMessage(LogLevel logLevel, const std::string& msg, const std::string& time);
    void clear();

    [[nodiscard]] const char* getName() const { return "Console"; }

private:
    void renderHeader();
    void renderMessages();

    bool isFilterLevelEnable(LogLevel level) { return mFilterLogLevel.at(to_underlying(level)); }
    void toogleFilterLevel(LogLevel level) {
        mFilterLogLevel.at(to_underlying(level)) = !mFilterLogLevel.at(to_underlying(level));
    }
    void filterButton(LogLevel level);

    struct Entry {
        LogLevel    level = LogLevel::Info;
        std::string message;
        std::string time;
    };

    std::vector<Entry> mMessage; // ring buffer (TODO: replace me with a real ring buffer)
    unsigned           mFirstMessageIndex{}; // fist message to display

    bool mAutoScrollToBottom{true};
    bool mScrollToBottom{false};

    std::array<unsigned, to_underlying(LogLevel::Count)> mLogLevelCounter{};
    std::array<bool, to_underlying(LogLevel::Count)>     mFilterLogLevel{};

    ImGuiTextFilter mTextFilter;
};
