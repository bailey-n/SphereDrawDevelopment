//
// Created by Nathan on 2/14/2026.
//

#include "point_mesh.h"
#include "shader_manager.h"
#include "cubemap_texture_util.h"
#include <cmath>
#include <queue>
#include <utility>
#include <iostream>

using Edge = std::pair<uint32_t, uint32_t>;
using Triangle = glm::u32vec3;

PointMesh::PointMesh(glm::vec3 position, glm::vec4 color, float radius) :
mesh(), position(position), color(color), radius(std::min(radius, (float)M_PI)) {
    program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});

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
    float scale = std::sin(this->radius);
    glm::mat4 transform = glm::mat4x4(1.0f); // Scaling
    glm::vec3 z_axis{0.0, 0.0, 1.0};
    float z_dot = glm::dot(this->position, z_axis);
    if (z_dot != 1.0) { // if z_dot == 1, then the normalization will fail
        transform = glm::rotate(transform, std::acos(z_dot), glm::normalize(glm::cross(z_axis, position)));
    }

    // Modify mesh data directly (to avoid unnecessary copies)
    mesh.positions.buff.reserve(vertices.size());
    for (const auto& vertex: vertices) {
        glm::vec4 base_point(scale * vertex.x, scale * vertex.y, std::cos(this->radius), 1.0);
        glm::vec3 point = glm::normalize(glm::vec3(transform * base_point));
        mesh.positions.buff.emplace_back(point);
    }
    mesh.colors.buff = std::vector<glm::vec4>(vertices.size(), color);
    mesh.indices.indices.reserve(curr_triangles->size());
    while (!curr_triangles->empty()) {
        mesh.indices.indices.emplace_back(curr_triangles->front());
        curr_triangles->pop();
    }
    mesh.model = glm::mat4x4(1.05f);
    mesh.update_all();
}

void PointMesh::draw(const Camera &camera) const {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);
    if (!camera.bind(program, mesh.model)) return;

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, 3*mesh.indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}