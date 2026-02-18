#include "gui.h"
#include <cstdio>
#include <cstring>
#include <nfd.h>

static std::string ensure_json_extension(const std::string& path) {
    // If user types "foo" or "foo.spheredraw", we still want a JSON project file.
    // Keep it simple: if it doesn't end with .json, append .json.
    if (path.size() >= 5) {
        std::string tail = path.substr(path.size() - 5);
        for (auto& c : tail) c = (char)tolower(c);
        if (tail == ".json") return path;
    }
    return path + ".json";
}

ProjectMenuResult SphereDrawGUI::DrawMainMenuBar(std::string& filepath) {
    ProjectMenuResult result;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Project")) {

            // File path input
            static char fileBuf[512] = {0};

            // Keep buffer in sync with filepath
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

            // NFD filter: only show JSON by default
            const nfdfilteritem_t filters[1] = { {"SphereDraw Project", "json"} };

            if (ImGui::MenuItem("New", "CTRL+N")) {
                nfdchar_t* outPath = nullptr;
                nfdresult_t r = NFD_SaveDialog(&outPath, filters, 1, nullptr, "project.json");

                if (r == NFD_OKAY && outPath) {
                    result.action = ProjectMenuAction::NewProject;
                    result.chosenPath = ensure_json_extension(outPath);
                    filepath = result.chosenPath;
                    std::snprintf(fileBuf, sizeof(fileBuf), "%s", filepath.c_str());
                    NFD_FreePath(outPath);
                } else if (r == NFD_ERROR) {
                    result.errorMessage = std::string("File dialog error: ") +
                                          (NFD_GetError() ? NFD_GetError() : "Unknown error");
                }
                // if CANCEL: do nothing (action stays None)
            }

            if (ImGui::MenuItem("Save", "CTRL+S")) {
                // If we already have a filepath, we can save immediately.
                // If not, do Save As dialog.
                if (!filepath.empty()) {
                    result.action = ProjectMenuAction::SaveProject;
                } else {
                    nfdchar_t* outPath = nullptr;
                    nfdresult_t r = NFD_SaveDialog(&outPath, filters, 1, nullptr, "project.json");

                    if (r == NFD_OKAY && outPath) {
                        result.action = ProjectMenuAction::SaveProject;
                        result.chosenPath = ensure_json_extension(outPath);
                        filepath = result.chosenPath;
                        std::snprintf(fileBuf, sizeof(fileBuf), "%s", filepath.c_str());
                        NFD_FreePath(outPath);
                    } else if (r == NFD_ERROR) {
                        result.errorMessage = std::string("File dialog error: ") +
                                              (NFD_GetError() ? NFD_GetError() : "Unknown error");
                    }
                    // if CANCEL: do nothing (action stays None)
                }
            }

            if (ImGui::MenuItem("Load", "CTRL+O")) {
                nfdchar_t* outPath = nullptr;
                nfdresult_t r = NFD_OpenDialog(&outPath, filters, 1, nullptr);

                if (r == NFD_OKAY && outPath) {
                    result.action = ProjectMenuAction::LoadProject;
                    result.chosenPath = outPath;
                    filepath = result.chosenPath;
                    std::snprintf(fileBuf, sizeof(fileBuf), "%s", filepath.c_str());
                    NFD_FreePath(outPath);
                } else if (r == NFD_ERROR) {
                    result.errorMessage = std::string("File dialog error: ") +
                                          (NFD_GetError() ? NFD_GetError() : "Unknown error");
                }
                // if CANCEL: do nothing (action stays None)
            }


            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z", false, false)) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    return result;
}

void SphereDrawGUI::DrawStatusWindow(bool* open, const std::string& status) {
    if (!open || !(*open)) return;
    if (status.empty()) return;

    ImGui::Begin("Project Status", open);
    ImGui::TextWrapped("%s", status.c_str());
    ImGui::End();
}
