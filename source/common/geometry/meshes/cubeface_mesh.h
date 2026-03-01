//
// Created by Nathan on 2/28/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_MESH_H
#define SPHEREDRAW_CUBEFACE_MESH_H

#include "texture_buffer.h"
#include "element_buffer.h"
#include "vertex_attribute.h"

class CubeFaceMesh {
    static constexpr int WIDTH = 1024;
    static constexpr int HEIGHT = 1024;
    static constexpr double MAX_SUBDIV_WIDTH = 0.01; // Radians

    GLuint VAO;
    GLuint frame_buffer;
    vertexAttribute<0, glm::vec3> positions;
    vertexAttribute<1, glm::vec2> uvs;
    elementBuffer indices;
    textureBuffer texture;
    glm::mat4x4 model;
    GLuint program;

public:
    CubeFaceMesh();
    ~CubeFaceMesh();
    void draw(const Camera& camera);
};


#endif //SPHEREDRAW_CUBEFACE_MESH_H
