#ifndef CUBEMAP_UTIL_H
#define CUBEMAP_UTIL_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "shapes.h"
#include <map>
#include <stack>
#include <set>

unsigned long pixel_index(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
unsigned long channel_index(unsigned long pixel_index, unsigned int channel, unsigned int channels);
void extend_cube_map_edges(unsigned char* data, unsigned int channels, unsigned int width, unsigned int height);

#endif //CUBEMAP_UTIL_H
