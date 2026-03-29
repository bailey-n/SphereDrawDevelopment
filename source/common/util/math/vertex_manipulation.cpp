//
// Created by Nathan on 2/27/2026.
//


#include "vertex_manipulation.h"
#include "cubemap_texture_util.h"
#include "sweepline_util.h"
#include "common_glm_operations.h"
#include <cstdint>
#include <cmath>
#include <queue>
#include <utility>
#include <vector>
#include <algorithm>


glm::vec3 slerp(const glm::vec3& u, const glm::vec3& v, float t) {
    const float angle = std::acos(glm::dot(u, v));
    return glm::normalize(((std::sin((1.0f-t)*angle) * u) + (std::sin(t*angle) * v) / std::sin(angle)));
}

std::vector<glm::vec3> subdivide_line(const glm::vec3&  u, const glm::vec3& v, double max_subdiv_angle) {
    if (glm::dot(u, v) > 0.9999) return {u, v};
    double total_angle = std::acos(glm::dot(u, v));
    float divs = std::ceil(total_angle / max_subdiv_angle);
    int num_div = (int)divs;

    std::vector<glm::vec3> output;
    output.reserve(num_div + 1);

    output.emplace_back(u);
    for (int i = 1; i < num_div; i++) {
        float t = (float)i / divs;
        output.emplace_back(slerp(u, v, t));
    }
    output.emplace_back(v);

    return output;
}

using Edge = std::pair<uint32_t, uint32_t>;
using Triangle = glm::u32vec3;

void make_sphere_circle(glm::vec3 center, float radius, std::vector<glm::vec3>& positions_buff, std::vector<glm::u32vec3>& indices_buff) {
    // Approximate a circle
    // TODO: make the circle look better dynamically using: function of radius, then geometry shader
    constexpr int APPROX_DEPTH = 4;
    // Initial triangle: equilateral triangle inscribed in the unit circle
    std::vector<glm::vec2> vertices = {
        glm::vec2(1.0, 0.0),
        glm::vec2(-0.5, std::sqrt(0.75)),
        glm::vec2(-0.5, -std::sqrt(0.75)),
    };

    // Edges to expand & triangles to inscribe. Note the use of queue pointer swapping.
    std::queue<Triangle> triangles_q1{{Triangle{0, 1, 2}}};
    std::queue<Triangle> triangles_q2;
    std::queue<Edge> edges_q1 {{Edge(0, 1), Edge(1, 2), Edge(2, 0)}};
    std::queue<Edge> edges_q2;
    std::queue<Triangle>* curr_triangles = &triangles_q1;
    std::queue<Triangle>* new_triangles = &triangles_q2;
    std::queue<Edge>* curr_edges = &edges_q1;
    std::queue<Edge>* new_edges = &edges_q2;

    // Make layers of triangles to approximate a circle
    for (int i = 1; i < APPROX_DEPTH; i++) {
        while (!curr_edges->empty()) {
            Edge curr_edge = curr_edges->front();
            curr_edges->pop();

            // Get midpoint of Edge and push it to edge of circle by normalizing it
            uint32_t new_vtx_idx = vertices.size();
            vertices.emplace_back(glm::normalize(vertices[curr_edge.first] + vertices[curr_edge.second]));
            new_edges->emplace(curr_edge.first, new_vtx_idx);
            new_edges->emplace(new_vtx_idx, curr_edge.second);
            new_triangles->emplace(curr_edge.first, new_vtx_idx, curr_edge.second);
        }
        while (!curr_triangles->empty()) {
            Triangle curr_triangle = curr_triangles->front();
            curr_triangles->pop();

            // Get triangle vertices from midpoint of triangles.
            uint32_t new_idx = vertices.size();
            // Inscribes triangle via finding edge midpoints
            vertices.emplace_back(0.5f * (vertices[curr_triangle.x] + vertices[curr_triangle.y]));
            vertices.emplace_back(0.5f * (vertices[curr_triangle.y] + vertices[curr_triangle.z]));
            vertices.emplace_back(0.5f * (vertices[curr_triangle.z] + vertices[curr_triangle.x]));
            // Subdivides into four triangles
            new_triangles->emplace(new_idx, new_idx+1, new_idx+2);
            new_triangles->emplace(curr_triangle.x, new_idx, new_idx+2);
            new_triangles->emplace(curr_triangle.y, new_idx+1, new_idx);
            new_triangles->emplace(curr_triangle.z, new_idx+2, new_idx+1);
        }

        std::swap(curr_triangles, new_triangles);
        std::swap(curr_edges, new_edges);
    }
    // Note: we end with curr_triangles pointer containing all new triangles generated in the last round.
    // We will discard all previous triangles and use only the new triangles.
    // First, we map the 2d circle coords to a hemisphere
    float scale = std::sin(radius);
    glm::tmat4x4<float> transform = glm::mat4x4(1.0f); // Scaling
    glm::vec3 z_axis{0.0, 0.0, 1.0};
    float z_dot = glm::dot(center, z_axis);
    if (z_dot != 1.0) { // if z_dot == 1, then the normalization will fail
        transform = glm::rotate(transform, std::acos(z_dot), glm::normalize(glm::cross(z_axis, center)));
    }

    // Modify mesh data directly (to avoid unnecessary copies)
    positions_buff.clear();
    positions_buff.reserve(vertices.size());
    for (const auto& vertex: vertices) {
        glm::vec4 base_point(scale * vertex.x, scale * vertex.y, std::cos(radius), 1.0);
        glm::vec3 point = glm::normalize(glm::vec3(transform * base_point));
        positions_buff.emplace_back(point);
    }

    indices_buff.clear();
    indices_buff.reserve(curr_triangles->size());
    while (!curr_triangles->empty()) {
        indices_buff.emplace_back(curr_triangles->front());
        curr_triangles->pop();
    }
}

ConnectorQuad::ConnectorQuad(const glm::vec4& tl, const glm::vec4& bl, const glm::vec4& br, const glm::vec4& tr) :
top_left(tl), bottom_left(bl), bottom_right(br), top_right(tr) {}

std::vector<glm::vec3> ConnectorQuad::subdivide_left(double max_subdiv_angle) const {
    return subdivide_line(top_left, bottom_left, max_subdiv_angle);
}

std::vector<glm::vec3> ConnectorQuad::subdivide_right(double max_subdiv_angle) const {
    return subdivide_line(top_right, bottom_right, max_subdiv_angle);
}

std::vector<ConnectorQuad> connect_vertices(const glm::vec3& left, const glm::vec3& right, float width, double subdiv_angle_threshold) {
    // if (glm::dot(left, right) > 0.9999) return {};
    double total_angle = std::acos(glm::dot(left, right));
    int subdivs = (int)std::ceil(total_angle / subdiv_angle_threshold) - 1;
    auto subdiv_angle = (float)(total_angle / (subdivs+1));

    // Get initial rotation, left side of rectangle, right side of rectangle
    float tan_w = std::tan(width);
    glm::vec3 rot_axis = glm::normalize(glm::cross(left, right));
    glm::vec3 offset = tan_w * rot_axis; // TODO: Make inclusive of angle PI/2. May break at ~PI/2 otherwise. Use multiple-cross-product instead.
    glm::vec4 top_left = glm::vec4((glm::normalize(left + offset)), 1.0f);
    glm::vec4 bottom_left = glm::vec4((glm::normalize(left - offset)), 1.0f);
    glm::vec4 top_end = glm::vec4((glm::normalize(right + offset)), 1.0f);
    glm::vec4 bottom_end = glm::vec4((glm::normalize(right - offset)), 1.0f);

    // Rotate until we get all the necessary quads
    std::vector<ConnectorQuad> output;
    auto rotation = glm::rotate(glm::mat4x4(1.0f), subdiv_angle, rot_axis);
    output.reserve(subdivs+1);
    for (int i = 0; i < subdivs; i++) {
        auto top_right = rotation * top_left;
        auto bottom_right = rotation * bottom_left;
        output.emplace_back(top_left, bottom_left, bottom_right, top_right);
        top_left = top_right;
        bottom_left = bottom_right;
    }
    output.emplace_back(top_left, bottom_left, bottom_end, top_end); // last quad

    return output;
};

void make_sphere_line(const std::vector<glm::vec3>& input_verts, float width, std::vector<glm::vec3>& positions, std::vector<glm::u32vec3>& indices) {
    if (input_verts.empty()) return;
    if (input_verts.size() == 1) {
        make_sphere_circle(input_verts[0], width, positions, indices);
        return;
    }

    // Build connecting rectangles
    for (int i = 0; i < input_verts.size()-1; i++) {
        auto rectangle_quads = connect_vertices(input_verts[i], input_verts[i+1], width, MAX_SUBDIV_WIDTH);
        auto top_left = rectangle_quads[0].top_left;
        auto bottom_left = rectangle_quads[0].bottom_left;
        positions.emplace_back(top_left);
        positions.emplace_back(bottom_left);

        for (const auto& quad: rectangle_quads) {
            size_t start_idx = positions.size()-2;
            positions.emplace_back(quad.top_right);
            positions.emplace_back(quad.bottom_right);

            indices.emplace_back(start_idx, start_idx+1, start_idx+3);
            indices.emplace_back(start_idx, start_idx+3, start_idx+2);

            // Move quad forward
            top_left = quad.top_right;
            bottom_left = quad.bottom_right;
        }
    }
}

void compose_polygon(const std::vector<glm::vec3>& input_verts, std::vector<glm::vec3>& positions, std::vector<glm::u32vec3>& indices) {
    if (input_verts.size() < 3) return;
    positions.clear();
    auto n = input_verts.size();

    // Subdivide arcs to ensure smaller than max length
    for (unsigned int i = 0; i < n; i++) {
        unsigned int j = next_idx(n, i);
        auto arc_dist = arc_length(input_verts[i], input_verts[j]);
        auto subdivs = (arc_dist / MAX_ARC_LENGTH_RADIANS);
        positions.emplace_back(input_verts[i]);
        if (subdivs > 1.0f) {
            // Subdivide via partial rotation
            float subdiv_ct = std::ceil(subdivs);
            auto subdiv_ct_int = (unsigned int)subdiv_ct;
            float rotate_distance = arc_dist / subdiv_ct;
            glm::vec3 axis = glm::normalize(glm::cross(input_verts[i], input_verts[j]));
            glm::mat4x4 base_rot = glm::rotate(glm::mat4x4(1.0f), rotate_distance, axis);
            glm::mat4x4 curr_rot = base_rot;
            for (int k = 1; k < subdiv_ct_int; k++) {
                positions.emplace_back(apply_rotation(curr_rot, input_verts[i]));
                curr_rot = base_rot * curr_rot;
            }
        }
        // Note that for each loop input_verts[j] is not added. This is assumed to be added in the next loop as input_verts[i].
    }

    // Find where lines cross cubeface borders
    n = positions.size();

    for (unsigned int i = 0; i < n; i++) {
        unsigned int j = next_idx(n, i);
    }

//    indices.clear();
//    indices.reserve(input_verts.size());
//    positions = input_verts;
//
//    // Create vector of indices
//    std::vector<unsigned int> v_indices;
//    v_indices.reserve(input_verts.size());
//    for (unsigned int i = 0; i < input_verts.size(); i++) {
//        v_indices.emplace_back(i);
//    }
//
//    // Sort indices (rather than points themselves; their order is important for segments)
//    std::sort(v_indices.begin(), v_indices.end(),[&positions](unsigned int i, unsigned int j){
//        return (positions[i].x < positions[j].x) || (positions[i].x == positions[j].x && positions[i].y < positions[j].y);
//    });
//
//    // Create event queue and fill with vertex events.
//    auto n = positions.size();
//    std::priority_queue<SweepLineEvent> event_queue;
//    for (auto& v_idx: v_indices) {
//        auto prev = prev_idx(n, v_idx);
//        bool prev_before = (positions[prev].x < positions[v_idx].x) || (positions[prev].x == positions[v_idx].x && positions[prev].y < positions[v_idx].y);
//
//        auto next = next_idx(n, v_idx);
//        bool next_before = (positions[next].x < positions[v_idx].x) || (positions[next].x == positions[v_idx].x && positions[next].y < positions[v_idx].y);
//
//        if (!prev_before && !next_before) {
//            event_queue.emplace(SweepLineEvent::Type::Vertex_Split, positions[v_idx], SweepLineEventData(v_idx, false));
//        }
//        else if (prev_before && next_before) {
//            event_queue.emplace(SweepLineEvent::Type::Vertex_Merge, positions[v_idx], v_idx);
//        }
//        else {
//            event_queue.emplace(SweepLineEvent::Type::Vertex_Segment, positions[v_idx], v_idx);
//        }
//    }
//
//    // Create sweep line data structure
//    std::set<SweepSegment> sweep_segments;
//
//    // Process events
//    while(!event_queue.empty()) {
//        auto event = event_queue.top();
//        event_queue.pop();
//
//        if (event.type == SweepLineEvent::Type::Vertex_Split) {
//            SweepSegment::sweep_x = event.position.x + 0.000001f; // Is a tiny bit forward to allow for proper evaluation during insertion
//            auto insert_1 = sweep_segments.emplace(positions, event.data.idx, true);
//            auto insert_2 = sweep_segments.emplace(positions, event.data.idx, false);
//            if (!insert_1.second || !insert_2.second) throw std::exception(); // This shouldn't fail
//
//            auto lower_segment = *insert_1.first < *insert_2.first ? insert_1.first : insert_2.first;
//            auto upper_segment = *insert_1.first < *insert_2.first ? insert_2.first : insert_1.first;
//
//            if (lower_segment != sweep_segments.begin()) {
//                auto cmp_down = lower_segment;
//                cmp_down--;
//                float s1 = get_intersect_t(positions, *lower_segment, *cmp_down);
//                float t1 = get_intersect_t(positions, *cmp_down, *lower_segment);
//                if ((0.0 <= s1 && s1 <= 1.0) && (0.0 <= t1 && t1 <= 1.0)) {
//                    event_queue.emplace(SweepLineEvent::Type::Intersect, lower_segment->at_t(s1));
//                }
//            }
//
//            if ((++upper_segment) != sweep_segments.end()) {
//                auto cmp_up = upper_segment;
//                upper_segment--;
//                float s2 = get_intersect_t(positions, *upper_segment, *cmp_up);
//                float t2 = get_intersect_t(positions, *cmp_up, *upper_segment);
//                if ((0.0 <= s2 && s2 <= 1.0) && (0.0 <= t2 && t2 <= 1.0)) {
//                    event_queue.emplace(SweepLineEvent::Type::Intersect, upper_segment->at_t(s2));
//                }
//            }
//        }
//
//        else if (event.type == SweepLineEvent::Type::Vertex_Merge) {
//            SweepSegment::sweep_x = event.position.x - 0.000001f; // Is a tiny bit behind to allow for proper evaluation during comparison
//            auto lower_segment = *event.data.segment_1 < *event.data.segment_2 ? *event.data.segment_1 : *event.data.segment_2;
//            auto upper_segment = *event.data.segment_1 < *event.data.segment_2 ? *event.data.segment_2 : *event.data.segment_1;
//
//            // Get the data for segments below lower segment and above upper segment
//            auto lower_segment_itt = sweep_segments.find(lower_segment);
//            auto upper_segment_itt = sweep_segments.find(upper_segment);
//            // Nothing to test if there is nothing above/below these segments.
//            if (lower_segment_itt == sweep_segments.begin() || upper_segment_itt == sweep_segments.end()) continue;
//
//            auto merge_lower = *--lower_segment_itt;
//            auto merge_upper = *++upper_segment_itt;
//
//            // Delete segments
//            sweep_segments.erase(lower_segment);
//            sweep_segments.erase(upper_segment);
//
//            // Test if merge_lower and merge_upper intersect
//            float s = get_intersect_t(positions, merge_lower, merge_upper);
//            float t = get_intersect_t(positions, merge_upper, merge_lower);
//            if ((0.0 <= s && s <= 1.0) && (0.0 <= t && t <= 1.0)) {
//                event_queue.emplace(SweepLineEvent::Type::Intersect, lower_segment.at_t(s));
//            }
//        }
//
//        else if (event.type == SweepLineEvent::Type::Vertex_Segment) {
//            SweepSegment::sweep_x = event.position.x;
//
//            // Swap target segment data with new segment data and test for intersections
//            // with the segments above/below it
//        }
//
//        else if (event.type == SweepLineEvent::Type::Intersect) {
//            SweepSegment::sweep_x = event.position.x;
//
//            // Swap the intersecting segments data, add intersection to overall position list,
//            // test for intersection of new upper segment with segment above it (if it exists)
//            // and of new lower segment with segment below it (if it exists)
//        }
//    }
}
