//
// Created by Nathan on 2/28/2026.
//

#include "cubeface_mesh.h"
#include "shader_manager.h"
#include "shapes.h"
#include "common_glm_operations.h"

CubeFaceMesh::CubeFaceMesh():
VAO(-1), frame_buffer(-1),
positions({}), uvs({}), indices({}),
texture(), model(1.0f), program(-1) {
    program = shaderManager::get_program({
        "texturedSphereQuad.vert", "texturedSphereQuad.frag", "texturedSphereQuad.tesc", "texturedSphereQuad.tese"
    });
    // Generate texture
    auto data = rgba_white_square_1024();
    texture.regenerate_buffer(WIDTH, HEIGHT, data);
    // Generate framebuffer
    frame_buffer = gen_fb_and_bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;
    glViewport(0, 0, WIDTH, HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Mesh generation
    VAO = gen_vao_and_bind();
    auto mesh_data = generate_cuboid_face_mesh_data(2);
    positions.buff = mesh_data.positions;
    uvs.buff = mesh_data.uvs;
    indices.indices = mesh_data.indices;

    positions.re_buffer();
    uvs.re_buffer();
    indices.re_buffer();
}

CubeFaceMesh::~CubeFaceMesh() {
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &VAO);
}

void CubeFaceMesh::draw(const Camera &camera) {
    if (program == static_cast<GLuint>(-1)) return;
    glUseProgram(program);

    // glBindTexture(GL_TEXTURE_2D, texture.tex);
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());

    if (!camera.bind(program, model)) return;

    const GLint tessellation_control_id = glGetUniformLocation(program, "inv_min_arc_dist");
    if (tessellation_control_id == -1) return;
    glUniform1f(tessellation_control_id, 1.0f / MAX_SUBDIV_WIDTH);

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.tex);
    const GLint texture_sampler_id = glGetUniformLocation(program, "textureSampler");
    if (texture_sampler_id == -1) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        return;
    }
    glUniform1f(texture_sampler_id, 0);
    glDrawElements(GL_PATCHES,indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

