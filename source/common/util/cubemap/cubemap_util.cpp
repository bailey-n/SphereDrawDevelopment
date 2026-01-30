#include "cubemap_util.h"
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