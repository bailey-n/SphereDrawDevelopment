#include "sphere_mesh.h"
#include <iostream>
#include "cubemap_texture_util.h"
#include "mesh_util.h"

void SphereMesh::load_texture() {
    load_cubemap_texture(texture_path, Texture, texture_width, texture_height, texture_channels);
}

SphereMesh::SphereMesh(std::string texture, const SphereMeshData& data) :
VAO(gen_vao_and_bind()),
texture_path(std::move(texture)), Texture(-1),
positions(data.positions),
colors(data.colors),
normals(data.normals),
uvs(data.uvs),
indices(data.indices),
program(-1),
model(glm::mat4(1.0)) {
    load_texture();
    glBindVertexArray(0);
}

SphereMesh::SphereMesh(std::string texture, const SphereMeshData& data, const glm::mat4 &model) :
VAO(gen_vao_and_bind()),
texture_path(std::move(texture)), Texture(-1),
positions(data.positions),
colors(data.colors),
normals(data.normals),
uvs(data.uvs),
indices(data.indices),
program(-1),
model(model) {
    load_texture();
    glBindVertexArray(0);
}

SphereMesh::~SphereMesh() {
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VAO);
}

void SphereMesh::re_buffer() {
    glBindVertexArray(VAO);

    positions.re_buffer();
    colors.re_buffer();
    normals.re_buffer();
    uvs.re_buffer();
    indices.re_buffer();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SphereMesh::re_buffer_data(const SphereMeshData& data) {
    glBindVertexArray(VAO);

    positions.re_buffer_data(data.positions);
    colors.re_buffer_data(data.colors);
    normals.re_buffer_data(data.normals);
    uvs.re_buffer_data(data.uvs);
    indices.re_buffer_data(data.indices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SphereMesh::draw(const Camera& camera, float texture_opacity) const {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    if (!camera.bind(program, model)) return;

    const GLint tex_opacity_id = glGetUniformLocation(program, "textureOpacity");
    if (tex_opacity_id == -1) return;
    glUniform1f(tex_opacity_id, texture_opacity);

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    const GLint texture_sampler_id = glGetUniformLocation(program, "textureSampler");
    if (texture_sampler_id == -1) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        return;
    }
    glUniform1f(texture_sampler_id, 0);

    glDrawElements(GL_TRIANGLES,indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void SphereMesh::set_model(const glm::mat4 &new_model) {
    this->model = new_model;
}

void SphereMesh::set_program(const GLuint new_program) {
    this->program = new_program;
}

void SphereMesh::set_array_data(const SphereMeshData& data) {
    re_buffer_data(data);
}