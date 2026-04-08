//
// Created by Nathan on 4/1/2026.
//

#ifndef LINALG_H
#define LINALG_H

#include "opengl_include.h"

inline bool is_coplanar(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    return glm::abs(glm::determinant(glm::mat3(a, b, c))) < CHECK_TRESHHOLD;
}

#endif //LINALG_H
