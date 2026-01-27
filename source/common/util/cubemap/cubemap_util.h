#ifndef CUBEMAP_UTIL_H
#define CUBEMAP_UTIL_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "shapes.h"
#include <map>
#include <stack>
#include <set>

enum CubeMapFace : int {
    North = 0,
    AntiMeridian = 1,
    West = 2,
    Meridian = 3,
    East = 4,
    South = 5
};

unsigned long pixel_index(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
unsigned long channel_index(unsigned long pixel_index, unsigned int channel, unsigned int channels);
void extend_cube_map_edges(unsigned char* data, unsigned int channels, unsigned int width, unsigned int height);
CubeMapFace cube_map_face(const glm::dvec3& coordinate);
glm::dvec2 face_uv(CubeMapFace face, const glm::dvec3& coord);
glm::dvec2 face_uv(const glm::dvec3& coord);
std::pair<CubeMapFace, glm::dvec2> cube_map_face_uv(const glm::dvec3& coord);
std::pair<int, glm::dvec3> find_line_uv_intersection(int face, glm::dvec3 v0, glm::dvec3 v1, double u_low, double u_high, double v_low, double v_high);
glm::dvec3 uv_face_to_xyz(CubeMapFace face, const glm::dvec2& uv);

struct faceQuadtreeNode {
    uint32_t depth = 0;
    uint32_t left_family = 0; // Sum of children of the siblings left of this node
    uint32_t c_idx = 0; // Index into uvs if children_count = 1, else index into tree
    uint32_t children_count = 0;

    faceQuadtreeNode() = default;
    explicit faceQuadtreeNode(const uint32_t depth) : depth(depth) {}
    faceQuadtreeNode(const uint32_t depth, const uint32_t left_family, const uint32_t c_idx, const uint32_t children_count)
    : depth(depth), left_family(left_family), c_idx(c_idx), children_count(children_count) {}
};

struct faceQuadtree {
    unsigned int max_height = 0;
    constexpr static uint64_t COORD_RANGE = static_cast<uint64_t>(UINT32_MAX) + 1ull;
    constexpr static double COORD_SCALE = COORD_RANGE >> 1;
    std::vector<glm::dvec2> uvs;
    std::vector<faceQuadtreeNode> tree;

    faceQuadtree();

    [[nodiscard]] uint32_t split_tree_children(uint32_t curr_depth, uint64_t c_dir);
    void update_left_family(uint32_t sibling_base_idx, uint64_t dir);
    [[nodiscard]] std::pair<uint32_t, int> uv_index(uint64_t path) const;
    [[nodiscard]] std::pair<uint32_t, int> uv_index(glm::dvec2 uv) const;
    [[nodiscard]] static uint64_t uv_to_path(glm::dvec2 uv);
    uint32_t insert(const glm::dvec2& uv);
};

class cubeMapCoordHandler {
    unsigned int max_render_depth;
    unsigned int min_render_depth;

public:
    cubeMapCoordHandler(const std::vector<glm::dvec3>& xyz_positions, unsigned int max_render_depth, unsigned int min_render_depth);
};

#endif //FANTASYPLATES_CUBEMAP_UTIL_H
