//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_BASIC_MESH_H
#define SPHEREDRAW_BASIC_MESH_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "camera.h"
#include "vertex_attribute.h"
#include "element_buffer.h"

struct BasicMesh {
    GLuint VAO;
    vertexAttribute<0, glm::vec3> positions;
    vertexAttribute<1, glm::vec4> colors;
    elementBuffer indices;
    glm::mat4x4 model;

    BasicMesh();
    BasicMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors, const std::vector<glm::u32vec3>& indices);
    ~BasicMesh();

    void set_positions(const std::vector<glm::vec3>& new_positions);
    void set_colors(const std::vector<glm::vec4>& new_colors);
    void set_indices(const std::vector<glm::u32vec3>& new_indices);
    void new_data(const std::vector<glm::vec3>& new_positions, const std::vector<glm::vec4>& new_colors, const std::vector<glm::u32vec3>& indices);
    void update_all();
    bool renderable() const;
};


#endif //SPHEREDRAW_BASIC_MESH_H
