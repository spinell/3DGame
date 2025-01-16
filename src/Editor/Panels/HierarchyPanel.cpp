#include "HierarchyPanel.h"

#include "../fonts/IconsMaterialDesignIcons.h"

#include <imgui.h>

void HierarchyPanel::onImGui(bool& isOpen) {
    if (ImGui::Begin(ICON_MDI_FILE_TREE " Hierarchy##Hierarchy", &isOpen)) {
    }
    ImGui::End();
}
