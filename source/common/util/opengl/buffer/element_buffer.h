#ifndef ELEMENT_BUFFER_H
#define ELEMENT_BUFFER_H

#include "opengl_include.h"
#include "mesh_util.h"
#include <vector>
#include <string>

template <typename T>
struct elementBufferT {
    std::vector<T> indices;
    GLuint ebo;

    explicit elementBufferT(const std::vector<T>& indices_) :
        indices(indices_), ebo(gen_ebo<T>(
        (long long)indices.size(),(void*)indices.data())
    ) {}
    ~elementBufferT() { glDeleteBuffers(1, &ebo); }
    void re_buffer() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size() * (long long)sizeof(T), data(), GL_DYNAMIC_DRAW);
    }
    void re_buffer_data(const std::vector<T>& indices_) {
        indices = indices_;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size() * (long long)sizeof(T), data(), GL_DYNAMIC_DRAW);
    }

    [[nodiscard]] long long size() const { return (long long)indices.size(); }
    [[nodiscard]] void* data() { return indices.data(); }
    [[nodiscard]] const void* data() const { return indices.data(); }
    [[nodiscard]] int count() const { return sizeof(T)*size()/sizeof(GLuint); }
};

using elementBuffer = elementBufferT<glm::u32vec3>;
using elementQuadBuffer = elementBufferT<glm::u32vec4>;

#endif //ELEMENT_BUFFER_H
