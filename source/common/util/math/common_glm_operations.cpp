//
// Created by Nathan on 2/27/2026.
//

#include "common_glm_operations.h"

glm::vec4 make_vec4_w1(const glm::vec3& base) {
    return glm::vec4(base, 1.0f);
}

glm::vec3 apply_rotation(const glm::mat4x4& rot, const glm::vec3& base) {
    return glm::vec3(rot * make_vec4_w1(base));
}