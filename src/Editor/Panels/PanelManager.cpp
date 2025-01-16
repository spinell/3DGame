#include "PanelManager.h"

void PanelManager::onImGui() {
    for (auto& [id, data] : mPanels) {
        if (data.isOpen) {
            data.panel->onImGui(data.isOpen);
            if (!data.isOpen) {
                //
                // Was close this frame
                //
            }
        }
    }
}
