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
    return {4 * 1024 * 1024, {255, 255, 255, 255}};
}

void print_current_bound_VAO() {
    GLint boundVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVAO);
    std::cout << "Bound VAO: " << boundVAO << std::endl;
}