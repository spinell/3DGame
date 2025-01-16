#pragma once
#include "EditorPanel.h"

/// \brief
///
///
class InspectorPanel : public EditorPanel {
public:
    InspectorPanel() = default;

    ~InspectorPanel() = default;

    InspectorPanel(const InspectorPanel&)            = delete;
    InspectorPanel(InspectorPanel&&)                 = delete;
    InspectorPanel& operator=(const InspectorPanel&) = delete;
    InspectorPanel& operator=(InspectorPanel&&)      = delete;

    /// \brief
    void onImGui(bool& isOpen) override;

private:
};
