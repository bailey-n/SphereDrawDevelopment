#ifndef MESH_UTIL_H
#define MESH_UTIL_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

template <typename T>
GLuint gen_float_vbo(int loc, int v_count, void* data) {
    constexpr int DO_NOT_NORMALIZE = GL_FALSE;
    constexpr int PACKED_DENSELY = 0;
    constexpr void* NO_OFFSET = nullptr;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    if (vbo == static_cast<GLuint>(-1)) return vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v_count * sizeof(T), data, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(loc, sizeof(T) / sizeof(float), GL_FLOAT, DO_NOT_NORMALIZE, PACKED_DENSELY, NO_OFFSET);
    glEnableVertexAttribArray(loc);

    return vbo;
}

template <size_t CCount>
GLuint gen_float_n_vbo(int loc, int v_count, void* data) {
    constexpr int DO_NOT_NORMALIZE = GL_FALSE;
    constexpr int PACKED_DENSELY = 0;
    constexpr void* NO_OFFSET = nullptr;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    if (vbo == static_cast<GLuint>(-1)) return vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v_count * sizeof(float) * CCount, data, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(loc, CCount, GL_FLOAT, DO_NOT_NORMALIZE, PACKED_DENSELY, NO_OFFSET);
    glEnableVertexAttribArray(loc);

    return vbo;
}

GLuint gen_float_1_vbo(int loc, long v_count, void* data);
GLuint gen_float_2_vbo(int loc, long v_count, void* data);
GLuint gen_float_3_vbo(int loc, long v_count, void* data);
GLuint gen_float_4_vbo(int loc, long v_count, void* data);

template <typename T>
GLuint gen_ebo(long long len, void* data) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    if (ebo == static_cast<GLuint>(-1)) return ebo;
    glBindBuffer(GL_INDEX_ARRAY, ebo);
    glBufferData(GL_INDEX_ARRAY, len * sizeof(T), data, GL_DYNAMIC_DRAW);
    return ebo;
}

template <size_t VertexCount>
GLuint gen_ebo_n(long primitive_count, void* data) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    if (ebo == static_cast<GLuint>(-1)) return ebo;
    glBindBuffer(GL_INDEX_ARRAY, ebo);
    glBufferData(GL_INDEX_ARRAY, primitive_count * VertexCount * sizeof(glm::u32), data, GL_DYNAMIC_DRAW);
    return ebo;
}

GLuint gen_vao_and_bind();

GLuint gen_texture_2d(GLint fmt, GLsizei width, GLsizei height, void* tex_data);
GLuint gen_texture_2d_rgba(GLsizei width, GLsizei height, void* tex_data);
GLuint gen_texture_2d_rgb(GLsizei width, GLsizei height, void* tex_data);

#endif MESH_UTIL_H
