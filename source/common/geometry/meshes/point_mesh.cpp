//
// Created by Nathan on 2/14/2026.
//

#include "point_mesh.h"
#include "shader_manager.h"
#include "cubemap_texture_util.h"
#include "vertex_manipulation.h"
#include <cmath>
#include <queue>
#include <utility>
#include <iostream>

using Edge = std::pair<uint32_t, uint32_t>;
using Triangle = glm::u32vec3;

PointMesh::PointMesh(glm::vec3 position, glm::vec4 color, float radius) :
mesh(), position(position), color(color), radius(std::min(radius, (float)M_PI)) {
    program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});

    make_sphere_circle(position, radius, mesh.positions.buff, mesh.indices.indices);
    mesh.colors.buff = std::vector(mesh.positions.buff.size(), color);
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