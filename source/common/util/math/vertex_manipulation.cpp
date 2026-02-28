//
// Created by Nathan on 2/27/2026.
//


#include "vertex_manipulation.h"
#include "cubemap_texture_util.h"
#include <cstdint>
#include <cmath>
#include <queue>
#include <utility>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

glm::vec3 slerp(const glm::vec3& u, const glm::vec3& v, float t) {
    const float angle = std::acos(glm::dot(u, v));
    return glm::normalize(((std::sin((1.0f-t)*angle) * u) + (std::sin(t*angle) * v) / std::sin(angle)));
}

std::vector<glm::vec3> subdivide_line(const glm::vec3&  u, const glm::vec3& v, double max_subdiv_angle) {
    if (glm::dot(u, v) > 0.9999) return {u, v};
    double total_angle = std::acos(glm::dot(u, v));
    float divs = std::ceil(total_angle / max_subdiv_angle);
    int num_div = (int)divs;

    std::vector<glm::vec3> output;
    output.reserve(num_div + 1);

    output.emplace_back(u);
    for (int i = 1; i < num_div; i++) {
        float t = (float)i / divs;
        output.emplace_back(slerp(u, v, t));
    }
    output.emplace_back(v);

    return output;
}

using Edge = std::pair<uint32_t, uint32_t>;
using Triangle = glm::u32vec3;

void make_sphere_circle(glm::vec3 center, float radius, std::vector<glm::vec3>& positions_buff, std::vector<glm::u32vec3>& indices_buff) {
    // Approximate a circle
    // TODO: make the circle look better dynamically using: function of radius, then geometry shader
    constexpr int APPROX_DEPTH = 4;
    // Initial triangle: equilateral triangle inscribed in the unit circle
    std::vector<glm::vec2> vertices = {
        glm::vec2(1.0, 0.0),
        glm::vec2(-0.5, std::sqrt(0.75)),
        glm::vec2(-0.5, -std::sqrt(0.75)),
    };

    // Edges to expand & triangles to inscribe. Note the use of queue pointer swapping.
    std::queue<Triangle> triangles_q1{{Triangle{0, 1, 2}}};
    std::queue<Triangle> triangles_q2;
    std::queue<Edge> edges_q1 {{Edge(0, 1), Edge(1, 2), Edge(2, 0)}};
    std::queue<Edge> edges_q2;
    std::queue<Triangle>* curr_triangles = &triangles_q1;
    std::queue<Triangle>* new_triangles = &triangles_q2;
    std::queue<Edge>* curr_edges = &edges_q1;
    std::queue<Edge>* new_edges = &edges_q2;

    // Make layers of triangles to approximate a circle
    for (int i = 1; i < APPROX_DEPTH; i++) {
        while (!curr_edges->empty()) {
            Edge curr_edge = curr_edges->front();
            curr_edges->pop();

            // Get midpoint of Edge and push it to edge of circle by normalizing it
            uint32_t new_vtx_idx = vertices.size();
            vertices.emplace_back(glm::normalize(vertices[curr_edge.first] + vertices[curr_edge.second]));
            new_edges->emplace(curr_edge.first, new_vtx_idx);
            new_edges->emplace(new_vtx_idx, curr_edge.second);
            new_triangles->emplace(curr_edge.first, new_vtx_idx, curr_edge.second);
        }
        while (!curr_triangles->empty()) {
            Triangle curr_triangle = curr_triangles->front();
            curr_triangles->pop();

            // Get triangle vertices from midpoint of triangles.
            uint32_t new_idx = vertices.size();
            // Inscribes triangle via finding edge midpoints
            vertices.emplace_back(0.5f * (vertices[curr_triangle.x] + vertices[curr_triangle.y]));
            vertices.emplace_back(0.5f * (vertices[curr_triangle.y] + vertices[curr_triangle.z]));
            vertices.emplace_back(0.5f * (vertices[curr_triangle.z] + vertices[curr_triangle.x]));
            // Subdivides into four triangles
            new_triangles->emplace(new_idx, new_idx+1, new_idx+2);
            new_triangles->emplace(curr_triangle.x, new_idx, new_idx+2);
            new_triangles->emplace(curr_triangle.y, new_idx+1, new_idx);
            new_triangles->emplace(curr_triangle.z, new_idx+2, new_idx+1);
        }

        std::swap(curr_triangles, new_triangles);
        std::swap(curr_edges, new_edges);
    }
    // Note: we end with curr_triangles pointer containing all new triangles generated in the last round.
    // We will discard all previous triangles and use only the new triangles.
    // First, we map the 2d circle coords to a hemisphere
    float scale = std::sin(radius);
    glm::tmat4x4<float> transform = glm::mat4x4(1.0f); // Scaling
    glm::vec3 z_axis{0.0, 0.0, 1.0};
    float z_dot = glm::dot(center, z_axis);
    if (z_dot != 1.0) { // if z_dot == 1, then the normalization will fail
        transform = glm::rotate(transform, std::acos(z_dot), glm::normalize(glm::cross(z_axis, center)));
    }

    // Modify mesh data directly (to avoid unnecessary copies)
    positions_buff.clear();
    positions_buff.reserve(vertices.size());
    for (const auto& vertex: vertices) {
        glm::vec4 base_point(scale * vertex.x, scale * vertex.y, std::cos(radius), 1.0);
        glm::vec3 point = glm::normalize(glm::vec3(transform * base_point));
        positions_buff.emplace_back(point);
    }

    indices_buff.clear();
    indices_buff.reserve(curr_triangles->size());
    while (!curr_triangles->empty()) {
        indices_buff.emplace_back(curr_triangles->front());
        curr_triangles->pop();
    }
}