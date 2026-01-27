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

#define CUBE_MAP_NORTH_FACE 0
#define CUBE_MAP_ANTI_MERIDIAN_FACE 1
#define CUBE_MAP_WEST_FACE 2
#define CUBE_MAP_MERIDIAN_FACE 3
#define CUBE_MAP_EAST_FACE 4
#define CUBE_MAP_SOUTH_FACE 5

// Given a set of x,y,z coordinates, determine which face of a cube map it would be mapped to.
CubeMapFace cube_map_face(const glm::dvec3& coordinate) {
    auto abs_x = abs(coordinate.x);
    auto abs_y = abs(coordinate.y);
    auto abs_z = abs(coordinate.z);
    if (abs_y > abs_x && abs_y > abs_z) { return coordinate.y > 0.0 ? North : South; }
    if (abs_z > abs_x && abs_z >= abs_y) { return coordinate.z > 0.0 ? AntiMeridian : Meridian; }
    else/*(abs_x >= abs_y && abs_x >= abs_z)*/{ return coordinate.x > 0.0 ? West : East; }
}

// Given a face and a set of x,y,z coordinates, determine what u-v value the coordinate would have on the face.
// Useful for computing u-v value of lines which may have out-of-range segments
glm::dvec2 face_uv(const CubeMapFace face, const glm::dvec3& coord) {
    double k;
    switch (face) {
        case North: case South:
            k = 1.0 / abs(coord.y); break;
        case AntiMeridian: case Meridian:
            k = 1.0 / abs(coord.z); break;
        case West: case East:
            k = 1.0 / abs(coord.x); break;
        default: k = std::nan("");
    }
    switch (face) {
        case North:         return { -k*coord.z, -k*coord.x };
        case AntiMeridian:  return {  k*coord.x,  k*coord.y };
        case West:          return { -k*coord.z,  k*coord.y };
        case Meridian:      return { -k*coord.x,  k*coord.y };
        case East:          return {  k*coord.z,  k*coord.y };
        case South:         return { -k*coord.z,  k*coord.x };
    }
    return {std::nan(""), std::nan("")};
}

// Given a coordinate, computes the u-v value of that coordinate on its respective face
glm::dvec2 face_uv(const glm::dvec3& coord) {
    auto abs_x = abs(coord.x);
    auto abs_y = abs(coord.y);
    auto abs_z = abs(coord.z);
    double k;
    if (abs_y > abs_x && abs_y > abs_z) {
        k = 1.0f / abs_y;
        if (coord.y > 0.0) // North
            return { -k*coord.z, -k*coord.x };
        else // South
            return { -k*coord.z, k*coord.x };
    }
    else if (abs_z > abs_x && abs_z >= abs_y) {
        k = 1.0f / abs_z;
        if (coord.z > 0.0) // Antimeridian
            return { k*coord.x, k*coord.y };
        else // Meridian
            return { -k*coord.x, k*coord.y };
    }
    else if (abs_x >= abs_y && abs_x >= abs_z) {
        k = 1.0f / abs_x;
        if (coord.x > 0.0) // West
            return { -k*coord.z, k*coord.y };
        else // East
            return { k*coord.z, k*coord.y };
    }
    else { // Should not be possible
        return { std::nan(""), std::nan("") };
    }
}

// Given a xyz coordinate, returns (in order)
std::pair<CubeMapFace, glm::dvec2> cube_map_face_uv(const glm::dvec3& coord) {
    auto abs_x = abs(coord.x);
    auto abs_y = abs(coord.y);
    auto abs_z = abs(coord.z);
    double k;
    if (abs_y > abs_x && abs_y > abs_z) {
        k = 1.0f / abs_y;
        if (coord.y > 0.0)
            return { North, {-k * coord.z, -k * coord.x} };
        else
            return { South, {-k * coord.z, k * coord.x} };
    }
    if (abs_z > abs_x && abs_z >= abs_y) {
        k = 1.0f / abs_z;
        if (coord.z > 0.0)
            return { AntiMeridian, {k * coord.x, k * coord.y} };
        else
            return { Meridian, {-k * coord.x, k * coord.y} };
    }
    else /*(abs_x >= abs_y && abs_x >= abs_z)*/ {
        k = 1.0f / abs_x;
        if (coord.x > 0.0)
            return { West, {-k * coord.z, k * coord.y} };
        else
            return { East, {k * coord.z, k * coord.y} };
    }
}

std::pair<int, glm::dvec3> find_line_uv_intersection(
    CubeMapFace face, glm::dvec3 v0, glm::dvec3 v1,
    double u_low, double u_high, double v_low, double v_high) {
    // Normalize inputs
    v0 = normalize(v0);
    v1 = normalize(v1);
    // Get box intersections
    double v_intersect_u_low;
    double v_intersect_u_high;
    double u_intersect_v_low;
    double u_intersect_v_high;
    glm::dvec3 plane = normalize(cross(v0, v1));
    glm::dvec3 iv_ul = {0.0f, 0.0f, 0.0f};
    glm::dvec3 iv_uh = {0.0f, 0.0f, 0.0f};
    glm::dvec3 iu_vl = {0.0f, 0.0f, 0.0f};
    glm::dvec3 iu_vh = {0.0f, 0.0f, 0.0f};
    switch (face) {
        case North:
            v_intersect_u_low = (plane.y - plane.z * u_low) / plane.x;
            v_intersect_u_high = (plane.y - plane.z * u_high) / plane.x;
            u_intersect_v_low = (plane.y - plane.x * v_low) / plane.z;
            u_intersect_v_high = (plane.y - plane.x * v_high) / plane.z;
            iv_ul = {-v_intersect_u_low, 1.0f, -u_low};
            iv_uh = {-v_intersect_u_high, 1.0f, -u_high};
            iu_vl = {-v_low, 1.0f, -u_intersect_v_low};
            iu_vh = {-v_high, 1.0f, -u_intersect_v_high};
        break;
        case AntiMeridian:
            v_intersect_u_low = (-plane.z - plane.x * u_low) / plane.y;
            v_intersect_u_high = (-plane.z - plane.x * u_high) / plane.y;
            u_intersect_v_low = (-plane.z - plane.y * v_low) / plane.x;
            u_intersect_v_high = (-plane.z - plane.y * v_high) / plane.x;
            iv_ul = {u_low, v_intersect_u_low, 1.0f};
            iv_uh = {u_high, v_intersect_u_high, 1.0f};
            iu_vl = {u_intersect_v_low, v_low, 1.0f};
            iu_vh = {u_intersect_v_high, v_high, 1.0f};
        break;
        case West:
            v_intersect_u_low = (-plane.x + plane.z * u_low) / plane.y;
            v_intersect_u_high = (-plane.x + plane.z * u_high) / plane.y;
            u_intersect_v_low = (plane.x + plane.y * v_low) / plane.z;
            u_intersect_v_high = (plane.x + plane.y * v_high) / plane.z;
            iv_ul = {1.0f, v_intersect_u_low, -u_low};
            iv_uh = {1.0f, v_intersect_u_high, -u_high};
            iu_vl = {1.0f, v_low, -u_intersect_v_low};
            iu_vh = {1.0f, v_high, -u_intersect_v_high};
        break;
        case Meridian:
            v_intersect_u_low = (plane.z + plane.x * u_low) / plane.y;
            v_intersect_u_high = (plane.z + plane.x * u_high) / plane.y;
            u_intersect_v_low = (-plane.z + plane.y * v_low) / plane.x;
            u_intersect_v_high = (-plane.z + plane.y * v_high) / plane.x;
            iv_ul = {-u_low, v_intersect_u_low, -1.0f};
            iv_uh = {-u_high, v_intersect_u_high, -1.0f};
            iu_vl = {-u_intersect_v_low, v_low, -1.0f};
            iu_vh = {-u_intersect_v_high, v_high, -1.0f};
        break;
        case East:
            v_intersect_u_low = (plane.x - plane.z * u_low) / plane.y;
            v_intersect_u_high = (plane.x - plane.z * u_high) / plane.y;
            u_intersect_v_low = (plane.x - plane.y * v_low) / plane.z;
            u_intersect_v_high = (plane.x - plane.y * v_high) / plane.z;
            iv_ul = {-1.0f, v_intersect_u_low, u_low};
            iv_uh = {-1.0f, v_intersect_u_high, u_high};
            iu_vl = {-1.0f, v_low, u_intersect_v_low};
            iu_vh = {-1.0f, v_high, u_intersect_v_high};
        break;
        case South:
            v_intersect_u_low = (plane.y - plane.z * u_low) / plane.x;
            v_intersect_u_high = (plane.y - plane.z * u_high) / plane.x;
            u_intersect_v_low = (plane.y - plane.x * v_low) / plane.z;
            u_intersect_v_high = (plane.y - plane.x * v_high) / plane.z;
            iv_ul = {v_intersect_u_low, -1.0f, u_low};
            iv_uh = {v_intersect_u_high, -1.0f, u_high};
            iu_vl = {v_low, -1.0f, u_intersect_v_low};
            iu_vh = {v_high, -1.0f, u_intersect_v_high};
        break;
    }
    // Normalize result
    iv_ul = normalize(iv_ul);
    iv_uh = normalize(iv_uh);
    iu_vl = normalize(iu_vl);
    iu_vh = normalize(iu_vh);
    // Variables for closest intersection computation
    glm::dvec3 v1_direction = normalize(cross(cross(v0, v1), v0));
    double arc_v0v1 = acos(dot(v0, v1));
    double dst = arc_v0v1; // Should not be this by the end, but will be the result if v0 and v1 are both within the uv box bounds
    double arc;
    int out_edge = -1;
    glm::dvec3 out;
    // Calculate dot products with direction; positive means it is in the line
    double dot_iv_ul = dot(v1_direction, iv_ul);
    double dot_iv_uh = dot(v1_direction, iv_uh);
    double dot_iu_vl = dot(v1_direction, iu_vl);
    double dot_iu_vh = dot(v1_direction, iu_vh);
    // If in the right direction and closest, set the out vector to the respective value
    if (glm::sign(dot_iv_ul) > 0.0f) {
        arc = acos(dot_iv_ul);
        if (arc < dst) {
            dst = arc;
            out = iv_ul;
            out_edge = 0;
        }
    }
    if (glm::sign(dot_iv_uh) > 0.0f) {
        arc = acos(dot_iv_uh);
        if (arc < dst) {
            dst = arc;
            out = iv_uh;
            out_edge = 1;
        }
    }
    if (glm::sign(dot_iu_vl) > 0.0f) {
        arc = acos(dot_iu_vl);
        if (arc < dst) {
            dst = arc;
            out = iu_vl;
            out_edge = 2;
        }
    }
    if (glm::sign(dot_iu_vh) > 0.0f) {
        arc = acos(dot_iu_vl);
        if (arc < dst) {
            out = iu_vh;
            out_edge = 3;
        }
    }
    // Return the intersection edge/value
    return {out_edge, out};
}

glm::dvec3 uv_face_to_xyz(const CubeMapFace face, const glm::dvec2& uv) {
    glm::dvec3 xyz = {std::nan(""), std::nan(""), std::nan("")};
    switch (face) {
        case 0: xyz = normalize(glm::dvec3(-uv.y, 1.0f, -uv.x)); break;
        case 1: xyz = normalize(glm::dvec3(uv.x, uv.y, 1.0f)); break;
        case 2: xyz = normalize(glm::dvec3(1.0f, uv.y, -uv.x)); break;
        case 3: xyz = normalize(glm::dvec3(-uv.x, uv.y, -1.0f)); break;
        case 4: xyz = normalize(glm::dvec3(-1.0f, uv.y, uv.x)); break;
        case 5: xyz = normalize(glm::dvec3(uv.y, -1.0f, uv.x)); break;
        default: break;
    }
    return xyz;
}

std::pair<uint32_t, int> faceQuadtree::uv_index(const uint64_t path) const {
    uint32_t tracked_index = 0;
    uint32_t tree_index = 0;
    for (int depth=0; depth<=max_height; depth++) {
        const faceQuadtreeNode& curr_node = tree[tree_index];
        tracked_index += curr_node.left_family;
        // We are done if there are 0 or 1 children; no siblings
        if (curr_node.children_count == 0) return {UINT32_MAX, -1}; // Child does not exist
        if (curr_node.children_count == 1) return {tracked_index, curr_node.depth};
        // Else, we increment by potential left_family count and carry on to next loop
        const uint64_t dir = path << 2*depth >> 62;
        tree_index = curr_node.c_idx+dir;
    }
    return {tracked_index, tree[tree_index].depth};
}

std::pair<uint32_t, int> faceQuadtree::uv_index(const glm::dvec2 uv) const {
    return uv_index(uv_to_path(uv));
}

uint64_t faceQuadtree::uv_to_path(glm::dvec2 uv) {
    // Convert range from [-1.0, 1.0]^2 \in R^2 to [0, UINT32_MAX]^2 \in Z^2
    uv += glm::dvec2(1.0f, 1.0f); // [-1.0, 1.0] -> [0.0, 2.0]
    uv *= COORD_SCALE; // [0.0. 2.0] -> [0.0, 1.0] -> [0.0, max_coordinate]
    uint64_t u_idx = std::min(static_cast<uint32_t>(uv.x), UINT32_MAX); // min with max_height-offset needed to ensure [0, max) interval
    uint64_t v_idx = std::min(static_cast<uint32_t>(uv.y), UINT32_MAX);
    // Interleave (apparently _pdep_u64 instruct would work here, but I'm avoiding it for architecture compatibility
    u_idx = (u_idx ^ u_idx << 16) & 0x0000ffff0000ffff;
    u_idx = (u_idx ^ u_idx << 8 ) & 0x00ff00ff00ff00ff;
    u_idx = (u_idx ^ u_idx << 4 ) & 0x0f0f0f0f0f0f0f0f;
    u_idx = (u_idx ^ u_idx << 2 ) & 0x3333333333333333;
    u_idx = (u_idx ^ u_idx << 1 ) & 0x5555555555555555;

    v_idx = (v_idx ^ v_idx << 16) & 0x0000ffff0000ffff;
    v_idx = (v_idx ^ v_idx << 8 ) & 0x00ff00ff00ff00ff;
    v_idx = (v_idx ^ v_idx << 4 ) & 0x0f0f0f0f0f0f0f0f;
    v_idx = (v_idx ^ v_idx << 2 ) & 0x3333333333333333;
    v_idx = (v_idx ^ v_idx << 1 ) & 0x5555555555555555;

    return v_idx << 1 | u_idx;
}

faceQuadtree::faceQuadtree() {
    tree.emplace_back();
}

uint32_t faceQuadtree::split_tree_children(const uint32_t curr_depth, const uint64_t c_dir) {
    const uint32_t tree_insert_start = tree.size();
    // Insert 4 new nodes with incremented depth, 3 empty, and 1 holding the current child
    switch (c_dir) {
        case 0: tree.insert(tree.end(), {
            {curr_depth+1, 0, 0, 1},
            {curr_depth+1, 1, 0, 0},
            {curr_depth+1, 1, 0, 0},
            {curr_depth+1, 1, 0, 0}
        }); break;
        case 1: tree.insert(tree.end(), {
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 1},
            {curr_depth+1, 1, 0, 0},
            {curr_depth+1, 1, 0, 0}
        }); break;
        case 2: tree.insert(tree.end(), {
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 1},
            {curr_depth+1, 1, 0, 0}
        }); break;
        case 3: tree.insert(tree.end(), {
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 0},
            {curr_depth+1, 0, 0, 1}
        }); break;
        default: break;
    }
    return tree_insert_start;
}

void faceQuadtree::update_left_family(const uint32_t sibling_base_idx, const uint64_t dir) {
    switch (dir) {
        default: case 3: break;
        case 2: tree[sibling_base_idx+3].left_family += 1;
        case 1: tree[sibling_base_idx+2].left_family += 1;
        case 0: tree[sibling_base_idx+1].left_family += 1;
    }
}

#define FACE_QUAD_TREE_FULL UINT32_MAX
#define FACE_QUAD_TREE_NO_LEAF_AVAILABLE 0xfffffffeU
uint32_t faceQuadtree::insert(const glm::dvec2 &uv) {
    if (uvs.size() >= UINT32_MAX-2) return FACE_QUAD_TREE_FULL;
    uint32_t tracked_index = 0;
    uint32_t child_count = UINT32_MAX;
    const uint64_t ins_path = uv_to_path(uv);
    size_t curr_index = 0;
    // Loop to find node to insert into; will either have 0 or 1 children
    while(child_count >= 2) {
        const faceQuadtreeNode& curr_node = tree[curr_index];
        tracked_index += curr_node.left_family;
        const uint64_t ins_dir = ins_path << 2*curr_node.depth >> 62;
        curr_index = curr_node.c_idx+ins_dir;
        child_count = curr_node.children_count;
    }

    faceQuadtreeNode& curr_node = tree[curr_index];
    // CASE 1: Node has no children
    if (!child_count) {
        // Insertion is now confirmed. Update left_family for right uncle nodes. (but not right siblings)
        uint32_t tree_index = 0;
        for (int depth=0; depth < static_cast<long long>(curr_node.depth)-1; depth++) {
            const faceQuadtreeNode& curr_parent = tree[tree_index];
            const uint64_t dir = ins_path << 2*depth >> 62;
            tree_index = curr_parent.c_idx;
            update_left_family(tree_index, dir);
            tree_index += dir;
        }

        // Update siblings
        const uint64_t ins_dir = ins_path << 2*curr_node.depth >> 62;
        const uint32_t sibling_idx = tree[tree_index].c_idx;
        update_left_family(sibling_idx, ins_dir);

        // Insert the uv value & update children
        curr_node.children_count = 1;
        uvs.insert(uvs.begin()+tracked_index, uv);

        return tracked_index;
    }

    // CASE 2: Node has a child

    // Ensure that child node is not too close to the node to be inserted
    // (so close that the quadtree path matches current child).
    // If so, fail and return early
    const uint64_t c_path = uv_to_path(uvs[tracked_index]); // Gets the current node's corresponding uv value, then converts it to a path
    if (c_path == ins_path) return FACE_QUAD_TREE_NO_LEAF_AVAILABLE; // If the paths are equal, we can't store this new node. Paths must be unique.

    // Insertion is now confirmed. Update left_family for right uncle nodes, and right siblings.
    uint64_t c_dir = c_path << 2*curr_node.depth >> 62;
    uint64_t ins_dir = ins_path << 2*curr_node.depth >> 62;

    uint32_t tree_index = 0;
    for (int depth=0; depth<curr_node.depth; depth++) {
        const faceQuadtreeNode& curr_parent = tree[tree_index];
        const uint64_t dir = c_path << 2*depth >> 62;
        tree_index = curr_parent.c_idx;
        update_left_family(tree_index, dir);
        tree_index += dir;
    }

    // Split tree until paths diverge
    uint32_t curr_depth = curr_node.depth;
    while (c_dir == ins_dir) {
        // Get current node, covert it to a branch node. Note that tree_index starts at curr_node's index.
        faceQuadtreeNode& temp_node = tree[tree_index];
        temp_node.c_idx = split_tree_children(curr_depth, c_dir); // Update c_idx to point to start of children
        tree_index = temp_node.c_idx+ins_dir;
        // Go to next depth
        curr_depth++;
        c_dir = c_path << 2*curr_depth >> 62;
        ins_dir = ins_path << 2*curr_depth >> 62;
    }

    // Finally, create last set of children, one of which corresponds to the inserted uv.
    faceQuadtreeNode& final_parent = tree[tree_index];
    final_parent.c_idx = split_tree_children(curr_depth, c_dir);
    const uint32_t final_children_base_idx = final_parent.c_idx;
    const uint32_t inserted_child_idx = final_children_base_idx+ins_dir;
    tree[inserted_child_idx].children_count = 1;
    update_left_family(final_children_base_idx, ins_dir); // Update right siblings of inserted uv

    // Insert uv and return
    tracked_index += tree[inserted_child_idx].left_family;
    uvs.insert(uvs.begin() + tracked_index, uv);
    return tracked_index;
}

cubeMapCoordHandler::cubeMapCoordHandler(const std::vector<glm::dvec3> &xyz_positions,
                                         unsigned int max_render_depth, unsigned int min_render_depth) :
max_render_depth(max_render_depth), min_render_depth(min_render_depth)
{

}