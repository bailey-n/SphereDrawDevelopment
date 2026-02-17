#include "mesh_util.h"
#include <iostream>

GLuint gen_vao_and_bind() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    if (vao == static_cast<GLuint>(-1)) return vao;
    glBindVertexArray(vao);
    return vao;
}

GLuint gen_texture_2d(GLint fmt, GLsizei width, GLsizei height, void* tex_data) {
    GLuint texture;
    glGenTextures(1, &texture);
    if (texture == static_cast<GLuint>(-1)) return texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    return texture;
}

GLuint gen_texture_2d_rgba(GLsizei width, GLsizei height, void* tex_data) {
    return gen_texture_2d(GL_RGBA, width, height, tex_data);
}

GLuint gen_texture_2d_rgb(GLsizei width, GLsizei height, void* tex_data) {
    return gen_texture_2d(GL_RGB, width, height, tex_data);
}
