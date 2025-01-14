#pragma once
#include <string>

namespace Engine {

class Event;

/// @brief
class Layer {
public:
    /// @brief
    /// @param name
    Layer(const std::string& name = "Layer") : mDebugName(name) {}
    virtual ~Layer() {}

    /// @brief
    virtual void onAttach() {}

    /// @brief
    virtual void onDetach() {}

    /// @brief
    virtual void onUpdate(float timeStep) {}

    /// @brief
    virtual void onImGuiRender() {}

    /// @brief
    /// @param event
    virtual bool onEvent(const Event& event) { return false; }

    /// @brief
    /// @return
    const std::string& GetName() const { return mDebugName; }

protected:
    std::string mDebugName;
};

} // namespace Engine
