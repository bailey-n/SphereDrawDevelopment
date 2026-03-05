//
// Created by Nathan on 2/28/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_MESH_H
#define SPHEREDRAW_CUBEFACE_MESH_H

#include "texture_buffer.h"
#include "element_buffer.h"
#include "vertex_attribute.h"
#include "basic_mesh.h"
#include "drawn_mesh.h"
#include <optional>
#include "reference_texture_mesh.h"

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
    glm::mat4x4 model;
    GLuint program;

    // BasicMesh _test_mesh;
    CubeFaceNum face;

public:
    CubeFaceMesh(CubeFaceNum face);
    ~CubeFaceMesh();
    void update_texture(const std::optional<ReferenceTextureMesh>& reference_texture, const std::vector<DrawnMesh*>& texture_meshes) const;
    void draw(const Camera& camera);
};


#endif //SPHEREDRAW_CUBEFACE_MESH_H
