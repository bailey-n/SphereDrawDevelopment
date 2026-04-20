#include "camera.h"
#include <iostream>

Camera::Camera() :
position(0.0f, 0.0f, 0.0f), target(1.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f),
view(glm::mat4(1.0f)), projection(glm::mat4(1.0f)), scr_width(0), scr_height(0) {}

Camera::Camera(const glm::mat4 &projection) :
position(0.0f, 0.0f, 0.0f), target(1.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f),
view(glm::lookAt(position, target, up)), projection(projection), scr_width(0), scr_height(0) {}

Camera::Camera(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up, const glm::mat4 projection)
: position(position), target(position + direction), up(up),
view(glm::lookAt(position, target, up)), projection(projection), scr_width(0), scr_height(0) {}

void Camera::update_view() {
    view = glm::lookAt(position, target, up);
    change_bits = 2;
}

void Camera::set_position(const glm::vec3 &pos) {
    this->position = pos;
    update_view();
}

void Camera::set_direction(const glm::vec3 &dir) {
    this->target = position + dir;
    update_view();
}

void Camera::set_up(const glm::vec3 &up) {
    this->up = up;
    update_view();
}

void Camera::set_projection(const glm::mat4 &projection) {
    this->projection = projection;
    update_view();
}

glm::vec3 Camera::get_position() const {
    return position;
}

glm::vec3 Camera::get_up() const {
    return up;
}

void Camera::rotate_position_left(float angle_rad) {
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), -angle_rad, up);
    glm::vec4 new_position = rotation_matrix * glm::vec4(position, 1.0f);
    position.x = new_position.x;
    position.y = new_position.y;
    position.z = new_position.z;
    update_view();
}

void Camera::rotate_position_right(float angle_rad) {
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_rad, up);
    glm::vec4 new_position = rotation_matrix * glm::vec4(position, 1.0f);
    position.x = new_position.x;
    position.y = new_position.y;
    position.z = new_position.z;
    update_view();
}

void Camera::rotate_position_up(float angle_rad) {
    glm::vec3 rotation_axis = glm::cross(position, up);
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_rad, rotation_axis);

    glm::vec4 new_position = rotation_matrix * glm::vec4(position, 1.0f);
    glm::vec4 new_up = rotation_matrix * glm::vec4(up, 1.0f);

    position.x = new_position.x;
    position.y = new_position.y;
    position.z = new_position.z;

    up.x = new_up.x;
    up.y = new_up.y;
    up.z = new_up.z;

    update_view();
}

void Camera::rotate_position_down(float angle_rad) {
    glm::vec3 rotation_axis = glm::cross(position, up);
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), -angle_rad, rotation_axis);

    glm::vec4 new_position = rotation_matrix * glm::vec4(position, 1.0f);
    glm::vec4 new_up = rotation_matrix * glm::vec4(up, 1.0f);

    position.x = new_position.x;
    position.y = new_position.y;
    position.z = new_position.z;

    up.x = new_up.x;
    up.y = new_up.y;
    up.z = new_up.z;

    update_view();
}

void Camera::rotate_up_left(float angle_rad) {
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_rad, position);
    glm::vec4 new_up = rotation_matrix * glm::vec4(up, 1.0f);
    up.x = new_up.x;
    up.y = new_up.y;
    up.z = new_up.z;
    update_view();
}

void Camera::rotate_up_right(float angle_rad) {
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), -angle_rad, position);
    glm::vec4 new_up = rotation_matrix * glm::vec4(up, 1.0f);
    up.x = new_up.x;
    up.y = new_up.y;
    up.z = new_up.z;
    update_view();
}

void Camera::move_to(const glm::vec3 &new_position, bool lock_camera) {
    if (!lock_camera) { target += (new_position - position); }
    this->position = new_position;
    update_view();
}

void Camera::look_at(const glm::vec3 &location) {
    this->target = location;
    update_view();
}

bool Camera::bind(GLuint program, const glm::mat4 &model) const {
    glViewport(0, 0, scr_width, scr_height);

    const GLint mvpID = glGetUniformLocation(program, "MVP");
    if (mvpID != -1) {
        glm::mat4 MVP = projection * view * model;
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(MVP));
    }

    GLint cameraPositionID = glGetUniformLocation(program, "cameraPosition");
    if (cameraPositionID != -1) {
        glUniform3fv(cameraPositionID, 1, glm::value_ptr(this->position));
    }
    return true;
}

void Camera::update_window(GLFWwindow *window) {
    glfwGetWindowSize(window, &scr_width, &scr_height);
    set_projection(glm::perspective(glm::radians(60.0f), (float)scr_width / (float)scr_height, 0.1f, 100.0f));
    glViewport(0, 0, scr_width, scr_height);
    // std::cout << scr_width << " " << scr_height << std::endl;
}

bool Camera::pop_change() {
    if (change_bits > 0) {
        change_bits--;
        return true;
    }
    return false;
}