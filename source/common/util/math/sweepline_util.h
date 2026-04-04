//
// Created by Nathan on 3/26/2026.
//

#ifndef SPHEREDRAW_SWEEPLINE_UTIL_H
#define SPHEREDRAW_SWEEPLINE_UTIL_H

#include "opengl_include.h"
#include <vector>
#include <optional>
#include <map>
#include <optional>
#include <algorithm>

std::optional<glm::vec3> get_sphere_line_intersection(const glm::vec3& A, const glm::vec3& B, const glm::vec3& P, const glm::vec3& Q);

inline glm::vec3 stereographic_project(glm::vec3 sphere_pt) {
    auto denom = (sphere_pt.z < 0.999995) ? (1.0f / (1.0f - sphere_pt.z)) : std::numeric_limits<float>::max();
    return {sphere_pt.x * denom, sphere_pt.y * denom, 0.0f};
}

// Helper functions and struct for polygon composition
inline unsigned int next_idx(size_t max, unsigned int curr) { return (curr+1) % max; }
inline unsigned int prev_idx(size_t max, unsigned int curr) { return std::min(curr-1, (unsigned)max-1);}
inline float arc_length(const glm::vec3& A, const glm::vec3 B) {return std::acos(glm::dot(A, B));}
constexpr float MAX_ARC_LENGTH_RADIANS = 0.6154f;

enum FaceTransition {
    No_Transition = 0,
    N_to_W = 0b000001, N_to_M = 0b000010, N_to_E = 0b000011, N_to_A = 0b000100,
    W_to_N = 0b001000, W_to_M = 0b001010, W_to_A = 0b001100, W_to_S = 0b001101,
    M_to_N = 0b010000, M_to_W = 0b010001, M_to_E = 0b010011, M_to_S = 0b010101,
    E_to_N = 0b011000, E_to_M = 0b011010, E_to_A = 0b011100, E_to_S = 0b011101,
    A_to_N = 0b100000, A_to_W = 0b100001, A_to_E = 0b100011, A_to_S = 0b100101,
    S_to_W = 0b101001, S_to_M = 0b101010, S_to_E = 0b101011, S_to_A = 0b101100
};

struct FaceCrossing {
    FaceTransition type;
    glm::vec3 location;
};

struct FaceCrossingData {
    unsigned int crossing_count = 0;
    FaceCrossing crossings[4];
};

FaceCrossingData compute_face_crossings(const glm::vec3& A, const glm::vec3& B);

struct SweepSegment {
    unsigned int prev_v_idx;
    unsigned int next_v_idx;
    glm::vec3 prev_v_pos;
    glm::vec3 next_v_pos;
    float slope;
    bool fwd;
    bool vertical;

    glm::vec3 abs_dir;
    glm::vec3 unit_dir;
    glm::vec3 perp_unit_dir;
    float len;

    static float sweep_x;

    [[nodiscard]] float eval() const {
        return vertical ? prev_v_pos.y : prev_v_pos.y + slope*(sweep_x - prev_v_pos.x);
    }

    [[nodiscard]] glm::vec3 at_t(float t) const {
        return prev_v_pos + t*abs_dir;
    }

    SweepSegment(const std::vector<glm::vec3>& positions, unsigned int prev_v, bool forward) : prev_v_idx(prev_v), fwd(forward) {
        auto n = positions.size();
        next_v_idx = fwd ? next_idx(n, prev_v_idx) : prev_idx(n, prev_v_idx);
        prev_v_pos = positions[prev_v_idx];
        next_v_pos = positions[next_v_idx];
        vertical = positions[prev_v_idx].x == positions[next_v_idx].x;
        slope = vertical ? std::nanf("") : (next_v_pos.y - prev_v_pos.y) / (next_v_pos.x - prev_v_pos.x);

        abs_dir = positions[next_v_idx] - positions[prev_v_idx];
        unit_dir = glm::normalize(abs_dir);
        perp_unit_dir = {-unit_dir.y, unit_dir.x, unit_dir.z};
        len = glm::length(abs_dir);
    }

    bool operator<(const SweepSegment& rhs) const { return this->eval() < rhs.eval(); }
};

// Note this is determinant for 2x2 matrix; l.z and r.z are assumed to be 0
inline float det(const glm::vec3& l, const glm::vec3& r) {
    return (l.x * r.y) - (l.y - r.x);
}

// Determine 'time' parameter for when segment 1 intersects segment 2.
// t=0 would be at segment_1.prev, t=1 would be at segment_1.next. Outside that range is no intersection.
float get_intersect_t(const std::vector<glm::vec3>& positions, const SweepSegment& segment_1, const SweepSegment& segment_2) {
    const glm::vec3& v_11 = positions[segment_2.prev_v_idx];
    const glm::vec3& v_12 = positions[segment_2.next_v_idx];
    const glm::vec3& v_21 = positions[segment_1.prev_v_idx];
    const glm::vec3& v_22 = positions[segment_1.next_v_idx];

    float det_1 = det(v_11, v_21);
    float det_2 = det(v_12, v_11);
    float det_3 = det(v_21, v_12);
    float det_4 = det(v_12, v_22);
    float det_5 = det(v_22, v_11);

    return (det_1 + det_2 + det_3) / (det_1 + det_3 + det_4 + det_5);
}

struct SweepLineEventData {
    unsigned int idx;
    bool fwd;
    std::optional<SweepSegment> segment_1;
    std::optional<SweepSegment> segment_2;

    SweepLineEventData() : idx(-1), fwd(false) {}
    SweepLineEventData(unsigned int idx, bool fwd) : idx(idx), fwd(fwd) {}
};

struct SweepLineEvent {
    enum Type {
        Vertex_Split = 0,
        Vertex_Segment = 1,
        Vertex_Merge = 2,
        Intersect = 3
    };

    Type type;
    glm::vec3 position;
    SweepLineEventData data;

    SweepLineEvent(Type ty, glm::vec3 position) : type(ty), position(position), data() {}
    SweepLineEvent(Type ty, glm::vec3 position, SweepLineEventData data) : type(ty), position(position), data(data) {}

    // Note that > operator is used for "larger" queue priority, which is actually the "lower" event x values.
    // Ensures that lowest-x events are processed first in priority queue.
    bool operator>(const SweepLineEvent& rhs) const {
        return position.x < rhs.position.x || (position.x == rhs.position.x && position.y < rhs.position.y);
    }
    bool operator==(const SweepLineEvent& rhs) const {
        return (position.x == rhs.position.x) && (position.y == rhs.position.y);
    }
    bool operator<(const SweepLineEvent& rhs) const {
        return position.x > rhs.position.x || (position.x == rhs.position.x && position.y > rhs.position.y);
    }
};

#endif //SPHEREDRAW_SWEEPLINE_UTIL_H
