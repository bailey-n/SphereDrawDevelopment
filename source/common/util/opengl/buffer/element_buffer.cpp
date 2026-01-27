#include "element_buffer.h"

elementBuffer::elementBuffer(const std::vector<glm::u32vec3> &indices_) :
indices(indices_),
ebo(gen_ebo<glm::u32vec3>(
        (long long)indices.size(),
        (void*)indices.data())
) {}

elementBuffer::~elementBuffer() {
    glDeleteBuffers(1, &ebo);
}

void elementBuffer::re_buffer() {
    glBindBuffer(GL_INDEX_ARRAY, ebo);
    glBufferData(GL_INDEX_ARRAY, size() * (long long)sizeof(glm::u32vec3), data(), GL_DYNAMIC_DRAW);
}

void elementBuffer::re_buffer_data(const std::vector<glm::u32vec3> &indices_) {
    indices = indices_;
    glBindBuffer(GL_INDEX_ARRAY, ebo);
    glBufferData(GL_INDEX_ARRAY, size() * (long long)sizeof(glm::u32vec3), data(), GL_DYNAMIC_DRAW);
}

long long elementBuffer::size() const {
    return (long long)indices.size();
}

void* elementBuffer::data() {
    return indices.data();
}

const void* elementBuffer::data() const{
    return indices.data();
}

int elementBuffer::count() const {
    return 3*size();
}