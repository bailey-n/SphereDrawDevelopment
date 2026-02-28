//
// Created by Nathan on 2/27/2026.
//

#include "line_mesh.h"
#include "shader_manager.h"
#include "cubemap_texture_util.h"
#include "common_glm_operations.h"
#include <cmath>
#include <queue>
#include <utility>
#include <iostream>

std::vector<ConnectorQuad> connect_vertices(const glm::vec3& left, const glm::vec3& right, float width, double subdiv_angle_threshold) {
    if (glm::dot(left, right) > 0.9999) return {};
    double total_angle = std::acos(glm::dot(left, right));
    int subdivs = (int)std::ceil(total_angle / subdiv_angle_threshold) - 1;
    auto subdiv_angle = (float)(total_angle / (subdivs+1));

    float tan_w = std::tan(width);
    glm::vec3 rot_axis = glm::normalize(glm::cross(left, right));
    glm::vec3 offset = tan_w * rot_axis; // TODO: Make inclusive of angle PI/2. May break at ~PI/2 otherwise. Use multiple-cross-product instead.
    glm::vec4 top_left = glm::vec4(glm::normalize(left + offset), 1.0);
    glm::vec4 bottom_left = glm::vec4(glm::normalize(left - offset), 1.0);
    glm::vec4 top_end = glm::normalize(right + offset);
    glm::vec4 bottom_end = glm::normalize(right - offset);

    auto rotation = glm::rotate(glm::mat4x4(1.0f), subdiv_angle, rot_axis);
    std::vector<ConnectorQuad> output;
    output.reserve(subdivs+1);
    for (int i = 0; i < subdivs; i++) {
        auto top_right = glm::vec3(rotation*glm::vec4(top_left, 1.0f));
        auto
        output.emplace_back(top_left, bottom_left)
    }
};

LineMesh::LineMesh(const std::vector<LineBuilderVertexInfo> &build_info, const glm::vec4& color, float width) :
mesh(), color(color), width(std::min(width, (float)M_PI)) {
    program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});
    if (build_info.empty()) return;

    // Build rectangles

}

void LineMesh::draw(const Camera &camera) const {
    if (mesh.indices.size() == 0) return;
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);
    if (!camera.bind(program, mesh.model)) return;
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, 3*mesh.indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}