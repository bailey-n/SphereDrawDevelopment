//
// Created by Nathan on 2/27/2026.
//

#include "common_glm_operations.h"

glm::vec4 make_vec4_w1(const glm::tvec3<float>& base) {
    return glm::vec4(base, 1.0f);
}

glm::vec3 apply_rotation(const glm::mat4x4& rot, const glm::vec3& base) {
    return glm::vec3(rot * make_vec4_w1(base));
}

std::vector<glm::u8vec4> rgba_white_square_1024() {
    return {1024 * 1024, {255, 255, 255, 255}};
}

std::vector<glm::u8> r_black_square_1024() {
    return std::vector<glm::u8>(1024 * 1024, 0);
}

void print_current_bound_VAO() {
    GLint boundVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVAO);
    std::cout << "Bound VAO: " << boundVAO << std::endl;
}

// std::tuple<std::vector<glm::vec2>, std::vector<glm::vec2>> split_line(std::vector<glm::vec2>& points) {
//     glm::vec2& x1 = points[0];
//     glm::vec2& x2 = points[points.size()-1];
//     glm::vec2 dir = x2 - x1;
//     glm::vec2 norm = {dir.x, -dir.y};
//     std::vector<glm::vec2> below;
//     std::vector<glm::vec2> above;
//
//     bool curr_above = glm::dot((points[1] - x1), norm) > 0.00001f;
//     for (int i = 2; i < points.size(); i++) {
//         if ()
//     }
// }
