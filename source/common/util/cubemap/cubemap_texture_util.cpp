#include "cubemap_texture_util.h"
#include "mesh_util.h"
#include <unistd.h>
#include <deque>
#include <map>
#include <optional>
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

void load_cubemap_texture(const std::string& tex_path, GLuint& tex_handle, int& tex_width, int& tex_height, int& tex_channels) {
    // Load texture
    unsigned char* tex_data = stbi_load(tex_path.c_str(), &tex_width, &tex_height, &tex_channels, 0);
    if (!tex_data) {
        std::cout << "Failed to load texture data" << std::endl;
        throw std::exception();
    }
    // Extend edges by 1 pixel to cover up lines formed from floating point imprecision
    extend_cube_map_edges(tex_data, tex_channels, tex_width, tex_height);
    switch(tex_channels) {
    case 3: tex_handle = gen_texture_2d_rgb(tex_width, tex_height, tex_data); break;
    case 4: tex_handle = gen_texture_2d_rgba(tex_width, tex_height, tex_data); break;
    default:
        std::cout << "Unrecognized texture format" << std::endl;
        throw std::exception();
    }
    stbi_image_free(tex_data);
}


unsigned long pixel_index(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
    return x + (width * y);
}

unsigned long channel_index(unsigned long pixel_index, unsigned int channel, unsigned int channels) {
    return channel + (channels * pixel_index);
}

void extend_cube_map_edges(unsigned char* data, unsigned int channels, unsigned int width, unsigned int height) {
    unsigned long left_source_edge = width / 4;
    unsigned long left_modify_edge = left_source_edge - 1;
    unsigned long right_source_edge = (width / 2) - 1;
    unsigned long right_modify_edge = right_source_edge + 1;

    unsigned long top_source_edge = height / 3;
    unsigned long top_modify_edge = top_source_edge - 1;
    unsigned long bottom_source_edge = (2 * height / 3) - 1;
    unsigned long bottom_modify_edge = bottom_source_edge + 1;

    // Top left vertical edge
    for (int y = 0; y < top_source_edge; y++) {
        unsigned long src_pixel = pixel_index(left_source_edge, y, width, height);
        unsigned long dst_pixel = pixel_index(left_modify_edge, y, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Top right vertical edge
    for (int y = 0; y < top_source_edge; y++) {
        unsigned long src_pixel = pixel_index(right_source_edge, y, width, height);
        unsigned long dst_pixel = pixel_index(right_modify_edge, y, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Top left horizontal edge
    for (int x = 0; x < left_source_edge; x++) {
        unsigned long src_pixel = pixel_index(x, top_source_edge, width, height);
        unsigned long dst_pixel = pixel_index(x, top_modify_edge, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Top right horizontal edge
    for (int x = (int)right_modify_edge; x < width; x++) {
        unsigned long src_pixel = pixel_index(x, top_source_edge, width, height);
        unsigned long dst_pixel = pixel_index(x, top_modify_edge, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Bottom left horizontal edge
    for (int x = 0; x < left_source_edge; x++) {
        unsigned long src_pixel = pixel_index(x, bottom_source_edge, width, height);
        unsigned long dst_pixel = pixel_index(x, bottom_modify_edge, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Bottom right horizontal edge
    for (int x = (int)right_modify_edge; x < width; x++) {
        unsigned long src_pixel = pixel_index(x, bottom_source_edge, width, height);
        unsigned long dst_pixel = pixel_index(x, bottom_modify_edge, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Bottom left vertical edge
    for (int y = (int)bottom_modify_edge; y < height; y++) {
        unsigned long src_pixel = pixel_index(left_source_edge, y, width, height);
        unsigned long dst_pixel = pixel_index(left_modify_edge, y, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Bottom right vertical edge
    for (int y = (int)bottom_modify_edge; y < height; y++) {
        unsigned long src_pixel = pixel_index(right_source_edge, y, width, height);
        unsigned long dst_pixel = pixel_index(right_modify_edge, y, width, height);
        unsigned long src_data_idx = channels*src_pixel;
        unsigned long dst_data_idx = channels*dst_pixel;
        memcpy(data+dst_data_idx, data+src_data_idx, channels);
    }

    // Four corners
    // Top left
    unsigned long src_pixel = pixel_index(left_source_edge, top_source_edge, width, height);
    unsigned long dst_pixel = pixel_index(left_modify_edge, top_modify_edge, width, height);
    unsigned long src_data_idx = channels*src_pixel;
    unsigned long dst_data_idx = channels*dst_pixel;
    memcpy(data+dst_data_idx, data+src_data_idx, channels);
    // Top right
    src_pixel = pixel_index(right_source_edge, top_source_edge, width, height);
    dst_pixel = pixel_index(right_modify_edge, top_modify_edge, width, height);
    src_data_idx = channels*src_pixel;
    dst_data_idx = channels*dst_pixel;
    memcpy(data+dst_data_idx, data+src_data_idx, channels);
    // Bottom left
    src_pixel = pixel_index(left_source_edge, bottom_source_edge, width, height);
    dst_pixel = pixel_index(left_modify_edge, bottom_modify_edge, width, height);
    src_data_idx = channels*src_pixel;
    dst_data_idx = channels*dst_pixel;
    memcpy(data+dst_data_idx, data+src_data_idx, channels);
    // Bottom right
    src_pixel = pixel_index(right_source_edge, bottom_source_edge, width, height);
    dst_pixel = pixel_index(right_modify_edge, bottom_modify_edge, width, height);
    src_data_idx = channels*src_pixel;
    dst_data_idx = channels*dst_pixel;
    memcpy(data+dst_data_idx, data+src_data_idx, channels);
}

CubemapTextureMeshData get_full_cubemap_texture_mesh_data(CubeFaceNum face) {
    // Possible uv X values
    constexpr float x0 = 0.0f;
    constexpr float x1 = 0.25f;
    constexpr float x2 = 0.5f;
    constexpr float x3 = 0.75f;
    constexpr float x4 = 1.0f;
    // Possible uv Y values
    constexpr float y0 = 0.0f;
    constexpr float y1 = (1.0f / 3.0f);
    constexpr float y2 = (2.0f / 3.0f);
    constexpr float y3 = 1.0f;

    // glm::vec2 uvs[20] = { // Note that some uvs are not possible
    // {x0, y0}, {x1, y0}, {x2, y0}, {x3, y0}, {x4, y0},
    // {x0, y1}, {x1, y1}, {x2, y1}, {x3, y1}, {x4, y1},
    // {x0, y2}, {x1, y2}, {x2, y2}, {x3, y2}, {x4, y2},
    // {x0, y3}, {x1, y3}, {x2, y3}, {x3, y3}, {x4, y3},
    // };

    switch (face) {
    case North: return {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x1, y1}, {x2, y1}, {x2, y0}, {x1, y0}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    case East: return { // Yes I know East and West are switched. I don't know why but it fixes a bug
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x0, y2}, {x1, y2}, {x1, y1}, {x0, y1}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    case Meridian: return {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x1, y2}, {x2, y2}, {x2, y1}, {x1, y1}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    case West: return {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x2, y2}, {x3, y2}, {x3, y1}, {x2, y1}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    case AntiMeridian: return {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x3, y2}, {x4, y2}, {x4, y1}, {x3, y1}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    case South: return {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}},
        {{x1, y3}, {x2, y3}, {x2, y2}, {x1, y2}},
        {{0, 1, 2}, {0, 2, 3}}
    };
    }
    return {
        {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f)},
        {glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f)},
        {glm::u32vec3(0), glm::u32vec3(0)}
    };
}