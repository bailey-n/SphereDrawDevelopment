//
// Created by Nathan on 3/3/2026.
//

#include "reference_texture_mesh.h"
#include "shader_manager.h"

glm::mat4x4 ReferenceTextureMesh::MVP = (
    glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f) *
    glm::lookAt(glm::vec3{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f})
);
std::string ReferenceTextureMesh::reference_texture_path = "textures/Earth_cube_map.png";

#include <iostream>
#include "cubemap_texture_util.h"
#include "mesh_util.h"

void ReferenceTextureMesh::load_texture() {
    load_cubemap_texture(texture_path, tex, texture_width, texture_height, texture_channels);
}

ReferenceTextureMesh::ReferenceTextureMesh(CubeFaceNum face) :
texture_path(reference_texture_path), VAO(gen_vao_and_bind()), tex(-1),
positions({}), uvs({}), indices({}),
program(-1) {
    load_texture();
    program = shaderManager::get_program({"importedCubemap.vert", "importedCubemap.frag"});
    auto m_data = get_full_cubemap_texture_mesh_data(face);
    positions.re_buffer_data({m_data.vertices[0], m_data.vertices[1], m_data.vertices[2], m_data.vertices[3]});
    uvs.re_buffer_data({m_data.uvs[1], m_data.uvs[0], m_data.uvs[3], m_data.uvs[2]}); // Got them wrong in the function so we undo that mistake here
    indices.re_buffer_data({m_data.indices[0], m_data.indices[1]});
    glBindVertexArray(0);
}

ReferenceTextureMesh::~ReferenceTextureMesh() {
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &VAO);
}

void ReferenceTextureMesh::draw_texture() const {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    const GLint mvpID = glGetUniformLocation(program, "MVP");
    if (mvpID != -1) {
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(MVP));
    }

    const GLint tex_opacity_id = glGetUniformLocation(program, "textureOpacity");
    if (tex_opacity_id == -1) return;
    glUniform1f(tex_opacity_id, TEXTURE_OPACITY);

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

    glDrawElements(GL_TRIANGLES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}