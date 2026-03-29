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
    if (meridian_val >= west_val) {
        if (position.x > 0) return Meridian;
        else return AntiMeridian;
    }
    if (position.z > 0) return West;
    else return East;
}

transitionAngles find_border_angles(const glm::vec3& position, CubeFaceNum face) {
    // Notes on border vs normal for plane which makes that border:
    /* North Face: +y
     * [left: (0, 1, -1)] [bottom: (-1, 1, 0)] [right: (0, 1, 1)] [top: (1, 1, 0)] <-> 2 9 3 11
     * [bl: (1, 1, 1)] [br: (1, 1, -1)] [tr: (-1, 1, -1)] [tl: (-1, 1, 1)]         <-> 7 6 2 3
     */
    /* West Face: +z
     * [left: (1, 0, 1)] [bottom: (0, 1, 1)] [right: (1, 0, -1)] [top: (0, 1, -1)] <-> 7 3 6 2
     * [bl: (-1, -1, 1)] [br: (1, -1, 1)] [tr: (1, 1, 1)] [tl: (-1, 1, 1)]         <-> 1 5 7 3
     */
    /* Meridian Face: +x
     * [left: (1, 0, -1)] [bottom: (1, 1, 0)] [right: (1, 0, 1)] [top: (-1, 1, 0)] <-> 6 11 7 9
     * [bl: (1, -1, 1)] [br: (1, -1, -1)] [tr: (1, 1, -1)] [tl: (1, 1, 1)]         <-> 5 4 6 7
     */
    /* East Face: -z
     * [left: (1, 0, 1)] [bottom: (0, 1, -1)] [right: (1, 0, -1)] [top: (0, 1, 1)] <-> 7 2 6 3
     * [bl: (1, -1, -1)] [br: (-1, -1, -1)] [tr: (-1, 1, -1)] [tl: (1, 1, -1)]     <-> 4 0 2 6
     */
    /* AntiMeridian Face: -x
     * [left: (1, 0, -1)] [bottom: (-1, 1, 0)] [right: (1, 0, 1)] [top: (1, 1, 0)] <-> 6 9 7 11
     * [bl: (-1, -1, -1)] [br: (-1, -1, 1)] [tr: (-1, 1, 1)] [tl: (-1, 1, -1)]     <-> 0 1 3 2
     */
    /* South Face: -y
     * [left: (0, 1, 1)] [bottom: (-1, 1, 0)] [right: (0, 1, -1)] [top: (1, 1, 0)] <-> 3 9 2 11
     * [bl: (-1, -1, 1)] [br: (-1, -1, -1)] [tr: (1, -1, -1)] [tl: (1, -1, 1)]     <-> 1 0 4 5
     */

    constexpr float k = std::sqrt(1.0f / 2.0f); // For normalizing the normals
    glm::vec3 plane_normals[12] = {
        {0.0, -k, -k}, {0.0, -k, k}, {0.0, k, -k}, {0.0, k, k},
        {-k, 0.0,-k}, {-k, 0.0, k}, {k, 0.0, -k}, {k, 0.0, k},
        {-k,-k, 0.0}, {-k, k, 0.0}, {k, -k, 0.0}, {k, k, 0.0},
    };
    constexpr float c = std::sqrt(1.0f / 3.0f);
    glm::vec3 corners[8] = {
    {-c, -c, -c},
    {-c, -c, c},
    {-c, c, -c},
    {-c, c, c},
    {c, -c, -c},
    {c, -c, c},
    {c, c, -c},
    {c, c, c},
    };
    
    switch(face) {
    case North: return {
        normalized_vec_plane_angle(position, plane_normals[2]),
        normalized_vec_plane_angle(position, plane_normals[9]),
        normalized_vec_plane_angle(position, plane_normals[3]),
        normalized_vec_plane_angle(position, plane_normals[11]),
        normalized_vec_vec_angle(position, corners[7]),
        normalized_vec_vec_angle(position, corners[6]),
        normalized_vec_vec_angle(position, corners[2]),
        normalized_vec_vec_angle(position, corners[3]),
        };
    case West: return {
        normalized_vec_plane_angle(position, plane_normals[7]),
        normalized_vec_plane_angle(position, plane_normals[3]),
        normalized_vec_plane_angle(position, plane_normals[6]),
        normalized_vec_plane_angle(position, plane_normals[2]),
        normalized_vec_vec_angle(position, corners[1]),
        normalized_vec_vec_angle(position, corners[5]),
        normalized_vec_vec_angle(position, corners[7]),
        normalized_vec_vec_angle(position, corners[3]),
        };
    case Meridian: return {
        normalized_vec_plane_angle(position, plane_normals[6]),
        normalized_vec_plane_angle(position, plane_normals[11]),
        normalized_vec_plane_angle(position, plane_normals[7]),
        normalized_vec_plane_angle(position, plane_normals[9]),
        normalized_vec_vec_angle(position, corners[5]),
        normalized_vec_vec_angle(position, corners[4]),
        normalized_vec_vec_angle(position, corners[6]),
        normalized_vec_vec_angle(position, corners[7]),
        };
    case East: return {
        normalized_vec_plane_angle(position, plane_normals[7]),
        normalized_vec_plane_angle(position, plane_normals[2]),
        normalized_vec_plane_angle(position, plane_normals[6]),
        normalized_vec_plane_angle(position, plane_normals[3]),
        normalized_vec_vec_angle(position, corners[4]),
        normalized_vec_vec_angle(position, corners[0]),
        normalized_vec_vec_angle(position, corners[2]),
        normalized_vec_vec_angle(position, corners[6]),
        };
    case AntiMeridian: return {
        normalized_vec_plane_angle(position, plane_normals[6]),
        normalized_vec_plane_angle(position, plane_normals[9]),
        normalized_vec_plane_angle(position, plane_normals[7]),
        normalized_vec_plane_angle(position, plane_normals[11]),
        normalized_vec_vec_angle(position, corners[0]),
        normalized_vec_vec_angle(position, corners[1]),
        normalized_vec_vec_angle(position, corners[3]),
        normalized_vec_vec_angle(position, corners[2]),
        };
    case South: return {
        normalized_vec_plane_angle(position, plane_normals[3]),
        normalized_vec_plane_angle(position, plane_normals[9]),
        normalized_vec_plane_angle(position, plane_normals[2]),
        normalized_vec_plane_angle(position, plane_normals[11]),
        normalized_vec_vec_angle(position, corners[1]),
        normalized_vec_vec_angle(position, corners[0]),
        normalized_vec_vec_angle(position, corners[4]),
        normalized_vec_vec_angle(position, corners[5]),
    }   ;
    }
    return {
        std::nanf(""), std::nanf(""), std::nanf(""), std::nanf(""),
        std::nanf(""), std::nanf(""), std::nanf(""), std::nanf("")
    };
}

quadraticSolComponents::quadraticSolComponents(float x0, float y0, float z0, float y, float z, float r) {
    float cos_r2 = std::cos(r);
    cos_r2 *= cos_r2;
    float yz_dot = y0*y + z0*z;
    float rx = (x0*x0 - cos_r2);
    float ry = (y0*y0 - cos_r2);
    float rz = (z0*z0 - cos_r2);

    base = -x0*yz_dot;
    discrim = x0*x0*yz_dot*yz_dot - rx*(y*y*ry + 2*y0*z0*y*z + z*z*rz);
    denom = rx;
}

float quadraticSolComponents::get_positive_sol() {
    if (discrim < 0.0f) return std::nanf("");
    return (base + std::sqrt(discrim))/denom;
}

float quadraticSolComponents::get_negative_sol() {
    if (discrim < 0.0f) return std::nanf("");
    return (base - std::sqrt(discrim))/denom;
}


circleBorderIntersects find_border_intersects(const glm::vec3& position, float r, CubeFaceNum face) {
    // Notes on border vs fixed values (solve for #, invert answer if - appears to get u/v respectively):
    /* North: [left: (-#v, 1, 1)] [bottom: (1, 1, -#u)] [right: (-#v, 1, -1)] [top: (-1, 1, -#u)]       <-> 3 11 2 1
     *
     *
     * West: [left: (-1, #v, 1)] [bottom: (#u, -1, 1)] [right: (1, #v, 1)] [top: (#u, 1, 1)]            <-> 5 1 7 3
     *
     *
     * Meridian: [left: (1, #v, 1)] [bottom: (1, -1, -#u)] [right: (1, #v, -1)] [top: (1, 1, -#u)]      <-> 7 10 6 11
     *
     *
     * East: [left: (1, #v, -1)] [bottom: (-#u, -1, -1)] [right: (-1, #v, -1)] [top: (-#u, 1, -1)]      <-> 7 2 6 3
     *
     *
     * AntiMeridian: [left: (-1, #v, -1)] [bottom: (-1, -1, #u)] [right: (-1, #v, 1)] [top: (-1, 1, #u)]<-> 6 9 7 11
     *
     *
     * South: [left: (#v, -1, 1)] [bottom: (-1, -1, -#u)] [right: (#v, -1, -1)] [top: (1, -1, -#u)]     <-> 3 9 2 11
     */
    constexpr float k = 1.0f; // For normalizing the normals
    glm::vec3 plane_normals[12] = {
        {0.0f, -k, -k}, {0.0f, -k, k}, {0.0f, k, -k}, {0.0f, k, k},
        {-k, 0.0f,-k}, {-k, 0.0f, k}, {k, 0.0f, -k}, {k, 0.0f, k},
        {-k,-k, 0.0f}, {-k, k, 0.0f}, {k, -k, 0.0f}, {k, k, 0.0f},
    };

    quadraticSolComponents equation_components[4];

    // switch(face) {
    // case North:
    //     equation_components[0] = quadraticSolComponents()
    // }
    return {
        std::nanf(""), std::nanf(""), std::nanf(""), std::nanf(""),
        std::nanf(""), std::nanf(""), std::nanf(""), std::nanf("")
    };
}

