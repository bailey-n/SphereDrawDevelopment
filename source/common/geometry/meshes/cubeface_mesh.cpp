//
// Created by Nathan on 2/28/2026.
//

#include "cubeface_mesh.h"
#include "shader_manager.h"
#include "shapes.h"
#include "common_glm_operations.h"
#include "line_mesh.h"

CubeFaceMesh::CubeFaceMesh():
VAO(gen_vao_and_bind()), frame_buffer(-1),
positions({}), uvs({}), indices({}),
texture(), model(1.0f), program(-1),
_test_mesh() {
    glBindVertexArray(VAO);
    program = shaderManager::get_program({
        "texturedSphereQuad.vert", "texturedSphereQuad.frag", "texturedSphereQuad.tesc", "texturedSphereQuad.tese"
    });
    // Generate texture
    auto data = rgba_white_square_1024();
    texture.regenerate_buffer(WIDTH, HEIGHT, data);
    std::cout << texture.tex << std::endl;

    // Generate framebuffer
    frame_buffer = gen_fb_and_bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;
    glViewport(0, 0, WIDTH, HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Mesh generation
    auto mesh_data = generate_cuboid_face_mesh_data(2);

    std::vector<glm::vec3> _positions;
    for (int i = 0; i < 4; i++) { _positions.emplace_back(mesh_data.positions.at(i)); }
    std::vector<glm::vec2> _uvs = {
        {1.0, 1.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {0.0, 0.0}
    };
    std::vector<glm::u32vec3> _indices;
    for (int i = 0; i < 2; i++) { _indices.emplace_back(mesh_data.indices.at(i)); }

    positions.re_buffer_data(_positions);
    uvs.re_buffer_data(_uvs);

    indices.re_buffer_data(_indices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(_test_mesh.VAO);
    std::vector<glm::vec3> _test_positions = {
        {0.4f, 0.4f, 0.0f},
        {-0.6f, 0.4f, 0.0f},
        {0.4f, -0.6f, 0.0f}
    };
    std::vector<glm::vec4> _test_colors = {
        {1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f}
    };
    std::vector<glm::u32vec3> _test_indices = {
        {2, 1, 0}
    };
    _test_mesh.set_positions(_test_positions);
    _test_mesh.set_colors(_test_colors);
    _test_mesh.set_indices(_test_indices);
    glBindVertexArray(0);
}

CubeFaceMesh::~CubeFaceMesh() {
    // std::cout << "Running destructor" << std::endl;
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &VAO);
}

void CubeFaceMesh::_test_render() {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    GLint textureId;
    glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0, // Or other attachment point
        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
        &textureId
    );
    std::cout << textureId << std::endl;

    glViewport(0, 0, texture.width, texture.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _test_mesh.draw_texture();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    glDrawElements(GL_PATCHES, indices.count(),GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // _test_mesh._ref_draw(camera);
}

