//
// Created by Nathan on 3/26/2026.
//

#include "sweepline_util.h"
#include "cubemap_util.h"
#include <algorithm>

float SweepSegment::sweep_x = std::nanf("");

// Get the intersection between line segments ab and pq on a sphere if it exists.
// Returns nullopt if the segments do not intersect. Will return nullopt if the vertices are joined (e.g., b == p)
// Assumes that a, b, p, q are already normalized.
std::optional<glm::vec3> get_sphere_line_intersection(const glm::vec3& A, const glm::vec3& B, const glm::vec3& P, const glm::vec3& Q) {
    // Check for vertex matching
    if (A == P || A == Q || B == P || B == Q) return std::nullopt;

    constexpr double TOLERANCE = 0.0000001f;
    // Get potential direction of intersection
    const auto dir = normalize(cross(cross(A,B),cross(P,Q)));
    // Tell if the direction of intersection is in the a-b range
    const double angle_ab = std::acos(std::clamp(dot(A, B), -1.0f, 1.0f));
    const double angle_adir = std::acos(std::clamp(dot(A, dir), -1.0f, 1.0f));
    const double angle_bdir = std::acos(std::clamp(dot(B, dir), -1.0f, 1.0f));
    bool in_ab_near = abs(angle_ab-angle_adir-angle_bdir) <= TOLERANCE;
    bool in_ab_far = 2.0*M_PI-angle_ab-angle_adir-angle_bdir <= TOLERANCE;

    // Tell if the direction of intersection is in the p-q range
    const double angle_pq = std::acos(std::clamp(dot(P, Q), -1.0f, 1.0f));
    const double angle_pdir = std::acos(std::clamp(dot(P, dir), -1.0f, 1.0f));
    const double angle_qdir = std::acos(std::clamp(dot(Q, dir), -1.0f, 1.0f));
    bool in_pq_near = abs(angle_pq-angle_pdir-angle_qdir) <= TOLERANCE;
    bool in_pq_far = 2.0*M_PI-angle_pq-angle_pdir-angle_qdir <= TOLERANCE;

    // Return the relevant intersect point
    bool intersect_near = in_ab_near && in_pq_near;
    bool intersect_far = in_ab_far && in_pq_far;
    if (intersect_near) return dir;
    if (intersect_far) return -dir;
    return std::nullopt;
}

inline bool assn_intersect(std::optional<glm::vec3>& intersection, const glm::vec3& A, const glm::vec3& B, FaceTransition transition_ty) {
    static float c = std::sqrt(1.0f / 3.0f);
    static std::map<FaceTransition, std::pair<glm::vec3, glm::vec3>> face_arcs {
            {N_to_W, {{-c, c, c}, {c, c, c}}}, // +/-x, +y, +z
            {W_to_N, {{-c, c, c}, {c, c, c}}},
            {N_to_M, {{c, c, -c}, {c, c, c}}},  // +x, +y, +/-z
            {M_to_N, {{c, c, -c}, {c, c, c}}},
            {N_to_E, {{-c, c, -c}, {c, c, -c}}},  // +/-x, +y, -z
            {E_to_N, {{-c, c, -c}, {c, c, -c}}},
            {N_to_A, {{-c, c, -c}, {-c, c, c}}},  // -x, +y, +/-z
            {A_to_N, {{-c, c, -c}, {-c, c, c}}},
            {W_to_M, {{c, -c, c}, {c, c, c}}}, // +x, +/-y, +z
            {M_to_W, {{c, -c, c}, {c, c, c}}},
            {W_to_A, {{-c, -c, c}, {-c, c, c}}}, // -x, +/-y, +z
            {A_to_W, {{-c, -c, c}, {-c, c, c}}},
            {W_to_S, {{-c, -c, c}, {c, -c, c}}}, // +/-x, -y, +z
            {S_to_W, {{-c, -c, c}, {c, -c, c}}},
            {M_to_E, {{c, -c, -c}, {c, c, -c}}}, // +x, +/-y, -z
            {E_to_M, {{c, -c, -c}, {c, c, -c}}},
            {M_to_S, {{c, -c, -c}, {c, -c, c}}}, // +x, -y, +/-z
            {S_to_M, {{c, -c, -c}, {c, -c, c}}},
            {E_to_A, {{-c, -c, -c}, {-c, c, -c}}}, // -x, +/-y, -z
            {A_to_E, {{-c, -c, -c}, {-c, c, -c}}},
            {E_to_S, {{-c, -c, -c}, {c, -c, -c}}}, // +/-x, -y, -z
            {S_to_E, {{-c, -c, -c}, {c, -c, -c}}},
            {A_to_S, {{-c, -c, -c}, {-c, -c, c}}}, // -x, -y, +/-z
            {S_to_A, {{-c, -c, -c}, {-c, -c, c}}},
    };

    return ((intersection = get_sphere_line_intersection(
            A, B, face_arcs.at(transition_ty).first, face_arcs.at(transition_ty).second)
                    ).has_value());
}

FaceCrossing compute_next_intersection(CubeFaceNum& curr_face, const glm::vec3& A, const glm::vec3& B) {
    std::optional<glm::vec3> intersection;
    FaceTransition transition_ty = No_Transition;
    switch(curr_face) {
        case North:
            transition_ty = N_to_W;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = West; break; }
            transition_ty = N_to_M;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = Meridian; break; }
            transition_ty = N_to_E;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = East; break; }
            transition_ty = N_to_A;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = AntiMeridian; break; }
            break;
        case West:
            transition_ty = W_to_N;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = North; break; }
            transition_ty = W_to_M;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = Meridian; break; }
            transition_ty = W_to_A;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = AntiMeridian; break; }
            transition_ty = W_to_S;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = South; break; }
            break;
        case Meridian:
            transition_ty = M_to_N;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = North; break; }
            transition_ty = M_to_W;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = West; break; }
            transition_ty = M_to_E;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = East; break; }
            transition_ty = M_to_S;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = South; break; }
            break;
        case East:
            transition_ty = E_to_N;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = North; break; }
            transition_ty = E_to_M;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = Meridian; break; }
            transition_ty = E_to_A;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = AntiMeridian; break; }
            transition_ty = E_to_S;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = South; break; }
            break;
        case AntiMeridian:
            transition_ty = A_to_N;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = North; break; }
            transition_ty = A_to_W;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = West; break; }
            transition_ty = A_to_E;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = East; break; }
            transition_ty = A_to_S;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = South; break; }
            break;
        case South:
            transition_ty = S_to_W;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = West; break; }
            transition_ty = S_to_M;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = Meridian; break; }
            transition_ty = S_to_E;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = East; break; }
            transition_ty = S_to_A;
            if (assn_intersect(intersection, A, B, transition_ty)) { curr_face = AntiMeridian; break; }
            break;
    }
    return {transition_ty, intersection.value()};
}

FaceCrossingData compute_face_crossings(const glm::vec3& A, const glm::vec3& B) {
    auto face_A = get_face(A);
    auto face_B = get_face(B);
    if (face_A == face_B) return {};
    auto curr_face = face_A;
    FaceCrossingData data;
    while (curr_face != face_B) {
        data.crossings[data.crossing_count] = compute_next_intersection(curr_face, A, B);
        data.crossing_count += 1;
    }
    return data;
}