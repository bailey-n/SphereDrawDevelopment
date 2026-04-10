#ifndef APPLICATION_H
#define APPLICATION_H

#include "opengl_include.h"
#include <iostream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <deque>
#include "application_action.h"
#include "shader_manager.h"
#include "camera.h"
#include "shapes.h"
#include "gui.h"
#include "project.h"
#include "project_io.h"
#include "cubemap.h"
#include "cubeface_mesh.h"

class Application {
    const static int w_width;
    const static int w_height;
    static GLFWwindow* window;
    static bool initialized;
    static bool nfd_initialized;
    static bool gui_change;
    static std::deque<AppAction> event_queue;
    // Maps key, modifier bits to an associated shortcut action
    static std::map<std::pair<int, int>, AppAction> press_key_actions;
    static std::map<std::pair<int, int>, AppAction> release_key_actions;

    bool running = false;
    double deltaTime = 0.0;
    double previousFrameTime = 0.0f;
    shaderManager shader_manager;
    ImDrawData* draw_data = nullptr;

    // Main program
    Camera camera;
    SphereDrawGUI app_gui;
    Cubemap renderer;

    std::optional<CubeFaceMesh> test_mesh;

    // Project state (for Save/Load/New)
    Project project;
    std::string project_filepath = "project_files/out/project.json";
    std::string project_status;
    bool show_project_status = false;

    // APPLICATION INIT
    static bool init_glfw();
    static void init_imgui();
    static void bind_input_callbacks();
    static void set_gl_preferences();
    static void setup_key_bindings();

    // Event callbacks
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods);
    static void windowSizeCallback(GLFWwindow* win, int width, int height);

    // Internal struct to hold state values
    struct State {
        enum CameraRotation : unsigned char {
            OrbitLeft = 1,
            OrbitRight = 2,
            OrbitUp = 4,
            OrbitDown = 8,
            RotateLeft = 16,
            RotateRight = 32,
            ZoomIn = 64,
            ZoomOut = 128
        };
        unsigned char camera_motion = 0;
        double camera_radius = 2.0;
        float camera_rotate_speed = 1.0f;
        double camera_zoom_speed = 1.0f;

        enum DrawMode : unsigned char {
            None = 0,
            Point = 1,
            Polyline = 2,
            Polygon = 3
        };
        DrawMode draw_mode = None;
    };
    State state;

    // Point tool state
    struct PointToolState {
        bool show_panel = false;          // show point tool UI panel
        bool armed_for_placement = false; // next sphere click places a point
        glm::vec4 color = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
        float size = 0.007f;              // matches PointPrimitive default
        ImVec2 panel_size = ImVec2(0.0f, 0.0f);
    };

    PointToolState point_tool;

    //Polyline tool state
    struct PolylineToolState {
        bool show_panel = false;
        bool armed_for_placement = false;
        glm::vec4 color = glm::vec4(0.20f, 0.75f, 1.0f, 1.0f);
        float width = 0.007f;
        std::vector<glm::vec3> verts;
        ImVec2 panel_size = ImVec2(0.0f, 0.0f);
    };

    PolylineToolState polyline_tool;

    // Polygon tool state
    struct PolygonToolState {
        bool show_panel = false;
        bool armed_for_placement = false;
        glm::vec4 color = glm::vec4(0.95f, 0.65f, 0.20f, 0.85f);
        std::vector<glm::vec3> verts;
        ImVec2 panel_size = ImVec2(0.0f, 0.0f);
    };

    PolygonToolState polygon_tool;

    struct OutlinerState {
        bool collapsed = false;
        uint32_t selected_primitive_id = 0;
        uint32_t name_buffer_primitive_id = 0;
        char name_buffer[256] = {};
        ImVec2 panel_size = ImVec2(0.0f, 0.0f);
    };
    OutlinerState outliner;

public:
    static bool init();
    // APPLICATION CORE
    Application();
    ~Application();

    // APPLICATION MAINLOOP
    void mainloop();

private:
    void handle_events();
    void update_window();
    void handle_event(const AppAction& action);
    void render_frame();
    void refreshPolylinePreview();
    void refreshPolygonPreview();

    void undoActivePolylineVertex();
    void undoActivePolygonVertex();
    void finishActivePolyline();
    void finishActivePolygon();
    void cancelActivePolyline();
    void cancelActivePolygon();
};

#endif //APPLICATION_H