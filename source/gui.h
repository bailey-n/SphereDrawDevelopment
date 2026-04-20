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

enum class DrawMenuAction {
    None,
    SelectPoint,
    SelectPolyline,
    SelectPolygon
};

enum class CubemapMenuAction {
    None,
    ImportCubemap,
    ExportCubemap
};

// Result from the menu bar: what action + (optional) chosen path from file dialog
struct MainMenuResult {
    ProjectMenuAction projectAction = ProjectMenuAction::None;
    DrawMenuAction drawAction = DrawMenuAction::None;
    CubemapMenuAction cubemapAction = CubemapMenuAction::None;
    std::string chosenPath;  // empty if user cancelled or no dialog used
    std::string errorMessage;
};

class SphereDrawGUI {
public:
    SphereDrawGUI() = default;

    // Draws the top menu bar and returns what the user clicked + any chosen filepath
    MainMenuResult DrawMainMenuBar(std::string& filepath);

    // Optional status window for feedback + screenshots
    void DrawStatusWindow(bool* open, const std::string& status);
};

#endif // GUI_H
