#include <Engine/KeyCode.h>

namespace Engine {

const char* toString(MouseButton button) {
    switch (button) {
            // clang-format off
        case MouseButton::Left:   return "Left";
        case MouseButton::Right:  return "Right";
        case MouseButton::Middle: return "Middle";
        case MouseButton::X4:     return "X4";
        case MouseButton::X5:     return "X5";
        case MouseButton::X6:     return "X6";
        case MouseButton::X7:     return "X7";
        case MouseButton::X8:     return "X8";
        default:                  return "Unknow";
            // clang-format on
    }
}

std::string toString(MouseButtonFlag flag) {
    using Type                       = std::underlying_type_t<MouseButtonFlag>;
    Type        underlying_type_flag = static_cast<Type>(flag);
    std::string str;

    Type index = 0;
    while (underlying_type_flag) {
        if (underlying_type_flag & 1) {
            if (!str.empty()) str.append("|");

            MouseButtonFlag flagToTest = static_cast<MouseButtonFlag>(1U << index);
            if (flagToTest & MouseButtonFlag::Left) {
                str += "Left";
            } else if (flagToTest & MouseButtonFlag::Right) {
                str += "Right";
            } else if (flagToTest & MouseButtonFlag::Middle) {
                str += "Middle";
            } else if (flagToTest & MouseButtonFlag::X4) {
                str += "X4";
            } else if (flagToTest & MouseButtonFlag::X5) {
                str += "X5";
            } else if (flagToTest & MouseButtonFlag::X6) {
                str += "X6";
            } else if (flagToTest & MouseButtonFlag::X7) {
                str += "X7";
            } else if (flagToTest & MouseButtonFlag::X8) {
                str += "X8";
            }
        }
        ++index;
        underlying_type_flag >>= 1;
    }
    return str;
}

std::string toString(KeyModFlag flag) {
    using Type                       = std::underlying_type_t<KeyModFlag>;
    Type        underlying_type_flag = static_cast<Type>(flag);
    std::string str;

    Type index = 0;
    while (underlying_type_flag) {
        if (underlying_type_flag & 1) {
            if (!str.empty()) str.append("|");

            KeyModFlag flagToTest = static_cast<KeyModFlag>(1U << index);
            if (flagToTest & KeyModFlag::LeftShift) {
                str += "LeftShift";
            } else if (flagToTest & KeyModFlag::RightShift) {
                str += "RightShift";
            } else if (flagToTest & KeyModFlag::LeftCtrl) {
                str += "LeftCtrl";
            } else if (flagToTest & KeyModFlag::RightCtrl) {
                str += "RightCtrl";
            } else if (flagToTest & KeyModFlag::LeftAlt) {
                str += "LeftAlt";
            } else if (flagToTest & KeyModFlag::RightAlt) {
                str += "RightAlt";
            } else if (flagToTest & KeyModFlag::LeftGUI) {
                str += "LeftGUI";
            } else if (flagToTest & KeyModFlag::RightGUI) {
                str += "RightGUI";
            } else if (flagToTest & KeyModFlag::NumLock) {
                str += "NumLock";
            } else if (flagToTest & KeyModFlag::CapsLock) {
                str += "CapsLock";
            } else if (flagToTest & KeyModFlag::Scroll) {
                str += "Scroll";
            }
        }
        ++index;
        underlying_type_flag >>= 1;
    }
    return str;
}

} // namespace Engine
