#include "application.h"
#include <chrono>
#include <thread>
#include <cmath>
#include <filesystem>
#include <random>
#include <nfd.h>
#include "shapes.h"


// ################################################## //


// Static variable initialization
const int Application::w_width = 1200; //sean changed for ui testing (OG = 1200)
const int Application::w_height = 900; //sean changed for ui testing (OG = 900)
GLFWwindow* Application::window = nullptr;
bool Application::initialized = false;
bool Application::gui_change = true;
bool Application::nfd_initialized = false;
std::deque<AppAction> Application::event_queue = {};
std::map<std::pair<int, int>, AppAction> Application::press_key_actions = {};
std::map<std::pair<int, int>, AppAction> Application::release_key_actions = {};

// APPLICATION INIT
bool Application::init() {
    if (!init_glfw()) return false;
    bind_input_callbacks();
    set_gl_preferences();
    setup_key_bindings();
    shaderManager::set_shader_root_directory("shaders/");
    init_imgui();
    // Initialize NFD (not fatal if it fails)
    nfd_initialized = (NFD_Init() == NFD_OKAY);
    if (!nfd_initialized) {
        std::cerr << "NFD_Init failed: " << NFD_GetError() << "\n";
    }
    initialized = true;
    return initialized;
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 3.0f; // font size, currently global probably should change to specifics later on
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

bool Application::init_glfw() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // FOR MAC

    window = glfwCreateWindow(w_width, w_height, "ogl-app", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to open window\n" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }

    glViewport(0, 0, w_width, w_height);
    return true;
}

void Application::bind_input_callbacks() {
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, width / 2.0, height / 2.0);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
}

void Application::set_gl_preferences() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    // glEnable(GL_CULL_FACE);
    glPointSize(10.0f);
    glLineWidth(4.0f);
}

void Application::setup_key_bindings() {
    // Camera rotation actions
    press_key_actions.emplace(std::pair(GLFW_KEY_W, 0), CAMERA_SET_ORBIT_UP);
    press_key_actions.emplace(std::pair(GLFW_KEY_S, 0), CAMERA_SET_ORBIT_DOWN);
    press_key_actions.emplace(std::pair(GLFW_KEY_A, 0), CAMERA_SET_ORBIT_LEFT);
    press_key_actions.emplace(std::pair(GLFW_KEY_D, 0), CAMERA_SET_ORBIT_RIGHT);
    press_key_actions.emplace(std::pair(GLFW_KEY_Q, 0), CAMERA_SET_ROTATE_CL);
    press_key_actions.emplace(std::pair(GLFW_KEY_E, 0), CAMERA_SET_ROTATE_CCL);

    release_key_actions.emplace(std::pair(GLFW_KEY_W, 0), CAMERA_UNSET_ORBIT_UP);
    release_key_actions.emplace(std::pair(GLFW_KEY_S, 0), CAMERA_UNSET_ORBIT_DOWN);
    release_key_actions.emplace(std::pair(GLFW_KEY_A, 0), CAMERA_UNSET_ORBIT_LEFT);
    release_key_actions.emplace(std::pair(GLFW_KEY_D, 0), CAMERA_UNSET_ORBIT_RIGHT);
    release_key_actions.emplace(std::pair(GLFW_KEY_Q, 0), CAMERA_UNSET_ROTATE_CL);
    release_key_actions.emplace(std::pair(GLFW_KEY_E, 0), CAMERA_UNSET_ROTATE_CCL);

    // Other camera actions
    press_key_actions.emplace(std::pair(GLFW_KEY_Z, 0), CAMERA_SET_ZOOM_IN);
    press_key_actions.emplace(std::pair(GLFW_KEY_X, 0), CAMERA_SET_ZOOM_OUT);

    release_key_actions.emplace(std::pair(GLFW_KEY_Z, 0), CAMERA_UNSET_ZOOM_IN);
    release_key_actions.emplace(std::pair(GLFW_KEY_X, 0), CAMERA_UNSET_ZOOM_OUT);

    release_key_actions.emplace(std::pair(GLFW_KEY_COMMA, 0), CAMERA_DECREASE_ROTATE_SPEED);
    release_key_actions.emplace(std::pair(GLFW_KEY_PERIOD, 0), CAMERA_INCREASE_ROTATE_SPEED);
    release_key_actions.emplace(std::pair(GLFW_KEY_R, 0), CAMERA_RESET);

    // Draw mode actions
    release_key_actions.emplace(std::pair(GLFW_KEY_H, 0), DRAW_MODE_ROTATE);
}

// ################################################## //

// APPLICATION CORE
Application::Application() :
camera({2.0f, 0.0f, 0.0f},
       {-2.0f, -0.0f, 0.0f},
       {0.0f, 1.0f, 0.0f},
       glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f)
       ){
    if (!initialized) return;
    renderer.init();
    camera.update_window(window);
    // connect project memory -> render cubemap pipeline
    project.attachCubemap(&renderer);
    // ensure renderer matches current project state on startup
    project.rebuildAttachedCubemapFromProject();
}

Application::~Application() {
    if (nfd_initialized) {
        NFD_Quit();
        nfd_initialized = false;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

// ################################################## //

void Application::keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    // if(ImGui::GetIO().WantCaptureKeyboard) return;
    if (action == GLFW_PRESS) {
        if (press_key_actions.contains({key, mods})) {
            event_queue.emplace_back(press_key_actions.at({key, mods}));
        }
    }
    else if (action == GLFW_RELEASE) {
        if (release_key_actions.contains({key, mods})) {
            event_queue.emplace_back(release_key_actions.at({key, mods}));
        }
    }
}

void Application::mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
    if(ImGui::GetIO().WantCaptureMouse) {
        gui_change = true;
        return;
    };
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        event_queue.emplace_back(CLICK_SPHERE);
    }
}

void Application::windowSizeCallback(GLFWwindow *win, int width, int height) {
    event_queue.emplace_back(WINDOW_RESIZE);
}

// ################################################## //

// APPLICATION MAINLOOP
void Application::mainloop() {
    constexpr double rotations_per_second = 0.5;
    constexpr unsigned int FPS = 60;
    const double TARGET_FRAME_TIME = 1.0 / (double)FPS;
    double angle = 0.0;
    constexpr float CAMERA_DIST_SCALE = 1.0f;

    running = true;
    while (running) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        deltaTime = glfwGetTime() - previousFrameTime;
        previousFrameTime = glfwGetTime();
        if (deltaTime > 17.0) {
            std::cout << "Spike of " << deltaTime * 1000 <<  "ms" << "\n";
        }

        // Events
        glfwPollEvents();
        handle_events();
        update_window();

        // Draw
        render_frame();

        // Sleep to hit target fps
        auto frame_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frame_time = frame_end - frame_start;
        double sleep_duration = TARGET_FRAME_TIME - frame_time.count();
        if (sleep_duration > 0.0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_duration));
        }
    }
    glfwTerminate();
}

void Application::handle_events() {
    while (!event_queue.empty()) {
        auto event = event_queue.front();
        handle_event(event);
        event_queue.pop_front();
    }
    if (glfwWindowShouldClose(window)) {
        running = false;
    }
}

void Application::handle_event(const AppAction &action) {
    double x_pos, y_pos;
    std::pair<float, float> click_coords = {0.0f, 0.0f};
    switch(action) {
        // ######## UTILITY ######## //
        case NONE:
            break;
        case CLOSE:
            running = false;
            break;

        // ######## CAMERA MOTION SET ######## //
        case CAMERA_SET_ORBIT_UP:
            state.camera_motion |= State::CameraRotation::OrbitUp;
            break;
        case CAMERA_SET_ORBIT_DOWN:
            state.camera_motion |= State::CameraRotation::OrbitDown;
            break;
        case CAMERA_SET_ORBIT_LEFT:
            state.camera_motion |= State::CameraRotation::OrbitLeft;
            break;
        case CAMERA_SET_ORBIT_RIGHT:
            state.camera_motion |= State::CameraRotation::OrbitRight;
            break;
        case CAMERA_SET_ROTATE_CL:
            state.camera_motion |= State::CameraRotation::RotateLeft;
            break;
        case CAMERA_SET_ROTATE_CCL:
            state.camera_motion |= State::CameraRotation::RotateRight;
            break;
        case CAMERA_SET_ZOOM_IN:
            state.camera_motion |= State::CameraRotation::ZoomIn;
            break;
        case CAMERA_SET_ZOOM_OUT:
            state.camera_motion |= State::CameraRotation::ZoomOut;
            break;

        // ######## CAMERA MOTION UNSET ######## //
        case CAMERA_UNSET_ORBIT_UP:
            state.camera_motion &= ~State::CameraRotation::OrbitUp;
            break;
        case CAMERA_UNSET_ORBIT_DOWN:
            state.camera_motion &= ~State::CameraRotation::OrbitDown;
            break;
        case CAMERA_UNSET_ORBIT_LEFT:
            state.camera_motion &= ~State::CameraRotation::OrbitLeft;
            break;
        case CAMERA_UNSET_ORBIT_RIGHT:
            state.camera_motion &= ~State::CameraRotation::OrbitRight;
            break;
        case CAMERA_UNSET_ROTATE_CL:
            state.camera_motion &= ~State::CameraRotation::RotateLeft;
            break;
        case CAMERA_UNSET_ROTATE_CCL:
            state.camera_motion &= ~State::CameraRotation::RotateRight;
            break;
        case CAMERA_UNSET_ZOOM_IN:
            state.camera_motion &= ~State::CameraRotation::ZoomIn;
            break;
        case CAMERA_UNSET_ZOOM_OUT:
            state.camera_motion &= ~State::CameraRotation::ZoomOut;
            break;

        case CAMERA_INCREASE_ROTATE_SPEED:
            state.camera_rotate_speed *= 2.0;
            break;
        case CAMERA_DECREASE_ROTATE_SPEED:
            state.camera_rotate_speed /= 2.0;
            break;

        // ######## CAMERA UTILITY ######## //
        case CAMERA_RESET:
            state.camera_rotate_speed = 1.0f;
            state.camera_radius = 2.0;
            state.camera_zoom_speed = 1.0f;
            camera.set_position(glm::vec3(2.0f, 0.0f, 0.0f));
            camera.set_up(glm::vec3(0.0f, 1.0f, 0.0f));
            break;

        // ######## SPHERE INTERACTION ######## //
        case CLICK_SPHERE:
            glfwGetCursorPos(window, &x_pos, &y_pos);
            click_coords = sphere_click_lat_lon(
                    camera.get_position(),
                    camera.get_up(),
                    1.0f,
                    glm::vec2((float)x_pos, (float)y_pos),
                    (float)w_width, (float)w_height,
                    glm::radians(60.0)
            );
            if (std::isnan(click_coords.first)) {
                std::cout << "latitude: NaN\nlongitude: NaN" << std::endl;
                break;
            }

            std::cout << "latitude: " << glm::degrees(click_coords.first)
                      << "\nlongitude: " << glm::degrees(click_coords.second) << std::endl;

            switch (state.draw_mode) {
                case State::DrawMode::Point:
                {
                    // Only place a point if user has armed the point tool
                    if (!point_tool.armed_for_placement) {
                        break;
                    }

                    // Convert clicked lat/lon (radians) to xyz on sphere radius 1.0
                    glm::vec3 pos = lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);

                    // Create point primitive with a new project ID
                    uint32_t newId = project.nextPrimitiveID();
                    auto point = std::make_unique<PointPrimitive>(newId);
                    point->p = pos;
                    point->color = point_tool.color;
                    point->size = point_tool.size;

                    // Add to project (should also update renderer if project pipeline is connected)
                    project.addPrimitiveToDefaultLayer(std::move(point));

                    // Feedback + disarm (single placement)
                    project_status = "Placed point ID " + std::to_string(newId);
                    show_project_status = true;
                    point_tool.armed_for_placement = false;

                    break;
                }

                case State::DrawMode::Polyline:
                    break;

                case State::DrawMode::Polygon:
                    break;

                case State::DrawMode::None:
                default:
                    break;
            }

            break;

        // ######## DRAW MODE ######## //
        case DRAW_MODE_ROTATE:
            switch (state.draw_mode) {
                case State::DrawMode::None:
                    state.draw_mode = State::DrawMode::Point;
                    break;
                case State::DrawMode::Point:
                    state.draw_mode = State::DrawMode::Polyline;
                    break;
                case State::DrawMode::Polyline:
                    state.draw_mode = State::DrawMode::Polygon;
                    break;
                case State::DrawMode::Polygon:
                    state.draw_mode = State::DrawMode::None;
                    break;
                default:
                    state.draw_mode = State::DrawMode::None;
                    break;
            }
            std::cout << "New draw state: " << static_cast<int>(state.draw_mode) << std::endl;
            break;

        case WINDOW_RESIZE:
            camera.update_window(window);
            break;

        default:
            break;
    }
}

void Application::update_window() {
    // Camera rotation animation
    if ((state.camera_motion & State::CameraRotation::OrbitLeft)
        && !(state.camera_motion & State::CameraRotation::OrbitRight)) {
        camera.rotate_position_left(state.camera_rotate_speed * (float)deltaTime);
    }
    else if ((state.camera_motion & State::CameraRotation::OrbitRight)
            && !(state.camera_motion & State::CameraRotation::OrbitLeft)) {
        camera.rotate_position_right(state.camera_rotate_speed * (float)deltaTime);
    }
    if ((state.camera_motion & State::CameraRotation::OrbitUp)
        && !(state.camera_motion & State::CameraRotation::OrbitDown)) {
        camera.rotate_position_up(state.camera_rotate_speed * (float)deltaTime);
    }
    else if ((state.camera_motion & State::CameraRotation::OrbitDown)
             && !(state.camera_motion & State::CameraRotation::OrbitUp)) {
        camera.rotate_position_down(state.camera_rotate_speed * (float)deltaTime);
    }
    if ((state.camera_motion & State::CameraRotation::RotateLeft)
        && !(state.camera_motion & State::CameraRotation::RotateRight)) {
        camera.rotate_up_left(state.camera_rotate_speed * (float)deltaTime);
    }
    else if ((state.camera_motion & State::CameraRotation::RotateRight)
             && !(state.camera_motion & State::CameraRotation::RotateLeft)) {
        camera.rotate_up_right(state.camera_rotate_speed * (float)deltaTime);
    }
    if ((state.camera_motion & State::CameraRotation::ZoomIn)
        && !(state.camera_motion & State::CameraRotation::ZoomOut)) {
        // Zoom exponentially
        state.camera_radius = exp2(log2(state.camera_radius - 1.0) + state.camera_zoom_speed * deltaTime) + 1.0;
        glm::vec3 pos_norm = glm::normalize(camera.get_position());
        camera.set_position((float)state.camera_radius * pos_norm);

        // std::cout << "Camera radius: " << state.camera_radius << std::endl;
    }
    else if ((state.camera_motion & State::CameraRotation::ZoomOut)
        && !(state.camera_motion & State::CameraRotation::ZoomIn)) {
        // Zoom exponentially
        state.camera_radius = exp2(log2(state.camera_radius - 1.0) - state.camera_zoom_speed * deltaTime) + 1.0;
        glm::vec3 pos_norm = glm::normalize(camera.get_position());
        camera.set_position((float)state.camera_radius * pos_norm);

        // std::cout << "Camera radius: " << state.camera_radius << std::endl;
    }
}

void Application::render_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // // ImGui::ShowDemoWindow();
    MainMenuResult menu = app_gui.DrawMainMenuBar(project_filepath);
    if (!menu.errorMessage.empty()) {
        project_status = menu.errorMessage;
        show_project_status = true;
    }

    // Handle draw tool selection (UI state change)
    switch (menu.drawAction) {
        case DrawMenuAction::SelectPoint:
            state.draw_mode = State::DrawMode::Point;
            point_tool.show_panel = true;
            point_tool.armed_for_placement = false; // user still needs to click "Place Point"
            project_status = "Draw tool selected: Point";
            show_project_status = true;
            break;

        case DrawMenuAction::SelectPolyline:
            state.draw_mode = State::DrawMode::Polyline;
            point_tool.show_panel = false;          // hide point panel when switching tools
            point_tool.armed_for_placement = false;
            project_status = "Draw tool selected: Polyline";
            show_project_status = true;
            break;

        case DrawMenuAction::SelectPolygon:
            state.draw_mode = State::DrawMode::Polygon;
            point_tool.show_panel = false;          // hide point panel when switching tools
            point_tool.armed_for_placement = false;
            project_status = "Draw tool selected: Polygon";
            show_project_status = true;
            break;

        case DrawMenuAction::None:
            break;
    }


    //Handle project I/O seperately
    ProjectMenuAction action = menu.projectAction;
    try {
        if (action == ProjectMenuAction::NewProject) {
            // If user cancelled dialog, do nothing.
            if (project_filepath.empty()) {
                throw std::runtime_error("New project cancelled (no file selected).");
            }

            // Reset + save blank project to the chosen path
            project = Project();
            //connect to pipeline
            project.attachCubemap(&renderer);
            project.rebuildAttachedCubemapFromProject(); // clears renderer to match blank project

            std::filesystem::path p(project_filepath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }

            SaveProjectToFile(project, project_filepath);

            project_status = "New project created and saved to:\n" + project_filepath;
            show_project_status = true;
        }
        else if (action == ProjectMenuAction::SaveProject) {
            if (!menu.chosenPath.empty()) {
                project_filepath = menu.chosenPath;
            }
            if (project_filepath.empty()) {
                throw std::runtime_error("No project file path set. Use Project > Save to choose a file.");
            }

            std::filesystem::path p(project_filepath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }

            SaveProjectToFile(project, project_filepath);

            project_status = "Saved project to:\n" + project_filepath;
            show_project_status = true;
        }
        else if (action == ProjectMenuAction::LoadProject) {
            if (project_filepath.empty()) {
                throw std::runtime_error("Load cancelled (no file selected).");
            }

            if (!std::filesystem::exists(project_filepath)) {
                throw std::runtime_error("File does not exist:\n" + project_filepath);
            }

            project = LoadProjectFromFile(project_filepath);
            // connect to pipeline
            project.attachCubemap(&renderer);
            project.rebuildAttachedCubemapFromProject();

            project_status = "Loaded project from:\n" + project_filepath;
            show_project_status = true;
        }
    }
    catch (const std::exception& e) {
        project_status = std::string("Project I/O error:\n") + e.what();
        show_project_status = true;
    }


    // Status window (closable)
    app_gui.DrawStatusWindow(&show_project_status, project_status);

    // Point tool panel (first interactive drawing UI)

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Position just below main menu bar
    const float menu_bar_height = ImGui::GetFrameHeight();
    ImVec2 panel_pos = ImVec2(viewport->WorkPos.x + 8.0f, viewport->WorkPos.y + menu_bar_height + 8.0f);
    ImVec2 panel_size = ImVec2(600.0f, 0.0f); // width fixed, height auto

    ImGui::SetNextWindowPos(panel_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);

    ImGuiWindowFlags pointToolFlags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse;


    if (point_tool.show_panel) {
        if (ImGui::Begin("Point Tool", &point_tool.show_panel, pointToolFlags)) {
            ImGui::Text("Point placement");
            ImGui::Separator();

            ImGui::Text("Color");
            ImGui::SameLine();
            ImGui::ColorButton("##PointColorPreview", ImVec4(
                    point_tool.color.r, point_tool.color.g, point_tool.color.b, point_tool.color.a
            ));

            ImGui::SameLine();
            if (ImGui::Button("Choose Color")) {
                point_tool.show_color_picker_window = true;
            }


            // Tune ranges later if needed
            ImGui::SliderFloat("Size", &point_tool.size, 0.001f, 0.05f, "%.3f");

            if (ImGui::Button("Place Point")) {
                state.draw_mode = State::DrawMode::Point;
                point_tool.armed_for_placement = true;
                project_status = "Point tool armed. Click the sphere to place a point.";
                show_project_status = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                point_tool.armed_for_placement = false;
                project_status = "Point placement cancelled.";
                show_project_status = true;
            }

            ImGui::Separator();
            ImGui::Text("Current mode: %s",
                    (state.draw_mode == State::DrawMode::Point) ? "Point" :
                    (state.draw_mode == State::DrawMode::Polyline) ? "Polyline" :
                    (state.draw_mode == State::DrawMode::Polygon) ? "Polygon" : "None");

            if (point_tool.armed_for_placement) {
                ImGui::TextWrapped("Status: Armed. Click on the sphere to place the next point.");
            } else {
                ImGui::TextWrapped("Status: Not armed. Click 'Place Point' to arm the next sphere click.");
            }
        }
        ImGui::End();

        // If user closes panel via X, also disarm placement for safety
        if (!point_tool.show_panel) {
            point_tool.armed_for_placement = false;
        }
    }

    if (!point_tool.show_panel) {
        point_tool.armed_for_placement = false;
        point_tool.show_color_picker_window = false;
    }


    if (point_tool.show_color_picker_window) {
        ImGui::Begin("Point Color Picker", &point_tool.show_color_picker_window);
        ImGui::ColorPicker4("##PointColorPicker", glm::value_ptr(point_tool.color));
        ImGui::End();
    }


    // Primitives Panel
    {
        // Persistent selection across frames
        static uint32_t selected_primitive_id = 0;

        // Right-side fixed panel
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float menu_bar_height = ImGui::GetFrameHeight();

        const float panel_width = 650.0f;
        ImVec2 panel_pos = ImVec2(
                viewport->WorkPos.x + viewport->WorkSize.x - panel_width - 8.0f,
                viewport->WorkPos.y + menu_bar_height + 8.0f
        );
        ImVec2 panel_size = ImVec2(panel_width, viewport->WorkSize.y - menu_bar_height - 16.0f);

        ImGui::SetNextWindowPos(panel_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);

        ImGuiWindowFlags outlinerFlags =
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse;

        bool outliner_open = true;
        if (ImGui::Begin("Primitives", &outliner_open, outlinerFlags)) {

            ImGui::Text("Outliner");
            ImGui::Separator();

            // ---- Primitive list (layer order -> primitive order) ----
            // Use a child so the list scrolls independently
            ImGui::BeginChild("##PrimitiveList", ImVec2(0, 500), true);

            // If selection refers to a primitive that no longer exists, clear it
            if (selected_primitive_id != 0 && project.primitives.find(selected_primitive_id) == project.primitives.end()) {
                selected_primitive_id = 0;
            }

            for (const auto& layer : project.layers) {
                // Show layer header
                ImGui::PushID((int)layer.id);
                ImGui::Text("%s%s", layer.name.c_str(), layer.visible ? "" : " (hidden)");
                ImGui::Separator();

                // If layer hidden, still show its contents (Blender-style) but greyed out
                bool layer_disabled = !layer.visible;
                if (layer_disabled) ImGui::BeginDisabled(true);

                for (uint32_t pid : layer.primitiveIDs) {
                    auto it = project.primitives.find(pid);
                    if (it == project.primitives.end() || !it->second) continue;

                    Primitive* base = it->second.get();

                    const char* type_str = "Unknown";
                    switch (base->getType()) {
                        case PrimitiveType::Point:    type_str = "Point"; break;
                        case PrimitiveType::Polyline: type_str = "Polyline"; break;
                        case PrimitiveType::Polygon:  type_str = "Polygon"; break;
                        default: break;
                    }

                    // Row label like: "Point  12345"
                    char label[128];
                    std::snprintf(label, sizeof(label), "%s  %u", type_str, pid);

                    bool is_selected = (pid == selected_primitive_id);
                    if (ImGui::Selectable(label, is_selected)) {
                        selected_primitive_id = pid;
                    }
                }

                if (layer_disabled) ImGui::EndDisabled();
                ImGui::Spacing();
                ImGui::PopID();
            }

            ImGui::EndChild();

            ImGui::Separator();

            // ---- Details / edit panel for selected primitive ----
            if (selected_primitive_id == 0) {
                ImGui::TextWrapped("Select a primitive to view/edit it.");
            } else {
                auto it = project.primitives.find(selected_primitive_id);
                if (it != project.primitives.end() && it->second) {
                    Primitive* base = it->second.get();

                    ImGui::Text("Selected ID: %u", selected_primitive_id);
                    ImGui::Text("Type: %s",
                            (base->getType() == PrimitiveType::Point)    ? "Point" :
                            (base->getType() == PrimitiveType::Polyline) ? "Polyline" :
                            (base->getType() == PrimitiveType::Polygon)  ? "Polygon" : "Unknown");

                    ImGui::Spacing();

                    bool changed = false;

                    // Only implement editing for Points for now
                    if (base->getType() == PrimitiveType::Point) {
                        auto* pt = dynamic_cast<PointPrimitive*>(base);
                        if (pt) {
                            // Color edit (your codebase uses glm::vec4 for colors)
                            changed |= ImGui::ColorEdit4("Color", glm::value_ptr(pt->color));
                            // Size edit
                            changed |= ImGui::SliderFloat("Size", &pt->size, 0.001f, 0.05f, "%.3f");
                        }
                    } else {
                        ImGui::TextWrapped("Editing for this primitive type isn't implemented yet.");
                    }

                    ImGui::Spacing();
                    ImGui::Separator();

                    // Delete button
                    if (ImGui::Button("Delete Primitive")) {
                        bool ok = project.deletePrimitive(selected_primitive_id);
                        if (ok) {
                            selected_primitive_id = 0;
                            project.rebuildAttachedCubemapFromProject();
                            project_status = "Deleted primitive.";
                            show_project_status = true;
                        } else {
                            project_status = "Delete failed (primitive not found).";
                            show_project_status = true;
                        }
                    }

                    // If we edited something, rebuild renderer (simple + safe for now)
                    if (changed) {
                        project.rebuildAttachedCubemapFromProject();
                    }
                }
            }
        }
        ImGui::End();
    }



    ImGui::Render();


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.draw(camera);

    draw_data = ImGui::GetDrawData();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
