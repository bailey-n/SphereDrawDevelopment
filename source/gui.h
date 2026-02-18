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

// Result from the menu bar: what action + (optional) chosen path from file dialog
struct ProjectMenuResult {
    ProjectMenuAction action = ProjectMenuAction::None;
    std::string chosenPath;  // empty if user cancelled or no dialog used
    std::string errorMessage;
};

class SphereDrawGUI {
public:
    SphereDrawGUI() = default;

    // Draws the top menu bar and returns what the user clicked + any chosen filepath
    ProjectMenuResult DrawMainMenuBar(std::string& filepath);

    // Optional status window for feedback + screenshots
    void DrawStatusWindow(bool* open, const std::string& status);
};

#endif // GUI_H
