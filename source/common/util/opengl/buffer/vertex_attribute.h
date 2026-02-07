#ifndef VERTEX_ATTRIBUTE_H
#define VERTEX_ATTRIBUTE_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "mesh_util.h"

template <size_t Location, typename T>
struct vertexAttribute {
    std::vector<T> buff;
    GLuint vbo;

    explicit vertexAttribute(std::vector<T> data) : buff(std::move(data)),
    vbo(gen_float_vbo<T>(Location, buff.size(), buff.data())) {}

    ~vertexAttribute() { glDeleteBuffers(1, &vbo); }

    void re_buffer() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size() * (long long)sizeof(T), data(), GL_DYNAMIC_DRAW);
    }

    void re_buffer_data(const std::vector<T>& new_buff) {
        buff = new_buff;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size() * (long long)sizeof(T), data(), GL_DYNAMIC_DRAW);
    }

    [[nodiscard]] long long size() const { return buff.size(); }
    [[nodiscard]] void* data() { return buff.data(); }
    [[nodiscard]] int count() { return size() * sizeof(T) / sizeof(float); }
};

#endif //VERTEX_ATTRIBUTE_H
