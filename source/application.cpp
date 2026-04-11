#include "application.h"
#include <chrono>
#include <cstdio>
#include <algorithm>
#include <thread>
#include <cmath>
#include <filesystem>
#include <random>
#include <array>
#include <cstring>
#include <nfd.h>

#include "common_glm_operations.h"
#include "shapes.h"

namespace {
    constexpr float kBaseWindowWidth = 1200.0f;
    constexpr float kBaseWindowHeight = 900.0f;
    constexpr float kBaseFontScale = 1.35f;
    constexpr float kBaseFontPixelSize = 18.0f;

    constexpr float kPanelMargin = 10.0f;
    constexpr float kLeftPanelWidth = 330.0f;
    constexpr float kRightPanelWidth = 360.0f;
    constexpr float kCollapsedOutlinerWidth = 48.0f;
    constexpr float kMinPointSize = 0.001f;
    constexpr float kMaxPointSize = 0.05f;

    ImGuiStyle g_base_style = {};
    bool g_base_style_captured = false;
    float g_current_ui_scale = -1.0f;

    enum class ResizeHandleCorner {
        BottomRight,
        BottomLeft
    };

    const char* drawModeLabel(int mode) {
        switch (mode) {
            case 1: return "Point";
            case 2: return "Polyline";
            case 3: return "Polygon";
            case 0:
            default: return "None";
        }
    }

    std::string ensurePngExtension(const std::string& path) {
        std::filesystem::path p(path);
        if (p.has_extension() && p.extension() == ".png") {
            return path;
        }
        return path + ".png";
    }

    float computeUiScale(GLFWwindow* window) {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);

        if (width <= 0 || height <= 0) {
            return 1.0f;
        }

        const float width_scale = static_cast<float>(width) / kBaseWindowWidth;
        const float height_scale = static_cast<float>(height) / kBaseWindowHeight;
        const float scale = std::min(width_scale, height_scale);

        return std::clamp(scale, 1.0f, 2.25f);
    }

    void applyResponsiveUiScale(GLFWwindow* window) {
        if (!g_base_style_captured) {
            return;
        }

        const float scale = computeUiScale(window);
        if (std::fabs(scale - g_current_ui_scale) < 0.01f) {
            return;
        }

        ImGuiStyle& style = ImGui::GetStyle();
        style = g_base_style;
        style.ScaleAllSizes(scale);

        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = kBaseFontScale * scale;

        g_current_ui_scale = scale;
    }

    ImFont* tryLoadUIFont(ImGuiIO& io, float pixel_size) {
        const std::array<const char*, 7> font_candidates = {
                "source/fonts/Inter-Medium.otf",
                "source/fonts/Inter-Regular.ttf",
                "source/fonts/Roboto-Regular.ttf",
                "fonts/Inter-Medium.ttf",
                "fonts/Inter-Regular.ttf",
                "C:/Windows/Fonts/segoeui.ttf",
                "C:/Windows/Fonts/arial.ttf"
        };

        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 2;
        config.PixelSnapH = false;

        for (const char* path : font_candidates) {
            if (std::filesystem::exists(path)) {
                if (ImFont* font = io.Fonts->AddFontFromFileTTF(path, pixel_size, &config)) {
                    return font;
                }
            }
        }

        return nullptr;
    }

    bool drawNormalizedSizeSlider(const char* id, float& size) {
        float percent = ((size - kMinPointSize) / (kMaxPointSize - kMinPointSize)) * 100.0f;
        percent = std::clamp(percent, 0.0f, 100.0f);

        ImGui::SetNextItemWidth(-1.0f);
        bool changed = ImGui::SliderFloat(id, &percent, 0.0f, 100.0f, "%.0f%%");
        if (changed) {
            size = kMinPointSize + (percent / 100.0f) * (kMaxPointSize - kMinPointSize);
        }
        return changed;
    }

    void drawSliderExtentsText(const char* left_label, const char* right_label) {
        ImGui::TextDisabled("%s", left_label);
        float right_width = ImGui::CalcTextSize(right_label).x;
        float cursor_x = ImGui::GetCursorPosX();
        float target_x = std::max(cursor_x, ImGui::GetWindowContentRegionMax().x - right_width);
        ImGui::SameLine(target_x);
        ImGui::TextDisabled("%s", right_label);
    }

    void copyStringToBuffer(const std::string& src, char* dst, size_t dst_size) {
        if (dst_size == 0) return;
        std::strncpy(dst, src.c_str(), dst_size - 1);
        dst[dst_size - 1] = '\0';
    }

    std::string primitiveFallbackLabel(const Primitive& primitive) {
        const char* type_str = "Primitive";
        switch (primitive.getType()) {
            case PrimitiveType::Point: type_str = "Point"; break;
            case PrimitiveType::Polyline: type_str = "Polyline"; break;
            case PrimitiveType::Polygon: type_str = "Polygon"; break;
            default: break;
        }
        return std::string(type_str) + " " + std::to_string(primitive.getID());
    }

    std::string primitiveDisplayName(const Primitive& primitive) {
        if (!primitive.getName().empty()) {
            return primitive.getName();
        }
        return primitiveFallbackLabel(primitive);
    }

    bool drawNormalizedWidthSlider(const char* id, float& width) {
        return drawNormalizedSizeSlider(id, width);
    }

    void drawPinnedResizeHandle(
            const char* id,
            ImVec2& panel_size,
            ResizeHandleCorner corner,
            float ui_scale,
            float min_width,
            float min_height,
            float max_width,
            float max_height) {

        const float grip_size = 18.0f * ui_scale;

        const ImVec2 window_pos = ImGui::GetWindowPos();
        const ImVec2 window_size = ImGui::GetWindowSize();

        ImVec2 handle_min;
        ImVec2 handle_max;

        if (corner == ResizeHandleCorner::BottomRight) {
            handle_min = ImVec2(window_pos.x + window_size.x - grip_size, window_pos.y + window_size.y - grip_size);
            handle_max = ImVec2(window_pos.x + window_size.x,             window_pos.y + window_size.y);
        } else {
            handle_min = ImVec2(window_pos.x,                             window_pos.y + window_size.y - grip_size);
            handle_max = ImVec2(window_pos.x + grip_size,                 window_pos.y + window_size.y);
        }

        ImGui::SetCursorScreenPos(handle_min);
        ImGui::InvisibleButton(id, ImVec2(grip_size, grip_size));

        const bool hovered = ImGui::IsItemHovered();
        const bool active = ImGui::IsItemActive();

        if (active) {
            const ImVec2 delta = ImGui::GetIO().MouseDelta;

            if (corner == ResizeHandleCorner::BottomRight) {
                panel_size.x += delta.x;
            } else {
                panel_size.x -= delta.x;
            }

            panel_size.y += delta.y;

            panel_size.x = std::clamp(panel_size.x, min_width, max_width);
            panel_size.y = std::clamp(panel_size.y, min_height, max_height);
        }

        ImU32 grip_col = ImGui::GetColorU32(
                active ? ImGuiCol_ResizeGripActive :
                hovered ? ImGuiCol_ResizeGripHovered :
                ImGuiCol_ResizeGrip);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        if (corner == ResizeHandleCorner::BottomRight) {
            draw_list->AddTriangleFilled(
                    handle_max,
                    ImVec2(handle_min.x, handle_max.y),
                    ImVec2(handle_max.x, handle_min.y),
                    grip_col
            );
        } else {
            draw_list->AddTriangleFilled(
                    ImVec2(handle_min.x, handle_max.y),
                    handle_max,
                    ImVec2(handle_min.x, handle_min.y),
                    grip_col
            );
        }
    }
}


// ################################################## //


// Static variable initialization
const int Application::w_width = 2400;
const int Application::w_height = 1800;
GLFWwindow* Application::window = nullptr;
bool Application::initialized = false;
bool Application::gui_change = true;
bool Application::nfd_initialized = false;
bool Application::mouse_moved = false;
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigWindowsResizeFromEdges = false;

    ImFont* loaded_font = tryLoadUIFont(io, kBaseFontPixelSize);
    if (loaded_font) {
        io.FontDefault = loaded_font;
    }

    io.FontGlobalScale = kBaseFontScale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.CellPadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 13.0f;
    style.GrabMinSize = 11.0f;

    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 7.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 7.0f;
    style.TabRounding = 8.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.94f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.60f, 0.67f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.10f, 0.13f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.07f, 0.10f, 0.75f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.12f, 0.15f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.16f, 0.21f, 0.27f, 0.95f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.22f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.28f, 0.37f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.07f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.07f, 0.10f, 0.65f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.33f, 0.44f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.40f, 0.53f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.47f, 0.62f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.39f, 0.67f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.67f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.58f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.24f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.33f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.27f, 0.41f, 0.58f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.26f, 0.40f, 0.95f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.34f, 0.50f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.41f, 0.58f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.39f, 0.67f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.39f, 0.67f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.58f, 0.80f, 1.00f, 0.95f);

    g_base_style = style;
    g_base_style_captured = true;
    applyResponsiveUiScale(window);
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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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
    glPointSize(10.0f);
    glLineWidth(4.0f);
}

void Application::setup_key_bindings() {
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

    press_key_actions.emplace(std::pair(GLFW_KEY_Z, 0), CAMERA_SET_ZOOM_IN);
    press_key_actions.emplace(std::pair(GLFW_KEY_X, 0), CAMERA_SET_ZOOM_OUT);

    release_key_actions.emplace(std::pair(GLFW_KEY_Z, 0), CAMERA_UNSET_ZOOM_IN);
    release_key_actions.emplace(std::pair(GLFW_KEY_X, 0), CAMERA_UNSET_ZOOM_OUT);

    release_key_actions.emplace(std::pair(GLFW_KEY_COMMA, 0), CAMERA_DECREASE_ROTATE_SPEED);
    release_key_actions.emplace(std::pair(GLFW_KEY_PERIOD, 0), CAMERA_INCREASE_ROTATE_SPEED);
    release_key_actions.emplace(std::pair(GLFW_KEY_R, 0), CAMERA_RESET);

    release_key_actions.emplace(std::pair(GLFW_KEY_H, 0), DRAW_MODE_ROTATE);
    press_key_actions.emplace(std::pair(GLFW_KEY_BACKSPACE, 0), DRAW_UNDO_LAST_VERTEX);
    press_key_actions.emplace(std::pair(GLFW_KEY_ENTER, 0), DRAW_FINISH_SHAPE);
    press_key_actions.emplace(std::pair(GLFW_KEY_KP_ENTER, 0), DRAW_FINISH_SHAPE);
}


// ################################################## //


// APPLICATION CORE
Application::Application() :
        camera({2.0f, 0.0f, 0.0f},
                {-2.0f, -0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f)) {
    if (!initialized) return;
    renderer.init();
    camera.update_window(window);
    project.attachCubemap(&renderer);
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
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantTextInput) {
        return;
    }

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
    if (ImGui::GetIO().WantCaptureMouse) {
        gui_change = true;
        return;
    }

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        event_queue.emplace_back(CLICK_SPHERE);
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
        event_queue.emplace_back(UNCLICK_SPHERE);
    }
}

void Application::cursorPositionCallback(GLFWwindow *win, double xpos, double ypos) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    if (!mouse_moved) {
        event_queue.emplace_back(MOVE_MOUSE);
        mouse_moved = true;
    }
}

void Application::windowSizeCallback(GLFWwindow *win, int width, int height) {
    event_queue.emplace_back(WINDOW_RESIZE);
}


// ################################################## //


// APPLICATION MAINLOOP
void Application::mainloop() {
    constexpr unsigned int FPS = 60;
    const double TARGET_FRAME_TIME = 1.0 / (double)FPS;

    running = true;
    while (running) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        deltaTime = glfwGetTime() - previousFrameTime;
        previousFrameTime = glfwGetTime();

        glfwPollEvents();
        handle_events();
        update_window();
        render_frame();

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
        case NONE:
            break;

        case CLOSE:
            running = false;
            break;

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

        case CAMERA_RESET:
            state.camera_rotate_speed = 1.0f;
            state.camera_radius = 2.0;
            state.camera_zoom_speed = 1.0f;
            camera.set_position(glm::vec3(2.0f, 0.0f, 0.0f));
            camera.set_up(glm::vec3(0.0f, 1.0f, 0.0f));
            break;

        case DRAW_UNDO_LAST_VERTEX:
            switch (state.draw_mode) {
                case State::DrawMode::Polyline:
                    undoActivePolylineVertex();
                    break;

                case State::DrawMode::Polygon:
                    undoActivePolygonVertex();
                    break;

                case State::DrawMode::Point:
                case State::DrawMode::None:
                default:
                    break;
            }
            break;

        case DRAW_FINISH_SHAPE:
            switch (state.draw_mode) {
                case State::DrawMode::Polyline:
                    finishActivePolyline();
                    break;

                case State::DrawMode::Polygon:
                    finishActivePolygon();
                    break;

                case State::DrawMode::Point:
                case State::DrawMode::None:
                default:
                    break;
            }
            break;


        case CLICK_SPHERE:
        {
            glfwGetCursorPos(window, &x_pos, &y_pos);

            int current_width = 0;
            int current_height = 0;
            glfwGetWindowSize(window, &current_width, &current_height);

            click_coords = sphere_click_lat_lon(
                    camera.get_position(),
                    camera.get_up(),
                    1.0f,
                    glm::vec2((float)x_pos, (float)y_pos),
                    (float)current_width, (float)current_height,
                    glm::radians(60.0)
            );

            if (std::isnan(click_coords.first)) {
                // std::cout << "latitude: NaN\nlongitude: NaN" << std::endl;
                break;
            }

            // std::cout << "latitude: " << glm::degrees(click_coords.first)
            //          << "\nlongitude: " << glm::degrees(click_coords.second) << std::endl;

            if (!point_tool.armed_for_placement && !polyline_tool.armed_for_placement) {
                glm::vec3 pos = lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);
                if (project.selectPrimitiveAt(pos)) {
                    state.mouse_click_position = pos;
                    state.last_valid_mouse_position = pos;
                    state.track_mouse_drag = true;
                }
            }

            switch (state.draw_mode) {
                case State::DrawMode::Point:
                {
                    if (!point_tool.armed_for_placement) {
                        break;
                    }

                    glm::vec3 pos = lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);

                    uint32_t newId = project.nextPrimitiveID();
                    auto point = std::make_unique<PointPrimitive>(newId);
                    point->p = pos;
                    point->color = point_tool.color;
                    point->size = point_tool.size;
                    point->setName(project.makeDefaultPrimitiveName(PrimitiveType::Point));

                    project.addPrimitiveToDefaultLayer(std::move(point));

                    outliner.selected_primitive_id = newId;
                    outliner.name_buffer_primitive_id = 0;
                    point_tool.armed_for_placement = false;
                    break;
                }

                case State::DrawMode::Polyline:
                {
                    if (!polyline_tool.armed_for_placement) {
                        break;
                    }

                    glm::vec3 pos = lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);
                    polyline_tool.verts.push_back(pos);
                    refreshPolylinePreview();
                    break;
                }

                case State::DrawMode::Polygon:
                {
                    if (!polygon_tool.armed_for_placement) {
                        break;
                    }

                    glm::vec3 pos = lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);
                    polygon_tool.verts.push_back(pos);
                    refreshPolygonPreview();
                    break;
                }

                case State::DrawMode::None:
                    break;

                default:
                    break;
            }
            break;
        }

        case UNCLICK_SPHERE:
        {
            if (!state.track_mouse_drag) break;

            glfwGetCursorPos(window, &x_pos, &y_pos);

            int current_width = 0;
            int current_height = 0;
            glfwGetWindowSize(window, &current_width, &current_height);

            click_coords = sphere_click_lat_lon(
                    camera.get_position(),
                    camera.get_up(),
                    1.0f,
                    glm::vec2((float)x_pos, (float)y_pos),
                    (float)current_width, (float)current_height,
                    glm::radians(60.0)
            );

            glm::vec3 final_position = std::isnan(click_coords.first) ? state.last_valid_mouse_position : lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);
            if (glm::dot(final_position, state.mouse_click_position) <= 0.99995) { // Only rotate if mouse has moved significantly
                project.rotateSelectedPrimitive(state.mouse_click_position, final_position);
            }

            state.track_mouse_drag = false;
            state.mouse_click_position = final_position;
            state.last_valid_mouse_position = final_position;

            break;
        }

        case MOVE_MOUSE:
        {
            if (!state.track_mouse_drag) break;

            glfwGetCursorPos(window, &x_pos, &y_pos);

            int current_width = 0;
            int current_height = 0;
            glfwGetWindowSize(window, &current_width, &current_height);

            click_coords = sphere_click_lat_lon(
                    camera.get_position(),
                    camera.get_up(),
                    1.0f,
                    glm::vec2((float)x_pos, (float)y_pos),
                    (float)current_width, (float)current_height,
                    glm::radians(60.0)
            );

            glm::vec3 new_position = std::isnan(click_coords.first) ? state.last_valid_mouse_position : lat_lon_to_xyz(click_coords.first, click_coords.second, 1.0f);
            state.last_valid_mouse_position = new_position;
            break;
        }

        case DRAW_MODE_ROTATE:
            // renderer.export_cubemap("textures/test_out.png"); Manually test cubemap export w/o ui
            // renderer.import_base_cubemap("textures/test_in.jpg");
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
        state.camera_radius = exp2(log2(state.camera_radius - 1.0) + state.camera_zoom_speed * deltaTime) + 1.0;
        glm::vec3 pos_norm = glm::normalize(camera.get_position());
        camera.set_position((float)state.camera_radius * pos_norm);
    }
    else if ((state.camera_motion & State::CameraRotation::ZoomOut)
             && !(state.camera_motion & State::CameraRotation::ZoomIn)) {
        state.camera_radius = exp2(log2(state.camera_radius - 1.0) - state.camera_zoom_speed * deltaTime) + 1.0;
        glm::vec3 pos_norm = glm::normalize(camera.get_position());
        camera.set_position((float)state.camera_radius * pos_norm);
    }
}

void Application::refreshPolylinePreview() {
    // project.rebuildAttachedCubemapFromProject();
    if (polyline_tool.verts.size() < 2 && polyline_tool.temp_line_render_id != UINT32_MAX) {
        renderer.remove_line(polyline_tool.temp_line_render_id);
        polyline_tool.temp_line_render_id = UINT32_MAX;
    }

    if (polyline_tool.verts.empty()) {
        return;
    }

    // Preview clicked vertices as temporary points
    for (size_t i = 0; i < polyline_tool.verts.size(); ++i) {
        PointPrimitive preview_point(4000000000u - static_cast<uint32_t>(i));
        preview_point.p = polyline_tool.verts[i];
        preview_point.color = polyline_tool.color;
        preview_point.size = std::clamp(polyline_tool.width * 0.4f, 0.008f, 0.03f);
        auto temp_vertex_id = renderer.add_new_point(preview_point);
        polyline_tool.temp_vertices_render_ids.emplace_back(temp_vertex_id);
    }

    // Preview the in-progress line once we have at least 2 vertices
    if (polyline_tool.verts.size() >= 2) {
        PolylinePrimitive preview_line(3900000000u);
        preview_line.color = polyline_tool.color;
        preview_line.width = polyline_tool.width;
        preview_line.verts = polyline_tool.verts;
        if (polyline_tool.verts.size() > 2) {
            renderer.remove_line(polyline_tool.temp_line_render_id);
        }
        polyline_tool.temp_line_render_id = renderer.add_new_line(preview_line);
    }
}

void Application::refreshPolygonPreview() {
    project.rebuildAttachedCubemapFromProject();

    if (polygon_tool.verts.empty()) {
        return;
    }

    // Preview clicked vertices as temporary points
    for (size_t i = 0; i < polygon_tool.verts.size(); ++i) {
        PointPrimitive preview_point(3800000000u - static_cast<uint32_t>(i));
        preview_point.p = polygon_tool.verts[i];
        preview_point.color = polygon_tool.color;
        preview_point.size = 0.01f;
        renderer.add_new_point(preview_point);
    }

    // Preview the polygon boundary using an ordinary polyline for now.
    // Do NOT call renderer.add_new_polygon(...) yet because backend polygon
    // rendering is not implemented in cubemap.cpp.
    if (polygon_tool.verts.size() >= 2) {
        PolylinePrimitive preview_outline(3700000000u);
        preview_outline.color = polygon_tool.color;
        preview_outline.width = 0.007f;
        preview_outline.verts = polygon_tool.verts;

        // Close the loop once we have enough vertices to look like a polygon.
        if (polygon_tool.verts.size() >= 3) {
            preview_outline.verts.push_back(polygon_tool.verts.front());
        }

        renderer.add_new_line(preview_outline);
    }
}

void Application::undoActivePolylineVertex() {
    if (!polyline_tool.armed_for_placement || polyline_tool.verts.empty()) {
        return;
    }

    polyline_tool.verts.pop_back();

    if (polyline_tool.verts.empty()) {
        project.rebuildAttachedCubemapFromProject();
    } else {
        refreshPolylinePreview();
    }
}

void Application::undoActivePolygonVertex() {
    if (!polygon_tool.armed_for_placement || polygon_tool.verts.empty()) {
        return;
    }

    polygon_tool.verts.pop_back();

    if (polygon_tool.verts.empty()) {
        project.rebuildAttachedCubemapFromProject();
    } else {
        refreshPolygonPreview();
    }
}

void Application::finishActivePolyline() {
    if (!polyline_tool.armed_for_placement || polyline_tool.verts.size() < 2) {
        return;
    }

    uint32_t newId = project.nextPrimitiveID();
    auto line = std::make_unique<PolylinePrimitive>(newId);
    line->color = polyline_tool.color;
    line->width = polyline_tool.width;
    line->verts = polyline_tool.verts;
    line->setName(project.makeDefaultPrimitiveName(PrimitiveType::Polyline));

    project.addPrimitiveToDefaultLayer(std::move(line));
    project.rebuildAttachedCubemapFromProject();

    outliner.selected_primitive_id = newId;
    outliner.name_buffer_primitive_id = 0;

    polyline_tool.armed_for_placement = false;
    polyline_tool.verts.clear();
}

void Application::finishActivePolygon() {
    if (!polygon_tool.armed_for_placement || polygon_tool.verts.size() < 3) {
        return;
    }

    uint32_t newId = project.nextPrimitiveID();
    auto polygon = std::make_unique<PolygonPrimitive>(newId);
    polygon->color = polygon_tool.color;
    polygon->verts = polygon_tool.verts;
    polygon->setName(project.makeDefaultPrimitiveName(PrimitiveType::Polygon));

    project.addPrimitiveToDefaultLayer(std::move(polygon));
    project.rebuildAttachedCubemapFromProject();

    outliner.selected_primitive_id = newId;
    outliner.name_buffer_primitive_id = 0;

    polygon_tool.armed_for_placement = false;
    polygon_tool.verts.clear();
}

void Application::cancelActivePolyline() {
    polyline_tool.armed_for_placement = false;
    polyline_tool.verts.clear();
    project.rebuildAttachedCubemapFromProject();
}

void Application::cancelActivePolygon() {
    polygon_tool.armed_for_placement = false;
    polygon_tool.verts.clear();
    project.rebuildAttachedCubemapFromProject();
}

void Application::render_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    applyResponsiveUiScale(window);
    ImGui::NewFrame();

    const float ui_scale = computeUiScale(window);
    const float panel_margin = kPanelMargin * ui_scale;
    const float left_panel_width = kLeftPanelWidth * ui_scale;
    const float right_panel_width = kRightPanelWidth * ui_scale;
    const float collapsed_outliner_width = kCollapsedOutlinerWidth * ui_scale;

    MainMenuResult menu = app_gui.DrawMainMenuBar(project_filepath);
    if (!menu.errorMessage.empty()) {
        project_status = menu.errorMessage;
        show_project_status = true;
    }

    switch (menu.drawAction) {
        case DrawMenuAction::SelectPoint:
            state.draw_mode = State::DrawMode::Point;
            point_tool.show_panel = true;
            point_tool.armed_for_placement = false;

            polyline_tool.show_panel = false;
            polyline_tool.armed_for_placement = false;
            polyline_tool.verts.clear();
            polyline_tool.reset(renderer);

            polygon_tool.show_panel = false;
            polygon_tool.armed_for_placement = false;
            polygon_tool.verts.clear();

            project.rebuildAttachedCubemapFromProject();
            break;

        case DrawMenuAction::SelectPolyline:
            state.draw_mode = State::DrawMode::Polyline;
            point_tool.show_panel = false;
            point_tool.armed_for_placement = false;

            polyline_tool.show_panel = true;
            polyline_tool.armed_for_placement = false;
            polyline_tool.verts.clear();
            polyline_tool.reset(renderer);

            polygon_tool.show_panel = false;
            polygon_tool.armed_for_placement = false;
            polygon_tool.verts.clear();

            project.rebuildAttachedCubemapFromProject();
            break;

        case DrawMenuAction::SelectPolygon:
            state.draw_mode = State::DrawMode::Polygon;
            point_tool.show_panel = false;
            point_tool.armed_for_placement = false;

            polyline_tool.show_panel = false;
            polyline_tool.armed_for_placement = false;
            polyline_tool.verts.clear();
            polyline_tool.reset(renderer);

            polygon_tool.show_panel = true;
            polygon_tool.armed_for_placement = false;
            polygon_tool.verts.clear();

            project.rebuildAttachedCubemapFromProject();
            break;

        case DrawMenuAction::None:
            break;
    }

    switch (menu.cubemapAction) {
        case CubemapMenuAction::ImportCubemap:
        {
            if (!nfd_initialized) {
                project_status = "Cubemap import failed:\nNative file dialog is not initialized.";
                show_project_status = true;
                break;
            }

            const nfdfilteritem_t filters[3] = {
                    { "PNG Image", "png" },
                    { "JPEG Image", "jpg" },
                    { "JPEG Image", "jpeg" }
            };

            nfdchar_t* outPath = nullptr;
            nfdresult_t r = NFD_OpenDialog(&outPath, filters, 3, nullptr);

            if (r == NFD_OKAY && outPath) {
                std::string cubemapPath = outPath;
                NFD_FreePath(outPath);

                if (!std::filesystem::exists(cubemapPath)) {
                    project_status = "Cubemap import failed:\nFile does not exist:\n" + cubemapPath;
                    show_project_status = true;
                    break;
                }

                renderer.import_base_cubemap(cubemapPath);
                project_status = "Imported cubemap from:\n" + cubemapPath;
                show_project_status = true;
            }
            else if (r == NFD_ERROR) {
                project_status = std::string("Cubemap import dialog error:\n") +
                                 (NFD_GetError() ? NFD_GetError() : "Unknown error");
                show_project_status = true;
            }
            break;
        }

        case CubemapMenuAction::ExportCubemap:
        {
            if (!nfd_initialized) {
                project_status = "Cubemap export failed:\nNative file dialog is not initialized.";
                show_project_status = true;
                break;
            }

            const nfdfilteritem_t filters[1] = {
                    { "PNG Image", "png" }
            };

            nfdchar_t* outPath = nullptr;
            nfdresult_t r = NFD_SaveDialog(&outPath, filters, 1, nullptr, "cubemap.png");

            if (r == NFD_OKAY && outPath) {
                std::string exportPath = ensurePngExtension(outPath);
                NFD_FreePath(outPath);

                std::filesystem::path p(exportPath);
                if (p.has_parent_path()) {
                    std::filesystem::create_directories(p.parent_path());
                }

                renderer.export_cubemap(exportPath);
                project_status = "Exported cubemap to:\n" + exportPath;
                show_project_status = true;
            }
            else if (r == NFD_ERROR) {
                project_status = std::string("Cubemap export dialog error:\n") +
                                 (NFD_GetError() ? NFD_GetError() : "Unknown error");
                show_project_status = true;
            }
            break;
        }

        case CubemapMenuAction::None:
            break;
    }

    ProjectMenuAction action = menu.projectAction;
    try {
        if (action == ProjectMenuAction::NewProject) {
            if (project_filepath.empty()) {
                throw std::runtime_error("New project cancelled (no file selected).");
            }

            project = Project();
            project.attachCubemap(&renderer);
            project.rebuildAttachedCubemapFromProject();

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

    app_gui.DrawStatusWindow(&show_project_status, project_status);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float menu_bar_height = ImGui::GetFrameHeight();
    const float panel_y = viewport->WorkPos.y + menu_bar_height + panel_margin;
    const float max_panel_height = viewport->WorkSize.y - menu_bar_height - (2.0f * panel_margin);

    if (point_tool.panel_size.x <= 0.0f || point_tool.panel_size.y <= 0.0f) {
        point_tool.panel_size = ImVec2(left_panel_width, 500.0f * ui_scale);
    }
    if (polyline_tool.panel_size.x <= 0.0f || polyline_tool.panel_size.y <= 0.0f) {
        polyline_tool.panel_size = ImVec2(left_panel_width, 560.0f * ui_scale);
    }
    if (polygon_tool.panel_size.x <= 0.0f || polygon_tool.panel_size.y <= 0.0f) {
        polygon_tool.panel_size = ImVec2(left_panel_width, 540.0f * ui_scale);
    }

    if (outliner.panel_size.x <= 0.0f || outliner.panel_size.y <= 0.0f) {
        outliner.panel_size = ImVec2(right_panel_width, max_panel_height);
    }

    point_tool.panel_size.x = std::clamp(point_tool.panel_size.x, 280.0f * ui_scale, viewport->WorkSize.x * 0.45f);
    point_tool.panel_size.y = std::clamp(point_tool.panel_size.y, 330.0f * ui_scale, max_panel_height);

    polyline_tool.panel_size.x = std::clamp(polyline_tool.panel_size.x, 280.0f * ui_scale, viewport->WorkSize.x * 0.45f);
    polyline_tool.panel_size.y = std::clamp(polyline_tool.panel_size.y, 380.0f * ui_scale, max_panel_height);

    polygon_tool.panel_size.x = std::clamp(polygon_tool.panel_size.x, 280.0f * ui_scale, viewport->WorkSize.x * 0.45f);
    polygon_tool.panel_size.y = std::clamp(polygon_tool.panel_size.y, 360.0f * ui_scale, max_panel_height);

    outliner.panel_size.x = std::clamp(outliner.panel_size.x, 300.0f * ui_scale, viewport->WorkSize.x * 0.50f);
    outliner.panel_size.y = std::clamp(outliner.panel_size.y, 500.0f * ui_scale, max_panel_height);

    // Point tool panel
    if (point_tool.show_panel) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + panel_margin, panel_y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(point_tool.panel_size, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.96f);

        ImGuiWindowFlags pointToolFlags =
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Point Tool", &point_tool.show_panel, pointToolFlags)) {
            point_tool.panel_size = ImGui::GetWindowSize();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextDisabled("%s", "Create single points directly on the globe.");
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            ImGui::TextUnformatted("Color");
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::ColorEdit4(
                    "##PointToolColor",
                    glm::value_ptr(point_tool.color),
                    ImGuiColorEditFlags_NoInputs |
                    ImGuiColorEditFlags_AlphaBar |
                    ImGuiColorEditFlags_AlphaPreviewHalf
            );

            ImGui::Spacing();
            ImGui::TextUnformatted("Point Size");
            drawNormalizedSizeSlider("##PointToolSize", point_tool.size);
            drawSliderExtentsText("Smaller", "Larger");

            ImGui::Spacing();
            float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
            if (ImGui::Button(point_tool.armed_for_placement ? "Armed" : "Place Point", ImVec2(button_width, 0.0f))) {
                state.draw_mode = State::DrawMode::Point;
                point_tool.armed_for_placement = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(button_width, 0.0f))) {
                point_tool.armed_for_placement = false;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Mode: %s", drawModeLabel(static_cast<int>(state.draw_mode)));
            ImGui::TextWrapped(
                    "%s",
                    point_tool.armed_for_placement
                    ? "Status: armed. Your next click on the sphere will place a point."
                    : "Status: idle. Click Place Point, then click the sphere to add a point."
            );

            drawPinnedResizeHandle(
                    "##PointToolResizeHandle",
                    point_tool.panel_size,
                    ResizeHandleCorner::BottomRight,
                    ui_scale,
                    280.0f * ui_scale,
                    330.0f * ui_scale,
                    viewport->WorkSize.x * 0.45f,
                    max_panel_height
            );
        }
        ImGui::End();

        if (!point_tool.show_panel) {
            point_tool.armed_for_placement = false;
        }
    }

    if (!point_tool.show_panel) {
        point_tool.armed_for_placement = false;
    }

    //Polyline Tool Panel
    if (polyline_tool.show_panel) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + panel_margin, panel_y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(polyline_tool.panel_size, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.96f);

        ImGuiWindowFlags polylineToolFlags =
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Polyline Tool", &polyline_tool.show_panel, polylineToolFlags)) {
            polyline_tool.panel_size = ImGui::GetWindowSize();
            bool preview_changed = false;

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextDisabled("%s", "Create a line by placing vertices on the globe, then finish the polyline.");
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            ImGui::TextUnformatted("Color");
            ImGui::SetNextItemWidth(-1.0f);
            preview_changed |= ImGui::ColorEdit4(
                    "##PolylineToolColor",
                    glm::value_ptr(polyline_tool.color),
                    ImGuiColorEditFlags_NoInputs |
                    ImGuiColorEditFlags_AlphaBar |
                    ImGuiColorEditFlags_AlphaPreviewHalf
            );

            ImGui::Spacing();
            ImGui::TextUnformatted("Line Width");
            preview_changed |= drawNormalizedWidthSlider("##PolylineToolWidth", polyline_tool.width);
            drawSliderExtentsText("Thinner", "Thicker");

            ImGui::Spacing();
            ImGui::Text("Vertices: %d", static_cast<int>(polyline_tool.verts.size()));

            ImGui::Spacing();
            float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

            if (ImGui::Button(polyline_tool.armed_for_placement ? "Adding Vertices" : "Start Polyline", ImVec2(button_width, 0.0f))) {
                state.draw_mode = State::DrawMode::Polyline;
                polyline_tool.armed_for_placement = true;
                if (!polyline_tool.verts.empty()) {
                    polyline_tool.verts.clear();
                }
            }

            ImGui::SameLine();
            bool can_finish = polyline_tool.verts.size() >= 2;
            if (!can_finish) ImGui::BeginDisabled();
            if (ImGui::Button("Finish", ImVec2(button_width, 0.0f))) {
                finishActivePolyline();
            }
            if (!can_finish) ImGui::EndDisabled();

                // project.rebuildAttachedCubemapFromProject();
            ImGui::Spacing();

            float lower_button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

            bool can_undo_polyline = !polyline_tool.verts.empty();
            if (!can_undo_polyline) ImGui::BeginDisabled();
            if (ImGui::Button("Undo Last", ImVec2(lower_button_width, 0.0f))) {
                undoActivePolylineVertex();
            }
            if (!can_undo_polyline) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(lower_button_width, 0.0f))) {
                cancelActivePolyline();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Mode: %s", drawModeLabel(static_cast<int>(state.draw_mode)));
            ImGui::TextWrapped(
                    "%s",
                    polyline_tool.armed_for_placement
                    ? "Status: active. Click the sphere to add vertices, then press Finish."
                    : "Status: idle. Press Start Polyline to begin placing vertices."
            );

            if (preview_changed && polyline_tool.armed_for_placement && !polyline_tool.verts.empty()) {
                refreshPolylinePreview();
            }

            drawPinnedResizeHandle(
                    "##PolylineToolResizeHandle",
                    polyline_tool.panel_size,
                    ResizeHandleCorner::BottomRight,
                    ui_scale,
                    280.0f * ui_scale,
                    380.0f * ui_scale,
                    viewport->WorkSize.x * 0.45f,
                    max_panel_height
            );
        }
        ImGui::End();

        if (!polyline_tool.show_panel) {
            polyline_tool.armed_for_placement = false;
            polyline_tool.verts.clear();
            polyline_tool.reset(renderer);
            project.rebuildAttachedCubemapFromProject();
        }
    }

    if (!polyline_tool.show_panel) {
        polyline_tool.armed_for_placement = false;
    }

    // Polygon Tool Panel
    if (polygon_tool.show_panel) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + panel_margin, panel_y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(polygon_tool.panel_size, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.96f);

        ImGuiWindowFlags polygonToolFlags =
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Polygon Tool", &polygon_tool.show_panel, polygonToolFlags)) {
            polygon_tool.panel_size = ImGui::GetWindowSize();
            bool preview_changed = false;

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextDisabled("%s", "Create a polygon by placing vertices on the globe, then finish the shape.");
            ImGui::PopTextWrapPos();
            ImGui::Spacing();

            ImGui::TextUnformatted("Color");
            ImGui::SetNextItemWidth(-1.0f);
            preview_changed |= ImGui::ColorEdit4(
                    "##PolygonToolColor",
                    glm::value_ptr(polygon_tool.color),
                    ImGuiColorEditFlags_NoInputs |
                    ImGuiColorEditFlags_AlphaBar |
                    ImGuiColorEditFlags_AlphaPreviewHalf
            );

            ImGui::Spacing();
            ImGui::Text("Vertices: %d", static_cast<int>(polygon_tool.verts.size()));

            ImGui::Spacing();
            float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

            if (ImGui::Button(polygon_tool.armed_for_placement ? "Adding Vertices" : "Start Polygon", ImVec2(button_width, 0.0f))) {
                state.draw_mode = State::DrawMode::Polygon;
                polygon_tool.armed_for_placement = true;
            }

            ImGui::SameLine();
            bool can_finish = polygon_tool.verts.size() >= 3;
            if (!can_finish) ImGui::BeginDisabled();
            if (ImGui::Button("Finish", ImVec2(button_width, 0.0f))) {
                finishActivePolygon();
            }
            if (!can_finish) ImGui::EndDisabled();

            ImGui::Spacing();

            float lower_button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

            bool can_undo_polygon = !polygon_tool.verts.empty();
            if (!can_undo_polygon) ImGui::BeginDisabled();
            if (ImGui::Button("Undo Last", ImVec2(lower_button_width, 0.0f))) {
                undoActivePolygonVertex();
            }
            if (!can_undo_polygon) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(lower_button_width, 0.0f))) {
                cancelActivePolygon();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Mode: %s", drawModeLabel(static_cast<int>(state.draw_mode)));
            ImGui::TextWrapped(
                    "%s",
                    polygon_tool.armed_for_placement
                    ? "Status: active. Click the sphere to add vertices, then press Finish."
                    : "Status: idle. Press Start Polygon to begin placing vertices."
            );
            ImGui::TextDisabled("%s", "Preview currently shows the polygon boundary only until backend polygon rendering is finished.");

            if (preview_changed && polygon_tool.armed_for_placement && !polygon_tool.verts.empty()) {
                refreshPolygonPreview();
            }

            drawPinnedResizeHandle(
                    "##PolygonToolResizeHandle",
                    polygon_tool.panel_size,
                    ResizeHandleCorner::BottomRight,
                    ui_scale,
                    280.0f * ui_scale,
                    360.0f * ui_scale,
                    viewport->WorkSize.x * 0.45f,
                    max_panel_height
            );
        }
        ImGui::End();

        if (!polygon_tool.show_panel) {
            polygon_tool.armed_for_placement = false;
            polygon_tool.verts.clear();
            project.rebuildAttachedCubemapFromProject();
        }
    }

    if (!polygon_tool.show_panel) {
        polygon_tool.armed_for_placement = false;
    }

    // Primitives Panel
    {
        const float outliner_height = viewport->WorkSize.y - menu_bar_height - (2.0f * panel_margin);

        if (outliner.collapsed) {
            ImGui::SetNextWindowPos(
                    ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - collapsed_outliner_width - panel_margin, panel_y),
                    ImGuiCond_Always
            );
            ImGui::SetNextWindowSize(ImVec2(collapsed_outliner_width, outliner_height), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.94f);

            ImGuiWindowFlags collapsedFlags =
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoTitleBar;

            if (ImGui::Begin("##PrimitivesCollapsed", nullptr, collapsedFlags)) {
                ImGui::Dummy(ImVec2(0.0f, 4.0f * ui_scale));
                if (ImGui::ArrowButton("##ExpandPrimitives", ImGuiDir_Left)) {
                    outliner.collapsed = false;
                }
            }
            ImGui::End();
        } else {
            ImGui::SetNextWindowPos(
                    ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - outliner.panel_size.x - panel_margin, panel_y),
                    ImGuiCond_Always
            );
            ImGui::SetNextWindowSize(outliner.panel_size, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.96f);

            ImGuiWindowFlags outlinerFlags =
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoCollapse;

            if (ImGui::Begin("Primitives", nullptr, outlinerFlags)) {
                outliner.panel_size = ImGui::GetWindowSize();

                ImGui::TextUnformatted("Outliner");
                const float collapse_button_x =
                        ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - ImGui::GetFrameHeight();
                ImGui::SameLine();
                ImGui::SetCursorPosX(collapse_button_x);
                if (ImGui::ArrowButton("##CollapsePrimitives", ImGuiDir_Right)) {
                    outliner.collapsed = true;
                }

                ImGui::Separator();

                if (outliner.selected_primitive_id != 0 &&
                    project.primitives.find(outliner.selected_primitive_id) == project.primitives.end()) {
                    outliner.selected_primitive_id = 0;
                    outliner.name_buffer_primitive_id = 0;
                    outliner.name_buffer[0] = '\0';
                }

                const float details_min_height = 450.0f * ui_scale;
                float list_height = ImGui::GetContentRegionAvail().y - details_min_height;
                list_height = std::max(list_height, 170.0f * ui_scale);

                ImGui::BeginChild("##PrimitiveList", ImVec2(0, list_height), true);

                for (const auto& layer : project.layers) {
                    ImGui::PushID((int)layer.id);
                    ImGui::Text("%s%s", layer.name.c_str(), layer.visible ? "" : " (hidden)");
                    ImGui::Separator();

                    bool layer_disabled = !layer.visible;
                    if (layer_disabled) ImGui::BeginDisabled(true);

                    for (uint32_t pid : layer.primitiveIDs) {
                        auto it = project.primitives.find(pid);
                        if (it == project.primitives.end() || !it->second) continue;

                        Primitive* base = it->second.get();
                        std::string label = primitiveDisplayName(*base) + "##prim_" + std::to_string(pid);

                        bool is_selected = (pid == outliner.selected_primitive_id);
                        if (ImGui::Selectable(label.c_str(), is_selected)) {
                            outliner.selected_primitive_id = pid;
                        }
                    }

                    if (layer_disabled) ImGui::EndDisabled();
                    ImGui::Spacing();
                    ImGui::PopID();
                }

                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::Separator();

                if (outliner.selected_primitive_id == 0) {
                    ImGui::TextWrapped("Select a primitive to inspect and edit it.");
                } else {
                    auto it = project.primitives.find(outliner.selected_primitive_id);
                    if (it != project.primitives.end() && it->second) {
                        Primitive* base = it->second.get();

                        ImGui::Text("Selected ID: %u", outliner.selected_primitive_id);
                        ImGui::Text("Type: %s",
                                (base->getType() == PrimitiveType::Point)    ? "Point" :
                                (base->getType() == PrimitiveType::Polyline) ? "Polyline" :
                                (base->getType() == PrimitiveType::Polygon)  ? "Polygon" : "Unknown");

                        bool changed = false;

                        if (outliner.name_buffer_primitive_id != outliner.selected_primitive_id) {
                            copyStringToBuffer(base->getName(), outliner.name_buffer, sizeof(outliner.name_buffer));
                            outliner.name_buffer_primitive_id = outliner.selected_primitive_id;
                        }

                        ImGui::Spacing();
                        ImGui::TextUnformatted("Name");
                        ImGui::SetNextItemWidth(-1.0f);
                        if (ImGui::InputText("##SelectedPrimitiveName", outliner.name_buffer, IM_ARRAYSIZE(outliner.name_buffer))) {
                            std::string new_name = outliner.name_buffer;
                            if (new_name.empty()) {
                                new_name = project.makeDefaultPrimitiveName(base->getType(), base->getID());
                                copyStringToBuffer(new_name, outliner.name_buffer, sizeof(outliner.name_buffer));
                            }
                            base->setName(new_name);
                        }

                        if (base->getType() == PrimitiveType::Point) {
                            auto* pt = dynamic_cast<PointPrimitive*>(base);
                            if (pt) {
                                ImGui::TextUnformatted("Color");
                                ImGui::SetNextItemWidth(-1.0f);
                                changed |= ImGui::ColorEdit4(
                                        "##SelectedPointColor",
                                        glm::value_ptr(pt->color),
                                        ImGuiColorEditFlags_NoInputs |
                                        ImGuiColorEditFlags_AlphaBar |
                                        ImGuiColorEditFlags_AlphaPreviewHalf
                                );

                                ImGui::Spacing();
                                ImGui::TextUnformatted("Point Size");
                                changed |= drawNormalizedSizeSlider("##SelectedPointSize", pt->size);
                                drawSliderExtentsText("Smaller", "Larger");
                            }
                        }
                        else if (base->getType() == PrimitiveType::Polyline) {
                            auto* line = dynamic_cast<PolylinePrimitive*>(base);
                            if (line) {
                                ImGui::Text("Vertices: %d", static_cast<int>(line->verts.size()));

                                ImGui::Spacing();
                                ImGui::TextUnformatted("Color");
                                ImGui::SetNextItemWidth(-1.0f);
                                changed |= ImGui::ColorEdit4(
                                        "##SelectedPolylineColor",
                                        glm::value_ptr(line->color),
                                        ImGuiColorEditFlags_NoInputs |
                                        ImGuiColorEditFlags_AlphaBar |
                                        ImGuiColorEditFlags_AlphaPreviewHalf
                                );

                                ImGui::Spacing();
                                ImGui::TextUnformatted("Line Width");
                                changed |= drawNormalizedWidthSlider("##SelectedPolylineWidth", line->width);
                                drawSliderExtentsText("Thinner", "Thicker");
                            }
                        }
                        else if (base->getType() == PrimitiveType::Polygon) {
                            auto* polygon = dynamic_cast<PolygonPrimitive*>(base);
                            if (polygon) {
                                ImGui::Text("Vertices: %d", static_cast<int>(polygon->verts.size()));

                                ImGui::Spacing();
                                ImGui::TextUnformatted("Color");
                                ImGui::SetNextItemWidth(-1.0f);
                                changed |= ImGui::ColorEdit4(
                                        "##SelectedPolygonColor",
                                        glm::value_ptr(polygon->color),
                                        ImGuiColorEditFlags_NoInputs |
                                        ImGuiColorEditFlags_AlphaBar |
                                        ImGuiColorEditFlags_AlphaPreviewHalf
                                );

                                ImGui::Spacing();
                                ImGui::TextDisabled("%s", "Polygon currently renders as a closed outline until backend polygon rendering is finished.");
                            }
                        }
                        else {
                            ImGui::TextWrapped("Editing for this primitive type isn't implemented yet.");
                        }

                        ImGui::Spacing();
                        ImGui::Separator();

                        if (ImGui::Button("Delete Primitive", ImVec2(-1.0f, 0.0f))) {
                            bool ok = project.deletePrimitive(outliner.selected_primitive_id);
                            if (ok) {
                                outliner.selected_primitive_id = 0;
                                outliner.name_buffer_primitive_id = 0;
                                outliner.name_buffer[0] = '\0';
                                project.rebuildAttachedCubemapFromProject();
                            }
                        }

                        if (changed) {
                            project.rebuildAttachedCubemapFromProject();
                        }
                    }
                }

                drawPinnedResizeHandle(
                        "##OutlinerResizeHandle",
                        outliner.panel_size,
                        ResizeHandleCorner::BottomLeft,
                        ui_scale,
                        300.0f * ui_scale,
                        500.0f * ui_scale,
                        viewport->WorkSize.x * 0.50f,
                        max_panel_height
                );
            }
            ImGui::End();
        }
    }

    ImGui::Render();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.draw(camera);

    draw_data = ImGui::GetDrawData();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}