#ifndef SPHERE_MESH_H
#define SPHERE_MESH_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#include <string>
#include "vertex_attribute.h"
#include "element_buffer.h"

struct SphereMeshData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::u32vec3> indices;
};

class SphereMesh {
    GLuint VAO;

    std::string texture_path;
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;
    GLuint Texture;

    vertexAttribute<0, glm::vec3> positions;
    vertexAttribute<1, glm::vec3> colors;
    vertexAttribute<2, glm::vec3> normals;
    vertexAttribute<3, glm::vec2> uvs;
    elementBuffer indices;

    GLuint program;
    glm::mat4 model;

    void load_texture();
    void re_buffer();
    void re_buffer_data(const SphereMeshData& data);

public:
    SphereMesh(std::string texture, const SphereMeshData& data);
    SphereMesh(std::string texture, const SphereMeshData& data, const glm::mat4& model);
    ~SphereMesh();
    SphereMesh& operator=(const SphereMesh& mesh) = default;
    void draw(const Camera& camera, float texture_opacity) const;

    void set_model(const glm::mat4& new_model);
    void set_program(GLuint program);

    void set_array_data(const SphereMeshData& data);
};


#endif //FANTASYPLATES_SPHERE_MESH_H
