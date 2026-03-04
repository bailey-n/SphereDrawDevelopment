//
// Created by Nathan on 3/3/2026.
//

#include "drawn_mesh.h"
#include "shader_manager.h"
#include <queue>

glm::mat4x4 DrawnMesh::MVP = (
    glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f) *
    glm::lookAt(glm::vec3{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f})
);

DrawnMesh::DrawnMesh(
    CubeFaceNum face, const std::vector<glm::vec3>& verts,
    const std::vector<glm::vec4>& cols, const std::vector<glm::u32vec3>& idxs
    ) : VAO(gen_vao_and_bind()), vertices(verts), colors(cols), indices({}), model(1.0f), face(face) {

    std::vector<glm::u32vec3>& _indices = indices.indices;
    std::vector<glm::vec3>& _vertices = vertices.buff;

    // Get normal to plane which points towards face
    glm::vec3 plane_normal;
    switch (face) {
        case North: plane_normal = {0.0f, 1.0f, 0.0f}; break;
        case West: plane_normal = {0.0f, 0.0f, 1.0f}; break;
        case Meridian: plane_normal = {1.0f, 0.0f, 0.0f}; break;
        case East: plane_normal = {0.0f, 0.0f, -1.0f}; break;
        case AntiMeridian: plane_normal = {-1.0f, 0.0f, 0.0f}; break;
        case South: plane_normal = {0.0f, -1.0f, 0.0f}; break;
    }

    // Disregard triangles are not mappable to the face (and, because of max arc size guarantees, are not renderable).
    _indices.reserve(idxs.size());
    for (const auto& triangle: idxs) {
        const auto& a = _vertices[triangle.x];
        const auto& b = _vertices[triangle.y];
        const auto& c = _vertices[triangle.z];

        // Test if any of the vertices cannot be validly mapped onto the cubeface
        bool a_below = glm::dot(a, plane_normal) <= 0.0f;
        bool b_below = glm::dot(b, plane_normal) <= 0.0f;
        bool c_below = glm::dot(c, plane_normal) <= 0.0f;

        // NOTE: We can simply disregard any triangles which have a single vertex more than 90 degrees from the center of the face.
        // Why? Because we guarantee that all arcs are less than ~ 35.26 degrees; so if a single point is below the plane,
        // we can guarantee that *none* of the vertices or arcs between them can reach any part of the face which will be rendered.
        if (a_below || b_below || c_below) continue;

        _indices.emplace_back(triangle);
    }

    // Map vertices to face uv
    for (auto& vertex: _vertices) {
        vertex = to_face_uv_vec3(vertex, face);
    }

    // Reset buffers
    vertices.re_buffer();
    colors.re_buffer();
    indices.re_buffer();

    glBindVertexArray(0);
}

DrawnMesh::~DrawnMesh() {
    glDeleteVertexArrays(1, &VAO);
}

void DrawnMesh::draw_texture() const {
    auto program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    const GLint mvpID = glGetUniformLocation(program, "MVP");
    if (mvpID != -1) {
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(MVP));
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

