//
// Created by Nathan on 2/27/2026.
//

#include "line_mesh.h"
#include "shader_manager.h"
#include "cubemap_texture_util.h"
#include "common_glm_operations.h"
#include "vertex_manipulation.h"
#include <cmath>
#include <queue>
#include <utility>
#include <iostream>

using Edge = std::pair<uint32_t, uint32_t>;
using Triangle = glm::u32vec3;

ConnectorQuad::ConnectorQuad(const glm::vec4& tl, const glm::vec4& bl, const glm::vec4& br, const glm::vec4& tr) :
top_left(tl), bottom_left(bl), bottom_right(br), top_right(tr) {}

std::vector<glm::vec3> ConnectorQuad::subdivide_left(double max_subdiv_angle) const {
    return subdivide_line(top_left, bottom_left, max_subdiv_angle);
}

std::vector<glm::vec3> ConnectorQuad::subdivide_right(double max_subdiv_angle) const {
    return subdivide_line(top_right, bottom_right, max_subdiv_angle);
}



std::vector<ConnectorQuad> connect_vertices(const glm::vec3& left, const glm::vec3& right, float width, double subdiv_angle_threshold) {
    // if (glm::dot(left, right) > 0.9999) return {};
    double total_angle = std::acos(glm::dot(left, right));
    int subdivs = (int)std::ceil(total_angle / subdiv_angle_threshold) - 1;
    auto subdiv_angle = (float)(total_angle / (subdivs+1));

    // Get initial rotation, left side of rectangle, right side of rectangle
    float tan_w = std::tan(width);
    glm::vec3 rot_axis = glm::normalize(glm::cross(left, right));
    glm::vec3 offset = tan_w * rot_axis; // TODO: Make inclusive of angle PI/2. May break at ~PI/2 otherwise. Use multiple-cross-product instead.
    glm::vec4 top_left = glm::vec4((glm::normalize(left + offset)), 1.0f);
    glm::vec4 bottom_left = glm::vec4((glm::normalize(left - offset)), 1.0f);
    glm::vec4 top_end = glm::vec4((glm::normalize(right + offset)), 1.0f);
    glm::vec4 bottom_end = glm::vec4((glm::normalize(right - offset)), 1.0f);

    // Rotate until we get all the necessary quads
    std::vector<ConnectorQuad> output;
    auto rotation = glm::rotate(glm::mat4x4(1.0f), subdiv_angle, rot_axis);
    output.reserve(subdivs+1);
    for (int i = 0; i < subdivs; i++) {
        auto top_right = rotation * top_left;
        auto bottom_right = rotation * bottom_left;
        output.emplace_back(top_left, bottom_left, bottom_right, top_right);
        top_left = top_right;
        bottom_left = bottom_right;
    }
    output.emplace_back(top_left, bottom_left, bottom_end, top_end); // last quad

    return output;
};

LineMesh::LineMesh(const std::vector<LineBuilderVertexInfo> &build_info, const glm::vec4& color, float width) :
mesh(), color(color), width(std::min(width, (float)M_PI)) {
    program = shaderManager::get_program({"lineVertexShader.vert", "lineFragmentShader.frag", "lineTesselationControlShader.tesc", "lineTesselationEvaluationShader.tese"});
    if (build_info.empty()) return;
    if (build_info.size() == 1) {
        make_sphere_circle(build_info[0].pos, width, mesh.positions.buff, mesh.indices.indices);
        mesh.colors.buff = std::vector(mesh.positions.buff.size(), color);
        mesh.model = glm::mat4x4(1.05f);
        mesh.update_all();
        return;
    }

    // Build connecting rectangles
    for (int i = 0; i < build_info.size()-1; i++) {
        auto rectangle_quads = connect_vertices(build_info[i].pos, build_info[i+1].pos, width, MAX_SUBDIV_WIDTH);
        auto top_left = rectangle_quads[0].top_left;
        auto bottom_left = rectangle_quads[0].top_right;

        for (const auto& quad: rectangle_quads) {
            size_t start_idx = mesh.positions.buff.size();
            mesh.positions.buff.emplace_back(top_left);
            mesh.positions.buff.emplace_back(bottom_left);
            mesh.positions.buff.emplace_back(quad.bottom_right);
            mesh.positions.buff.emplace_back(quad.top_right);

            mesh.indices.indices.emplace_back(start_idx, start_idx+1, start_idx+2);
            mesh.indices.indices.emplace_back(start_idx, start_idx+2, start_idx+3);
        }
    }
    mesh.colors.buff = std::vector(mesh.positions.buff.size(), color);
    mesh.update_all();
}

void LineMesh::draw(const Camera &camera) const {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);
    if (!camera.bind(program, mesh.model)) return;

    const GLint tessellation_control_id = glGetUniformLocation(program, "inv_min_arc_dist");
    if (tessellation_control_id == -1) return;
    glUniform1f(tessellation_control_id, 1.0f / MAX_SUBDIV_WIDTH);

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_PATCHES, mesh.indices.count(),GL_UNSIGNED_INT,nullptr);
    glBindVertexArray(0);
}