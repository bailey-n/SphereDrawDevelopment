//
// Created by Nathan on 3/3/2026.
//

#ifndef REFERENCE_TEXTURE_MESH_H
#define REFERENCE_TEXTURE_MESH_H

#include "opengl_include.h"
#include <vector>
#include "camera.h"
#include <string>
#include "vertex_attribute.h"
#include "element_buffer.h"
#include "cubemap_util.h"

class ReferenceTextureMesh {
    GLuint VAO;

    std::string texture_path;
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;
    GLuint tex;

    vertexAttribute<0, glm::vec3> positions;
    vertexAttribute<1, glm::vec2> uvs;
    elementBuffer indices;

    GLuint program;

    static glm::mat4x4 MVP;
    // static glm::mat4x4 view;
    static std::string default_reference_texture_path;
    static constexpr float TEXTURE_OPACITY = 1.0f;

    void load_texture();

public:
    ReferenceTextureMesh(CubeFaceNum face);
    ReferenceTextureMesh(CubeFaceNum face, std::string texture_path);
    ~ReferenceTextureMesh();

    void draw_texture() const;
};



#endif //REFERENCE_TEXTURE_MESH_H
