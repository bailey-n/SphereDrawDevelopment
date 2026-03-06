//
// Created by Nathan on 2/28/2026.
//

#include "cubeface_mesh.h"
#include "shader_manager.h"
#include "shapes.h"
#include "common_glm_operations.h"
#include "drawn_mesh.h"

CubeFaceMesh::CubeFaceMesh(CubeFaceNum face):
VAO(gen_vao_and_bind()), frame_buffer(-1),
positions({}), uvs({}), indices({}),
texture(), model(1.0f), program(-1), face(face)
/*_test_mesh()*/ {
    glBindVertexArray(VAO);
    program = shaderManager::get_program({
        "texturedSphereQuad.vert", "texturedSphereQuad.frag", "texturedSphereQuad.tesc", "texturedSphereQuad.tese"
    });
    // Generate texture
    auto data = rgba_white_square_1024();
    texture.regenerate_buffer(WIDTH, HEIGHT, data);
    // std::cout << texture.tex << std::endl;

    // Generate framebuffer
    frame_buffer = gen_fb_and_bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;
    glViewport(0, 0, WIDTH, HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto d_mesh = get_cube_face_mesh_data(face);

    positions.re_buffer_data({d_mesh.vertices[0], d_mesh.vertices[1], d_mesh.vertices[2], d_mesh.vertices[3]});
    uvs.re_buffer_data({d_mesh.uvs[0], d_mesh.uvs[1], d_mesh.uvs[2], d_mesh.uvs[3]});
    indices.re_buffer_data({d_mesh.indices[0], d_mesh.indices[1]});

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

CubeFaceMesh::~CubeFaceMesh() {
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &VAO);
}

void CubeFaceMesh::update_texture(const std::optional<ReferenceTextureMesh>& reference_texture, const std::vector<DrawnMesh*>& texture_meshes) const {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // GLint textureId;
    // glGetFramebufferAttachmentParameteriv(
    //     GL_FRAMEBUFFER,
    //     GL_COLOR_ATTACHMENT0, // Or other attachment point
    //     GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
    //     &textureId
    // );
    // std::cout << textureId << std::endl;

    glViewport(0, 0, texture.width, texture.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (reference_texture.has_value()) {
        reference_texture->draw_texture();
    }
    for (auto mesh: texture_meshes) {
        mesh->draw_texture();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeFaceMesh::draw(const Camera &camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    glDrawElements(GL_PATCHES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

