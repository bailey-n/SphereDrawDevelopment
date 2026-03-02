//
// Created by Nathan on 2/14/2026.
//

#include "basic_mesh.h"
#include "shader_manager.h"

BasicMesh::BasicMesh() : VAO(gen_vao_and_bind()), positions({}), colors({}), indices({}), model(glm::mat4(1.0f)) {
    glBindVertexArray(0);
}

BasicMesh::BasicMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors, const std::vector<glm::u32vec3>& indices) :
        VAO(gen_vao_and_bind()), positions(positions), colors(colors), indices(indices), model(glm::mat4(1.0f)) {
    glBindVertexArray(0);
}

BasicMesh::~BasicMesh() {
    // std::cout << "Running destructor" << std::endl;
    glDeleteVertexArrays(1, &VAO);
}

void BasicMesh::set_positions(const std::vector<glm::vec3> &new_positions) {
    glBindVertexArray(VAO);

    positions.re_buffer_data(new_positions);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BasicMesh::set_colors(const std::vector<glm::vec4> &new_colors) {
    glBindVertexArray(VAO);
    colors.re_buffer_data(new_colors);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BasicMesh::set_indices(const std::vector<glm::u32vec3> &new_indices) {
    glBindVertexArray(VAO);
    indices.re_buffer_data(new_indices);
    glBindVertexArray(0);
}

void BasicMesh::new_data(const std::vector<glm::vec3>& new_positions, const std::vector<glm::vec4>& new_colors, const std::vector<glm::u32vec3>& new_indices) {
    glBindVertexArray(VAO);
    positions.re_buffer_data(new_positions);
    colors.re_buffer_data(new_colors);
    indices.re_buffer_data(new_indices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BasicMesh::update_all() {
    glBindVertexArray(VAO);

    positions.re_buffer();
    colors.re_buffer();
    indices.re_buffer();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool BasicMesh::renderable() const {
    return indices.size() >= 3;
}

void BasicMesh::_ref_draw(const Camera& camera) const {
    auto program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);
    if (!camera.bind(program, model)) return;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void BasicMesh::draw_texture() const {
    auto program = shaderManager::get_program({"pointVertexShader.glsl", "pointFragmentShader.glsl"});
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    const GLint mvpID = glGetUniformLocation(program, "MVP");
    if (mvpID != -1) {
        glm::mat4 MVP = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f) *
            glm::lookAt(glm::vec3{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(MVP));
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}