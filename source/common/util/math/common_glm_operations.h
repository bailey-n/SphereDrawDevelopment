//
// Created by Nathan on 2/27/2026.
//

#ifndef SPHEREDRAW_COMMON_GLM_OPERATIONS_H
#define SPHEREDRAW_COMMON_GLM_OPERATIONS_H

#include "opengl_include.h"
#include <vector>
#include <string>
#include <climits>
#include <optional>
#include <iostream>

inline glm::vec4 make_vec4_w1(const glm::vec3& base);
inline glm::vec3 apply_rotation(const glm::mat4x4& rot, const glm::vec3& vec);
std::vector<glm::u8vec4> rgba_white_square_1024();
void print_current_bound_VAO();

#endif //SPHEREDRAW_COMMON_GLM_OPERATIONS_H
