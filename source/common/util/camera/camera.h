#ifndef CAMERA_H
#define CAMERA_H

#include "opengl_include.h"
#include <functional>

class Camera {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    glm::mat4 view;
    glm::mat4 projection;

    void update_view();
    unsigned int change_bits = 2;

public:
    uint32_t scr_width;
    uint32_t scr_height;

    Camera();
    explicit Camera(const glm::mat4& projection);
    Camera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up, glm::mat4 projection);

    void set_projection(const glm::mat4& projection);
    void set_position(const glm::vec3& position);
    void set_direction(const glm::vec3& direction);
    void set_up(const glm::vec3& up);

    [[nodiscard]] glm::vec3 get_position() const;
    [[nodiscard]] glm::vec3 get_up() const;

    void rotate_position_left(float angle_rad);
    void rotate_position_right(float angle_rad);
    void rotate_position_up(float angle_rad);
    void rotate_position_down(float angle_rad);
    void rotate_up_left(float angle_rad);
    void rotate_up_right(float angle_rad);

    void move_to(const glm::vec3& new_position, bool lock_camera = true);
    void look_at(const glm::vec3& location);

    [[nodiscard]] bool bind(GLuint program, const glm::mat4& model) const;
    bool pop_change();
};


#endif //FANTASYPLATES_CAMERA_H
