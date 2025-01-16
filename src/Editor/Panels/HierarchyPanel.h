#pragma once
#include "EditorPanel.h"

/// \brief
///
///
class HierarchyPanel : public EditorPanel {
public:
    HierarchyPanel() = default;

    ~HierarchyPanel() = default;

    HierarchyPanel(const HierarchyPanel&)            = delete;
    HierarchyPanel(HierarchyPanel&&)                 = delete;
    HierarchyPanel& operator=(const HierarchyPanel&) = delete;
    HierarchyPanel& operator=(HierarchyPanel&&)      = delete;

    /// \brief
    void onImGui(bool& isOpen) override;

private:
};
