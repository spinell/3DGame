#include "InspectorPanel.h"

#include "../fonts/IconsMaterialDesignIcons.h"

#include <imgui.h>

void InspectorPanel::onImGui(bool& isOpen) {
    if (ImGui::Begin(ICON_MDI_INFORMATION " Inspector##Inspector", &isOpen)) {
    }
    ImGui::End();
}
