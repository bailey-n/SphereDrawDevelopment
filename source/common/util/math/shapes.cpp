#include "shapes.h"
#include <cmath>
#include <iostream>
#include <iomanip>

SphereMeshData generate_cuboid_face_mesh_data(unsigned int vertices_per_edge) {
    const double k = sqrt(3.0) / 3.0;
    glm::vec3 vertices[8] = {
            // Upper positions
            {-k, k, -k},
            {k, k, -k},
            {k, k, k},
            {-k, k, k},
            // Lower positions
            {-k, -k, -k},
            {k, -k, -k},
            {k, -k, k,},
            {-k, -k, k}
    };
    glm::i32vec3 indices[12] = {
            {2, 1, 0},{3, 2, 0}, // North
            {7, 3, 0},{4, 7, 0}, // West
            {6, 2, 3},{7, 6, 3}, // Meridian
            {5, 1, 2},{6, 5, 2}, // East
            {4, 0, 1},{5, 4, 1}, // Far-East
            {5, 6, 7},{4, 5, 7} // South
    };
    glm::vec3 v_colors[6] = {
            {0.3f, 0.3f, 0.0f}, // North
            {0.3f, 0.0f, 0.0f}, // West
            {0.0f, 0.0f, 0.3f}, // Meridian
            {0.3f, 0.0f, 0.3f}, // East
            {0.0f, 0.3f, 0.0f}, // Far-East
            {0.0f, 0.3f, 0.3f} // South
    };
    std::vector<glm::vec3> positions(36);
    std::vector<glm::vec3> colors(36);

    for (int i = 0; i < 12; i++) {
        positions[3*i+0] = vertices[indices[i].x];
        positions[3*i+1] = vertices[indices[i].y];
        positions[3*i+2] = vertices[indices[i].z];
        colors[3*i+0] = v_colors[i/2];
        colors[3*i+1] = v_colors[i/2];
        colors[3*i+2] = v_colors[i/2];
    }

    constexpr double UV_LEFT_LEFT = 0.25;
    constexpr double UV_LEFT_RIGHT = 0.5;
    constexpr double UV_RIGHT_LEFT = 0.75;
    constexpr double UV_RIGHT_RIGHT = 1.0;
    constexpr double UV_TOP = 1.0 / 3.0;
    constexpr double UV_MIDDLE = 2.0 / 3.0;
    constexpr double UV_BOTTOM = 1.0;

    std::vector<glm::vec2> UV_OFFSETS = {
            {UV_LEFT_RIGHT, UV_TOP}, // BR of North face
            {UV_LEFT_LEFT, UV_MIDDLE}, // BR of West face
            {UV_LEFT_RIGHT, UV_MIDDLE}, // BR of Meridian face
            {UV_RIGHT_LEFT, UV_MIDDLE}, // BR of East face
            {UV_RIGHT_RIGHT, UV_MIDDLE}, // BR of Far-East face
            {UV_LEFT_RIGHT, UV_BOTTOM}, // BR of South face
    };

    // Safety
    if (vertices_per_edge < 2) {
        vertices_per_edge = 2;
    }

    unsigned int vertices_per_face = vertices_per_edge * vertices_per_edge;
    unsigned int triangles_per_face = 2*(vertices_per_edge - 1)*(vertices_per_edge - 1);

    SphereMeshData data(
            std::vector<glm::vec3>(6*vertices_per_face),
            std::vector<glm::vec3>(6*vertices_per_face),
            std::vector<glm::vec3>(6*vertices_per_face),
            std::vector<glm::vec2>(6*vertices_per_face),
            std::vector<glm::u32vec3>(6*triangles_per_face)
    );

    const glm::vec2 uv_dx = {-1.0f / (4.0f * (float)(vertices_per_edge-1)), 0.0f};
    const glm::vec2 uv_dy = {0.0f, -1.0f / (3.0f * (float)(vertices_per_edge-1))};
    // const float edge_cover = 0.01f; // Reduces visibility of face joins

    for (int face = 0; face < 6; face++) {
        const glm::vec3& BL = positions[6*face+0];
        const glm::vec3& TL = positions[6*face+1];
        const glm::vec3& BR = positions[6*face+3];

        const glm::vec2& base_uv = UV_OFFSETS[face];

        const glm::vec3& color = colors[6*face];
        const glm::vec3& dx = (BR - BL) / (float)(vertices_per_edge-1);
        const glm::vec3& dy = (TL - BL) / (float)(vertices_per_edge-1);

        unsigned int v_face_index = face * vertices_per_face;
        unsigned int t_face_index = face * triangles_per_face;

        // Set vertex data
        for (int y = 0; y < vertices_per_edge; y++) {
            for (int x = 0; x < vertices_per_edge; x++) {
                // Allow for overlap on +/- edge cover to hide joins
                // float face_x = (float)(vertices_per_edge-1)*((1.0f+2.0f*edge_cover)*((float)x / (float)(vertices_per_edge-1))-edge_cover);
                // float face_y = (float)(vertices_per_edge-1)*((1.0f+2.0f*edge_cover)*((float)y / (float)(vertices_per_edge-1))-edge_cover);
                // Calculate position and uv
                glm::vec3 pos = glm::normalize(BL + dx*(float)x + dy*(float)y);
                glm::vec2 uv = base_uv + uv_dx*(float)x + uv_dy*(float)y;
                unsigned int index = v_face_index + x + y*vertices_per_edge;
                data.positions[index] = pos;
                data.colors[index] = color;
                data.normals[index] = pos;
                data.uvs[index] = uv;
            }
        }
        // Set indices
        for (int y = 0; y < vertices_per_edge-1; y++) {
            for (int x = 0; x < vertices_per_edge-1; x++) {
                // Index of triangle in triangle indices array
                unsigned int bl_t_idx = t_face_index + 2*(x + y*(vertices_per_edge-1));
                unsigned int tr_t_idx = bl_t_idx + 1;
                // Index of square corners in positions array
                unsigned int bl_index = v_face_index + (x+0) + (y+0)*(vertices_per_edge);
                unsigned int tl_index = v_face_index + (x+0) + (y+1)*(vertices_per_edge);
                unsigned int br_index = v_face_index + (x+1) + (y+0)*(vertices_per_edge);
                unsigned int tr_index = v_face_index + (x+1) + (y+1)*(vertices_per_edge);
                // bl-tl-br
                data.indices[bl_t_idx] = {bl_index, tl_index, br_index};
                data.indices[tr_t_idx] = {br_index, tl_index, tr_index};
            }
        }
    }
    return data;
}

std::pair<float, float> xyz_to_lat_lon(const glm::vec3& xyz) {
    float longitude = std::atan2(-xyz.z, xyz.x);
    float latitude = std::atan2(xyz.y, std::hypot(xyz.x, xyz.z));
    return {latitude, longitude};
}

glm::vec3 lat_lon_to_xyz(const float latitude, const float longitude, const float radius) {
    return {
        radius * std::cos(latitude) * std::cos(longitude),
        radius * std::sin(latitude),
        -radius * std::cos(latitude) * std::sin(longitude),
    };
}

glm::vec3 sphere_click_xyz(const glm::vec3& position, const glm::vec3& up, float sphere_radius,
                                            const glm::vec2& screen_coords, float window_width, float window_height,
                                            float fov_y) {
    // Normalize positional coordinates
    auto unit_sphere_pos = (1.0f / sphere_radius) * position;
    float inv_window_height = 1.0f / window_height;
    float inv_window_width = 1.0f / window_width;
    // Range: [-1.0, 1.0] for x and [-1.0, 1.0] for y. 'cn' means centered and normalized.
    glm::vec2 scr_coords_cn = 2.0f * glm::vec2(inv_window_width, -inv_window_height) *
            (screen_coords - glm::vec2(window_width*0.5f, window_height*0.5f));

    glm::vec3 true_right = glm::normalize(glm::cross(up, unit_sphere_pos));
    glm::vec3 true_up = glm::normalize(glm::cross(unit_sphere_pos, true_right));
    glm::vec3 look_direction = glm::normalize(-unit_sphere_pos);

    float y_scalar = std::tan(0.5f*fov_y);
    float x_scalar = window_width * y_scalar * inv_window_height;
    glm::vec3 click_vertical = y_scalar * scr_coords_cn.y * true_up;
    glm::vec3 click_horizontal = x_scalar * scr_coords_cn.x * true_right;
    glm::vec3 click_direction = glm::normalize(look_direction + click_vertical + click_horizontal);

    // Compute unit sphere intersection of line { p(t) = pos + t*click_dir }
    float proj = 2.0f * glm::dot(click_direction, unit_sphere_pos);
    float dist_sqr = glm::dot(unit_sphere_pos, unit_sphere_pos);
    float discriminant = proj*proj - 4.0f*(dist_sqr-1.0f);

    if (discriminant < 0.0f) return {std::nanf(""), std::nanf(""), std::nanf("")}; // No solution

    float t = -0.5f * (proj + std::sqrt(discriminant));
    glm::vec3 unit_sphere_intersect = unit_sphere_pos + t*click_direction;
    return unit_sphere_intersect;
}

std::pair<float, float> sphere_click_lat_lon(const glm::vec3& position, const glm::vec3& up, const float sphere_radius,
                                            const glm::vec2& screen_coords, const float window_width, const float window_height,
                                            const float fov_y) {
    const glm::vec3 res = sphere_click_xyz(position, up, sphere_radius, screen_coords, window_width, window_height, fov_y);
    if (std::isnan(res.x)) return {std::nanf(""), std::nanf("")};
    return xyz_to_lat_lon(res);
}

CubeFaceMeshData get_cube_face_mesh_data(CubeFaceNum face) {
    /* North Face: [bl: (1, 1, 1)] [br: (1, 1, -1)] [tr: (-1, 1, -1)] [tl: (-1, 1, 1)]         <-> 7 6 2 3
     * West Face: [bl: (-1, -1, 1)] [br: (1, -1, 1)] [tr: (1, 1, 1)] [tl: (-1, 1, 1)]         <-> 1 5 7 3
     * Meridian Face: [bl: (1, -1, 1)] [br: (1, -1, -1)] [tr: (1, 1, -1)] [tl: (1, 1, 1)]         <-> 5 4 6 7
     * East Face: [bl: (1, -1, -1)] [br: (-1, -1, -1)] [tr: (-1, 1, -1)] [tl: (1, 1, -1)]     <-> 4 0 2 6
     * AntiMeridian Face: [bl: (-1, -1, -1)] [br: (-1, -1, 1)] [tr: (-1, 1, 1)] [tl: (-1, 1, -1)]     <-> 0 1 3 2
     * South Face: [bl: (-1, -1, 1)] [br: (-1, -1, -1)] [tr: (1, -1, -1)] [tl: (1, -1, 1)]     <-> 1 0 4 5
     */

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
    glm::vec2 uvs[4] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
    };
    glm::u32vec3 indices[2] = {
        {0, 1, 2}, {0, 2, 3}
    };

    switch (face) {
    case North: return {
        {corners[7], corners[6], corners[2], corners[3]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
            {indices[0], indices[1]}
    };
    case West: return {
        {corners[1], corners[5], corners[7], corners[3]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case Meridian: return {
        {corners[5], corners[4], corners[6], corners[7]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case East: return {
        {corners[4], corners[0], corners[2], corners[6]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case AntiMeridian: return {
        {corners[0], corners[1], corners[3], corners[2]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case South: return {
        {corners[1], corners[0], corners[4], corners[5]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    }
    return {
        {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f)},
        {glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f)},
        {glm::u32vec3(0), glm::u32vec3(0)}
    };
}
