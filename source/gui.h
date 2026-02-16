#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>

enum class ProjectMenuAction {
    None,
    NewProject,
    SaveProject,
    LoadProject
};

class SphereDrawGUI {
public:
    SphereDrawGUI() = default;

    // Draws the top menu bar and returns what the user clicked.
    ProjectMenuAction DrawMainMenuBar(std::string& filepath);

    // Optional status window for feedback + screenshots
    void DrawStatusWindow(bool* open, const std::string& status);
};

#endif //GUI_H
