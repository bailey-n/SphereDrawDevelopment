//
// Created by Nathan on 2/28/2026.
//

#ifndef SPHEREDRAW_TEXTURE_BUFFER_H
#define SPHEREDRAW_TEXTURE_BUFFER_H

#include "opengl_include.h"
#include <cstdint>
#include <vector>

struct textureBuffer {
    uint32_t width;
    uint32_t height;
    std::vector<glm::u8vec4> pixels;
    GLuint tex;

    textureBuffer();
    textureBuffer(uint32_t width, uint32_t height);
    ~textureBuffer();

    void regenerate_buffer(uint32_t width, uint32_t height, const std::vector<glm::u8vec4>& data);
    void resize(uint32_t width, uint32_t height);
    void set_uniform_color(const glm::u8vec4& color_rgb);
};


#endif //SPHEREDRAW_TEXTURE_BUFFER_H
