//
// Created by Nathan on 2/28/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_MESH_H
#define SPHEREDRAW_CUBEFACE_MESH_H

#include "opengl_include.h"
#include "texture_buffer.h"
#include "element_buffer.h"
#include "vertex_attribute.h"
#include "drawn_mesh.h"
#include "reference_texture_mesh.h"
#include <optional>

class CubeFaceMesh {
    static constexpr int WIDTH = 1024;
    static constexpr int HEIGHT = 1024;
    static constexpr double MAX_SUBDIV_WIDTH = 0.01; // Radians

    GLuint VAO;
    GLuint frame_buffer;
    // GLuint render_buffer;
    vertexAttribute<0, glm::vec3> positions;
    vertexAttribute<1, glm::vec2> uvs;
    elementBuffer indices;
    textureBuffer texture;
    maskBuffer mask;
    glm::mat4x4 model;
    GLuint program;

    bool has_selected_texture = false;

    // BasicMesh _test_mesh;
    CubeFaceNum face;

public:
    explicit CubeFaceMesh(CubeFaceNum face);
    ~CubeFaceMesh();
    void update_texture(
        const std::optional<ReferenceTextureMesh>& reference_texture,
        const std::vector<DrawnMesh*>& texture_meshes,
        const std::optional<DrawnMesh*>& selected_mesh
    );
    void draw(const Camera& camera);

    std::vector<glm::u8vec4>& get_pixel_buffer();
};


#endif //SPHEREDRAW_CUBEFACE_MESH_H
