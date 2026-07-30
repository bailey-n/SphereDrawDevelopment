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
#include <tuple>

inline glm::vec4 make_vec4_w1(const glm::vec3& base);
glm::vec3 apply_rotation(const glm::mat4x4& rot, const glm::vec3& vec);
std::vector<glm::u8vec4> rgba_white_square_1024();
std::vector<glm::u8> r_black_square_1024();
void print_current_bound_VAO();
std::tuple<std::vector<glm::vec2>, std::vector<glm::vec2>> split_line(glm::vec2 x1, glm::vec2 x2, std::vector<glm::vec2> points);

#endif //SPHEREDRAW_COMMON_GLM_OPERATIONS_H
