#include "sphere_mesh.h"
#include <iostream>
#include "cubemap_texture_util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh_util.h"

void SphereMesh::load_texture() {
    // Load texture
    unsigned char* tex_data = stbi_load(texture_path.c_str(), &texture_width, &texture_height, &texture_channels, 0);
    if (!tex_data) {
        std::cout << "Failed to load texture data" << std::endl;
        throw std::exception();
    }
    // Extend edges by 1 pixel to cover up lines formed from floating point imprecision
    extend_cube_map_edges(tex_data, texture_channels, texture_width, texture_height);
    switch(texture_channels) {
        case 3: Texture = gen_texture_2d_rgb(texture_width, texture_height, tex_data); break;
        case 4: Texture = gen_texture_2d_rgba(texture_width, texture_height, tex_data); break;
        default:
            std::cout << "Unrecognized texture format" << std::endl;
            throw std::exception();
    }
    stbi_image_free(tex_data);
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

    glBindTexture(GL_TEXTURE_2D, Texture);
    glBindVertexArray(VAO);
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