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
#include <vector>

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
    CubeFaceFlags face_flags;
};

struct LayerPrimitiveInfo {
    CubeMapId id;
    CubeMapId end_id;
};

struct LineBuilderVertexInfo {
    enum lvType {
        lvStart = 0,
        lvMiddle = 1,
        lvEnd = 2
    };

    lvType ty;
    glm::vec3 pos;
    CubeFaceNum face;
    bool face_transition_before = false;
    bool face_transition_after = false;
};

CubeFaceNum get_face(const glm::vec3& position);

#endif //SPHEREDRAW_CUBEMAP_UTIL_H
