#pragma once

/// \brief
///
///
class EditorPanel {
public:
    EditorPanel() = default;

    ~EditorPanel() = default;

    EditorPanel(const EditorPanel&)            = delete;
    EditorPanel(EditorPanel&&)                 = delete;
    EditorPanel& operator=(const EditorPanel&) = delete;
    EditorPanel& operator=(EditorPanel&&)      = delete;

    /// \brief
    virtual void onImGui(bool& isOpen) = 0;
};
