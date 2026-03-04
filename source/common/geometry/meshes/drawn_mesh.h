//
// Created by Nathan on 3/3/2026.
//

#ifndef DRAWN_MESH_H
#define DRAWN_MESH_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "camera.h"
#include "cubemap_util.h"
#include "vertex_attribute.h"
#include "element_buffer.h"

struct DrawnMesh {
    GLuint VAO;
    vertexAttribute<0, glm::vec3> vertices;
    vertexAttribute<1, glm::vec4> colors;
    elementBuffer indices;
    glm::mat4x4 model;

    CubeFaceNum face;

    static glm::mat4x4 MVP;
    static glm::mat4x4 view;

    DrawnMesh(CubeFaceNum face, const std::vector<glm::vec3>& sphere_vertices, const std::vector<glm::vec4>& vertex_colors, const std::vector<glm::u32vec3>& indices);
    ~DrawnMesh();
    void draw_texture() const;
};



#endif //DRAWN_MESH_H
