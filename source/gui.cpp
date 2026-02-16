#include "gui.h"
#include <cstdio>  // std::snprintf
#include <cstring> // std::strcmp

ProjectMenuAction SphereDrawGUI::DrawMainMenuBar(std::string& filepath) {
    ProjectMenuAction action = ProjectMenuAction::None;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Project")) {

            // File path input
            static char fileBuf[512] = {0};

            // Keep the buffer in sync with filepath (if filepath changed elsewhere)
            if (filepath.empty()) {
                if (fileBuf[0] != '\0') fileBuf[0] = '\0';
            } else {
                if (std::strcmp(fileBuf, filepath.c_str()) != 0) {
                    std::snprintf(fileBuf, sizeof(fileBuf), "%s", filepath.c_str());
                }
            }

            ImGui::Text("Project file:");
            ImGui::InputText("##ProjectFile", fileBuf, sizeof(fileBuf));
            filepath = std::string(fileBuf);

            ImGui::Separator();

            if (ImGui::MenuItem("New", "CTRL+N"))  action = ProjectMenuAction::NewProject;
            if (ImGui::MenuItem("Save", "CTRL+S")) action = ProjectMenuAction::SaveProject;
            if (ImGui::MenuItem("Load", "CTRL+O")) action = ProjectMenuAction::LoadProject;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z", false, false)) {} // placeholder
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // placeholder
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    return action;
}

void SphereDrawGUI::DrawStatusWindow(bool* open, const std::string& status) {
    if (!open || !(*open)) return;
    if (status.empty()) return;

    ImGui::Begin("Project Status", open);  // <-- adds an X close button
    ImGui::TextWrapped("%s", status.c_str());
    ImGui::End();
}
