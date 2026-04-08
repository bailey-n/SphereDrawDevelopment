//
// Created by Nathan on 2/28/2026.
//

#include "texture_buffer.h"
#include "mesh_util.h"

textureBuffer::textureBuffer() : width(0), height(0), pixels({}), tex(-1) {}

textureBuffer::textureBuffer(uint32_t width, uint32_t height) :
width(width), height(height),
pixels(width * height, {0, 0, 0, 0}),
tex(-1) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

textureBuffer::~textureBuffer() {
    if (tex != static_cast<GLuint>(-1)) glDeleteTextures(1, &tex);
}

void textureBuffer::regenerate_buffer(uint32_t _width, uint32_t _height, const std::vector<glm::u8vec4> &data) {
    if (tex != static_cast<GLuint>(-1)) glDeleteTextures(1, &tex);
    width = _width;
    height = _height;
    pixels = data;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void textureBuffer::resize(uint32_t _width, uint32_t _height) {
    width = _width;
    height = _height;
    pixels = std::vector<glm::u8vec4>(_width * _height, {0, 0, 0, 0});
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void textureBuffer::set_uniform_color(const glm::u8vec4 &color_rgb) {
    std::fill(pixels.begin(), pixels.end(), color_rgb);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

maskBuffer::maskBuffer() : width(0), height(0), pixels({}), mask_tex(-1) {}

maskBuffer::maskBuffer(uint32_t width, uint32_t height) :
width(width), height(height),
pixels(width * height, 0),
mask_tex(-1) {
    glGenTextures(1, &mask_tex);
    glBindTexture(GL_TEXTURE_2D, mask_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

maskBuffer::~maskBuffer() {
    if (mask_tex != static_cast<GLuint>(-1)) glDeleteTextures(1, &mask_tex);
}

void maskBuffer::regenerate_buffer(uint32_t _width, uint32_t _height, const std::vector<glm::u8> &data) {
    if (mask_tex != static_cast<GLuint>(-1)) glDeleteTextures(1, &mask_tex);
    width = _width;
    height = _height;
    pixels = data;
    glGenTextures(1, &mask_tex);
    glBindTexture(GL_TEXTURE_2D, mask_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void maskBuffer::resize(uint32_t _width, uint32_t _height) {
    width = _width;
    height = _height;
    pixels = std::vector<glm::u8>(_width * _height, 0);
    glBindTexture(GL_TEXTURE_2D, mask_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void maskBuffer::set_uniform_color(const glm::u8 &color_rgb) {
    std::fill(pixels.begin(), pixels.end(), color_rgb);
    glBindTexture(GL_TEXTURE_2D, mask_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}