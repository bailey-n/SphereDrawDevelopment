//
// Created by Nathan on 2/14/2026.
//

#include "cubemap_util.h"
#include <cmath>

CubeFaceNum get_face(const glm::vec3& position) {
    float latitude_val = std::abs(position.y);
    float meridian_val = std::abs(position.x);
    float west_val = std::abs(position.z);
    if (latitude_val >= meridian_val && latitude_val >= west_val) {
        if (position.y > 0) return North;
        else return South;
    }
    if (meridian_val >= latitude_val) {
        if (position.x > 0) return Meridian;
        else return AntiMeridian;
    }
    if (position.z > 0) return West;
    else return East;
}