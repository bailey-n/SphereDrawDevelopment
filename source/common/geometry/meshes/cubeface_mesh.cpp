//
// Created by Nathan on 2/28/2026.
//

#include "cubeface_mesh.h"
#include "shader_manager.h"
#include "shapes.h"
#include "common_glm_operations.h"
#include "drawn_mesh.h"
#include "mesh_util.h"

CubeFaceMesh::CubeFaceMesh(CubeFaceNum face):
VAO(gen_vao_and_bind()), frame_buffer(-1),
positions({}), uvs({}), indices({}),
texture(), mask(), model(1.0f), program(-1), face(face)
/*_test_mesh()*/ {
    glBindVertexArray(VAO);
    program = shaderManager::get_program({
        "texturedSphereQuad.vert", "texturedSphereQuad.frag", "texturedSphereQuad.tesc", "texturedSphereQuad.tese"
    });
    // Generate texture
    auto tex_data = rgba_white_square_1024();
    texture.regenerate_buffer(WIDTH, HEIGHT, tex_data);

    auto mask_data = r_black_square_1024();
    mask.regenerate_buffer(WIDTH, HEIGHT, mask_data);
    // std::cout << texture.tex << std::endl;

    // Generate framebuffer
    frame_buffer = gen_fb_and_bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mask.mask_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Failed to build cubeface framebuffer" << std::endl;
        return;
    };
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
    // std::cout << "Deleted CubeFaceMesh" << std::endl;
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &VAO);
}

void CubeFaceMesh::update_texture(
    const std::optional<ReferenceTextureMesh>& reference_texture,
    const std::vector<DrawnMesh*>& texture_meshes,
    const std::optional<DrawnMesh*>& selected_mesh
) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glViewport(0, 0, texture.width, texture.height);
    GLenum selectBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, selectBuffers);
    glClear(GL_COLOR_BUFFER_BIT);

    // std::cout << "Drawing reference texture" << std::endl;
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    if (reference_texture.has_value()) {
        reference_texture->draw_texture();
    }
    for (auto mesh: texture_meshes) {
        // std::cout << "Drawing mesh" << std::endl;
        mesh->draw_texture();
    }
    if (selected_mesh.has_value()) {
        glDrawBuffers(2, selectBuffers);
        (*selected_mesh)->draw_texture(true);
        has_selected_texture = true;
    }
    else { has_selected_texture = false; }

    // GLint textureId;
    // GLint textureType;
    // glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureId);
    // glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &textureType);
    // std::cout << "Attached texture: " << textureId << ", type " << textureType << ". Expected: " << texture.tex << std::endl;
    //
    // GLint maskId;
    // GLint maskType;
    // glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &maskId);
    // glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &maskType);
    // std::cout << "Attached mask: " << maskId << ", type " << maskType << ". Expected: " << mask.mask_tex << std::endl;

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
    glUniform1i(texture_sampler_id, 0);

    // GLint tex0;
    // glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex0);
    // std::cout << "Current bound texture0: " << tex0 << ". Expected " << texture.tex << std::endl;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mask.mask_tex);
    const GLint mask_sampler_id = glGetUniformLocation(program, "maskSampler");
    if (mask_sampler_id == -1) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        return;
    }
    glUniform1i(mask_sampler_id, 1);

    // GLint tex1;
    // glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex1);
    // std::cout << "Current bound texture1: " << tex1 << ". Expected " << mask.mask_tex << std::endl;

    const GLint texel_size_id = glGetUniformLocation(program, "texelSize");
    const GLint border_color_id = glGetUniformLocation(program, "borderColor");
    const GLint border_width_id = glGetUniformLocation(program, "borderWidth");
    const GLint draw_highlight_id = glGetUniformLocation(program, "drawHighlight");

    if (!(texel_size_id == -1 || border_color_id == -1 || border_width_id == -1)) {
        glUniform2f(texel_size_id, 1.0f / (float)texture.width, 1.0f / (float)texture.height);
        glUniform4f(border_color_id, 1.0f, 0.8f, 0.8f, 1.0f);
        glUniform1i(border_width_id, 2);

        // glBindTexture(GL_TEXTURE_2D, 0);
        // glBindVertexArray(0);
        // return;
    }

    if (draw_highlight_id != -1) {
        glUniform1i(draw_highlight_id, has_selected_texture);
    }

    glDrawElements(GL_PATCHES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

std::vector<glm::u8vec4>& CubeFaceMesh::get_pixel_buffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return texture.pixels;
}

