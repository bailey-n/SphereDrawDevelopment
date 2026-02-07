#ifndef ELEMENT_BUFFER_H
#define ELEMENT_BUFFER_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "mesh_util.h"

struct elementBuffer {
    std::vector<glm::u32vec3> indices;
    GLuint ebo;

    elementBuffer(const std::vector<glm::u32vec3>& indices_);
    ~elementBuffer();
    void re_buffer();
    void re_buffer_data(const std::vector<glm::u32vec3>& indices_);

    [[nodiscard]] long long size() const;
    [[nodiscard]] void* data();
    [[nodiscard]] const void* data() const;
    [[nodiscard]] int count() const;
};


#endif //ELEMENT_BUFFER_H
