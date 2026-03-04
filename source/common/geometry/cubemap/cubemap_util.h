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
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

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
inline glm::vec2 to_face_uv(glm::vec3 position, CubeFaceNum face) {
    // Mapping notes:
    /* North face:
     * (0.0, 0.0)uv <-> (1.0, #, 1.0)xyz
     * (1.0, 0.0)uv <-> (1.0, #, -1.0)xyz (+u <-> -z)
     * (0.0, 1.0)uv <-> (-1.0, #, 1.0)xyz (+v <-> -x)
     * (1.0, 1.0)uv <-> (-1.0, #, -1.0)xyz
    */
    /* West face:
     * (0.0, 0.0)uv <-> (-1.0, -1.0, #)xyz
     * (1.0, 0.0)uv <-> (1.0, -1.0, #)xyz (+u <-> +x)
     * (0.0, 1.0)uv <-> (-1.0, 1.0, #)xyz (+v <-> +y)
     * (1.0, 1.0)uv <-> (1.0, 1.0, #)xyz
    */
    /* Meri face:
     * (0.0, 0.0)uv <-> (#, -1.0, 1.0)xyz
     * (1.0, 0.0)uv <-> (#, -1.0, -1.0)xyz (+u <-> -z)
     * (0.0, 1.0)uv <-> (#, 1.0, 1.0)xyz (+v <-> +y)
     * (1.0, 1.0)uv <-> (#, 1.0, -1.0)xyz
    */
    /* East face:
     * (0.0, 0.0)uv <-> (1.0, -1.0, #)xyz
     * (1.0, 0.0)uv <-> (-1.0, -1.0, #)xyz (+u <-> -x)
     * (0.0, 1.0)uv <-> (1.0, 1.0, #)xyz (+v <-> +y)
     * (1.0, 1.0)uv <-> (-1.0, 1.0, #)xyz
    */
    /* AnMe face:
     * (0.0, 0.0)uv <-> (#, -1.0, -1.0)xyz
     * (1.0, 0.0)uv <-> (#, -1.0, 1.0)xyz (+u <-> +z)
     * (0.0, 1.0)uv <-> (#, 1.0, -1.0)xyz (+v <-> +y)
     * (1.0, 1.0)uv <-> (#, 1.0, 1.0)xyz
    */
    /* South face:
     * (0.0, 0.0)uv <-> (-1.0, #, 1.0)xyz
     * (1.0, 0.0)uv <-> (-1.0, #, -1.0)xyz (+u <-> -z)
     * (0.0, 1.0)uv <-> (1.0, #, 1.0)xyz (+v <-> +x)
     * (1.0, 1.0)uv <-> (1.0, #, -1.0)xyz
    */

    // Scale values such that the relevant face coordinate i 1
    switch(face) {
    case North:
    case South:
        position /= std::abs(position.y);
        break;
    case West:
    case East:
        position /= std::abs(position.z);
        break;
    case Meridian:
    case AntiMeridian:
        position /= std::abs(position.x);
        break;
    }

    // Return values, flipped to match face uv
    switch(face) {
    case North: return {-position.z, -position.x};
    case West: return {position.x, position.y};
    case Meridian: return {-position.z, position.y};
    case East: return {-position.x, position.y};
    case AntiMeridian: return {position.z, position.y};
    case South: return {-position.z, position.x};
    }
    return {std::nanf(""), std::nanf("")};
}

inline glm::vec3 to_face_uv_vec3(glm::vec3 position, CubeFaceNum face) {
    return (glm::vec3(to_face_uv(position, face), 0.0f));
}

// Note: vec and plane are assumed to be normalized (may crashed otherwise)
inline float normalized_vec_plane_angle(glm::vec3 v, glm::vec3 n) {
    return std::asin(glm::dot(v, n));
}

inline float normalized_vec_vec_angle(glm::vec3 a, glm::vec3 b) {
    return std::acos(glm::dot(a, b));
}

// For detecting how far cubemap borders are from a point
struct transitionAngles {
    float un1_border_angle; // Left border of face square
    float vn1_border_angle; // Bottom border of face square
    float up1_border_angle; // Right border of face square
    float vp1_border_angle; // Top border of face square
    float un1vn1_corner_angle; // BL corner of square
    float up1vn1_corner_angle; // BR corner of square
    float up1vp1_corner_angle; // TR corner of square
    float un1vp1_corner_angle; // TL corner of square
};

transitionAngles find_border_angles(const glm::vec3& position, CubeFaceNum face);

// For finding where a circle around a point intersects the cubemap borders
struct circleBorderIntersects {
    float un1n_border_intersect; // Left border intersection 1
    float un1p_border_intersect; // Left border intersection 2
    float vn1n_border_intersect; // Bottom border intersection 1
    float vn1p_border_intersect; // Bottom border intersection 2
    float up1n_border_intersect; // Right border intersection 1
    float up1p_border_intersect; // Right border intersection 2
    float vp1n_border_intersect; // Top border intersection 1
    float vp1p_border_intersect; // Top border intersection 2
};

struct quadraticSolComponents {
    float base;
    float discrim;
    float denom;

    quadraticSolComponents() : base(std::nanf("")), discrim(std::nanf("")), denom(std::nanf("")) {}
    quadraticSolComponents(float x0, float y0, float z0, float y, float z, float r);
    float get_positive_sol();
    float get_negative_sol();
};

// NOT IMPLEMENTED YET. RETURNS NAN FOR ALL CATEGORIES.
circleBorderIntersects find_border_intersects(const glm::vec3& position, float r, CubeFaceNum face);

#endif //SPHEREDRAW_CUBEMAP_UTIL_H
