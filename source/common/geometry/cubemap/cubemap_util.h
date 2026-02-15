//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_CUBEMAP_UTIL_H
#define SPHEREDRAW_CUBEMAP_UTIL_H

// Reserve max uint32 value for invalid ids, so we can only have uint32_max - 1 features
#define InvalidId UINT32_MAX
#define MaxFeatureCt 0xfffffffeU

#include <cstdint>
#include <climits>
#include "glm/vec3.hpp"

using CubeMapId = uint32_t;
using CubeFaceFlags = uint8_t;

enum ObjectType: uint32_t {
    InvalidObject = UINT32_MAX,
    renderLayer = 0,
    renderPoint = 1,
    renderLine = 2,
    renderPolygon = 3,
};

// "d" is for direction
enum CubeFaceNum : uint32_t {
    North = 0,
    West = 1,
    Meridian = 2,
    East = 3,
    AntiMeridian = 4,
    South = 5
};

enum CubeFaceFlag : uint8_t {
    flagNorth = 1,
    flagWest = 2,
    flagMeridian = 4,
    flagEast = 8,
    flagAntiMeridian = 16,
    flagSouth = 32,
};

struct DrawnPrimitiveInfo {
    ObjectType type;
    CubeMapId id;
    CubeMapId parent_layer;
    uint32_t draw_position;
    CubeFaceFlags face_flags;
};

CubeFaceNum get_face(const glm::vec3& position);

#endif //SPHEREDRAW_CUBEMAP_UTIL_H
