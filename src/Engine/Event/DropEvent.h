#pragma once
#include "Event.h"

#include <format>
#include <string>

namespace Engine {

/// @brief Event trigerred when user drop files or folders into a window.
///        This event markthe beginning of a sequence of one ormultiple \p DropFileEvent.
///        One \p DropFileEvent will be triggered for each file.
class DropBeginEvent : public Event {
    EVENT_CLASS_TYPE(DropBegin)
    EVENT_CLASS_CATEGORY(EventCategoryDrop)
};

/// @brief Event trigerred when all \p DropFileEvent has been received..
class DropCompleteEvent : public Event {
    EVENT_CLASS_TYPE(DropComplete)
    EVENT_CLASS_CATEGORY(EventCategoryDrop)
};

/// @brief Event trigerred for each files / folder the user drop into the window.
///        This event is always preceded by \p DropBeginEvent and fallow by \p DropCompleteEvent.
class DropFileEvent : public Event {
public:
    EVENT_CLASS_TYPE(DropFile)
    EVENT_CLASS_CATEGORY(EventCategoryDrop)

    DropFileEvent(std::string path) : mPath{path} {}

    std::string getPath() const { return mPath; }

    std::string toString() const override { return std::format("{}: {}", getName(), mPath); }

private:
    std::string mPath;
};

} // namespace Engine
