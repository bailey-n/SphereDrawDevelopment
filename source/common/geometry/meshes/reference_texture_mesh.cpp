//
// Created by Nathan on 3/3/2026.
//

#include "reference_texture_mesh.h"

#include "cubeface.h"

std::string ReferenceTextureMesh::reference_texture_path = "textures/Earth_cube_map.png";

#include "sphere_mesh.h"
#include <iostream>
#include "cubemap_texture_util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh_util.h"

void ReferenceTextureMesh::load_texture() {
    // Load texture
    unsigned char* tex_data = stbi_load(texture_path.c_str(), &texture_width, &texture_height, &texture_channels, 0);
    if (!tex_data) {
        std::cout << "Failed to load texture data" << std::endl;
        throw std::exception();
    }
    // Extend edges by 1 pixel to cover up lines formed from floating point imprecision
    extend_cube_map_edges(tex_data, texture_channels, texture_width, texture_height);
    switch(texture_channels) {
        case 3: tex = gen_texture_2d_rgb(texture_width, texture_height, tex_data); break;
        case 4: tex = gen_texture_2d_rgba(texture_width, texture_height, tex_data); break;
        default:
            std::cout << "Unrecognized texture format" << std::endl;
            throw std::exception();
    }
    stbi_image_free(tex_data);
}

ReferenceTextureMesh::ReferenceTextureMesh(CubeFaceNum face) :
VAO(gen_vao_and_bind()), tex(-1),
positions(get),
colors(data.colors),
normals(data.normals),
uvs(data.uvs),
indices(data.indices),
program(-1),
model(glm::mat4(1.0)) {
    load_texture();
    glBindVertexArray(0);
}

ReferenceTextureMesh::~ReferenceTextureMesh() {
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &VAO);
}

void ReferenceTextureMesh::draw_texture() const {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    if (!camera.bind(program, model)) return;

    const GLint tex_opacity_id = glGetUniformLocation(program, "textureOpacity");
    if (tex_opacity_id == -1) return;
    glUniform1f(tex_opacity_id, texture_opacity);

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
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