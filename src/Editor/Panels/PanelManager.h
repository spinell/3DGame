#pragma once
#include "EditorPanel.h"

#include <string>
#include <unordered_map>

/// @brief
class PanelManager {
    struct PanelData {
        EditorPanel* panel{};
        std::string  id;
        std::string  displayName;
        bool         isOpen = true;
    };

public:
    PanelManager() = default;

    ~PanelManager() {
        for (auto [id, data] : mPanels) {
            delete data.panel;
        }
        mPanels.clear();
    };

    template <class TPanel>
    TPanel* addPanel(std::string id) {
        TPanel* panel = new TPanel();

        PanelData dData;
        dData.id    = id;
        dData.panel = panel;
        if (auto it = mPanels.emplace(id, dData); !it.second) {
            return nullptr;
        }
        return panel;
    }

    std::unordered_map<std::string, PanelData>& getPanels() { return mPanels; }

    void onImGui();

private:
    std::unordered_map<std::string, PanelData> mPanels;
};
