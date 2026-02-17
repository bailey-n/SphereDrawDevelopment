#include "application.h"
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include "shapes.h"

// ################################################## //

// Static variable initialization
const int Application::w_width = 1200;
const int Application::w_height = 900;
GLFWwindow* Application::window = nullptr;
bool Application::initialized = false;
bool Application::gui_change = true;
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
    initialized = true;
    return initialized;
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
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
}

Application::~Application() {
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

// ################################################## //

// APPLICATION MAINLOOP
void Application::mainloop() {
    // FOR TESTING SPEED & STABILITY
//    PointPrimitive test_point(0);
//    std::random_device rd;
//    std::default_random_engine rng(rd());
//    std::uniform_real_distribution<float> long_dist(-M_PI, M_PI);
//    std::uniform_real_distribution<float> lat_dist(-M_PI/2.0, M_PI/2.0);
//    std::uniform_real_distribution<float> color_dist(0.0f, 1.0f);
//    for (int i = 0; i < 500; i++) {
//        test_point.p = lat_lon_to_xyz(lat_dist(rng), long_dist(rng), 1.0f);
//        test_point.color = glm::vec4(color_dist(rng), color_dist(rng), color_dist(rng), 1.0f);
//        renderer.add_new_point(test_point);
//    }
    // END TEST

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
                    (float)planet.get_radius(),
                    glm::vec2((float)x_pos, (float)y_pos),
                    (float)w_width, (float)w_height,
                    glm::radians(60.0)
                    );
            if (std::isnan(click_coords.first)) {
                std::cout << "latitude: NaN\nlongitude: NaN" << std::endl;
                break;
            }
            std::cout << "latitude: " << glm::degrees(click_coords.first) << "\nlongitude: " << glm::degrees(click_coords.second) << std::endl;
            switch (state.draw_mode) {
                case State::DrawMode::Point:
                    break;
                case State::DrawMode::Polyline:
                break;
            }

            break;

        // ######## DRAW MODE ######## //
        case DRAW_MODE_ROTATE:
            state.draw_mode += 1;
            state.draw_mode %= 3;
            std::cout << "New draw state: " << (int)(state.draw_mode) << std::endl;

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
//    if (camera.pop_change() || ImGui::GetIO().WantCaptureMouse) {
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        planet.draw(camera);
//    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // // ImGui::ShowDemoWindow();
    SphereDrawGUI::MainMenuBar();
    ImGui::Render();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    planet.draw(camera);
    renderer.draw(camera);
    draw_data = ImGui::GetDrawData();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
