#ifndef CUBEMAP_UTIL_H
#define CUBEMAP_UTIL_H

#include "opengl_include.h"
#include <vector>
#include "shapes.h"
#include <map>
#include <stack>
#include <set>

void load_cubemap_texture(const std::string& tex_path, GLuint& tex_handle, int& tex_width, int& tex_height, int& tex_channels);
unsigned long pixel_index(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
unsigned long channel_index(unsigned long pixel_index, unsigned int channel, unsigned int channels);
void extend_cube_map_edges(unsigned char* data, unsigned int channels, unsigned int width, unsigned int height);
unsigned char* add_alpha_channel(unsigned char* data, unsigned int width, unsigned int height);

struct CubemapTextureMeshData {
    glm::vec3 vertices[4];
    glm::vec2 uvs[4];
    glm::u32vec3 indices[2];
};

CubemapTextureMeshData get_full_cubemap_texture_mesh_data(CubeFaceNum face);

#endif //CUBEMAP_UTIL_H
