#include "cubemap_texture_util.h"
#include <unistd.h>
#include <deque>
#include <map>
#include <optional>

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
    /* North Face: [bl: (1, 1, 1)] [br: (1, 1, -1)] [tr: (-1, 1, -1)] [tl: (-1, 1, 1)]         <-> 7 6 2 3
     * West Face: [bl: (-1, -1, 1)] [br: (1, -1, 1)] [tr: (1, 1, 1)] [tl: (-1, 1, 1)]         <-> 1 5 7 3
     * Meridian Face: [bl: (1, -1, 1)] [br: (1, -1, -1)] [tr: (1, 1, -1)] [tl: (1, 1, 1)]         <-> 5 4 6 7
     * East Face: [bl: (1, -1, -1)] [br: (-1, -1, -1)] [tr: (-1, 1, -1)] [tl: (1, 1, -1)]     <-> 4 0 2 6
     * AntiMeridian Face: [bl: (-1, -1, -1)] [br: (-1, -1, 1)] [tr: (-1, 1, 1)] [tl: (-1, 1, -1)]     <-> 0 1 3 2
     * South Face: [bl: (-1, -1, 1)] [br: (-1, -1, -1)] [tr: (1, -1, -1)] [tl: (1, -1, 1)]     <-> 1 0 4 5
     */

    constexpr float c = std::sqrt(1.0f / 3.0f);
    glm::vec3 corners[8] = {
        {-c, -c, -c},
        {-c, -c, c},
        {-c, c, -c},
        {-c, c, c},
        {c, -c, -c},
        {c, -c, c},
        {c, c, -c},
        {c, c, c},
        };
    glm::vec2 uvs[4] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
    };
    glm::u32vec3 indices[2] = {
        {0, 1, 2}, {0, 2, 3}
    };

    switch (face) {
    case North: return {
        {corners[7], corners[6], corners[2], corners[3]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
            {indices[0], indices[1]}
    };
    case West: return {
        {corners[1], corners[5], corners[7], corners[3]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case Meridian: return {
        {corners[5], corners[4], corners[6], corners[7]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case East: return {
        {corners[4], corners[0], corners[2], corners[6]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case AntiMeridian: return {
        {corners[0], corners[1], corners[3], corners[2]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    case South: return {
        {corners[1], corners[0], corners[4], corners[5]},
        {uvs[0], uvs[1], uvs[2], uvs[3]},
        {indices[0], indices[1]}
    };
    }
    return {
        {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f)},
        {glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f)},
        {glm::u32vec3(0), glm::u32vec3(0)}
    };
}