//
// Created by Nathan on 2/27/2026.
//

#ifndef SPHEREDRAW_COMMON_GLM_OPERATIONS_H
#define SPHEREDRAW_COMMON_GLM_OPERATIONS_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <climits>
#include <optional>

inline glm::vec4 make_vec4_w1(const glm::vec3& base);
inline glm::vec3 apply_rotation(const glm::mat4x4& rot, const glm::vec3& vec);

#endif //SPHEREDRAW_COMMON_GLM_OPERATIONS_H
