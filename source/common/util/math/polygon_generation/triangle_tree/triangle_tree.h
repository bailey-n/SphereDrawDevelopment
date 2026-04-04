//
// Created by Nathan on 4/1/2026.
//

#ifndef TRIANGLE_TREE_H
#define TRIANGLE_TREE_H

#include "opengl_include.h"
#include <optional>
#include <climits>
#include <set>
#include <queue>

#include "sweepline_util.h"

class TriangleNode {
    unsigned int id;
    glm::vec3 vertices[3];
    glm::mat3x3 containment_check_matrix;
    bool degenerate;
    bool is_root;

    struct Edge {
        unsigned char segment_from_idx;
        unsigned char segment_to_idx;
        unsigned int neighbor_id;
    };

    Edge left_child_edge;
    Edge right_child_edge;
    Edge parent_edge;
    Edge& bottom_child_edge = parent_edge;


public:
    TriangleNode(unsigned int id, glm::u32vec3 vtx_idxs, glm::vec3 a, glm::vec3 b, glm::vec3 c, bool make_root = false) :
    id(id), vertices({a, b, c}), containment_check_matrix({a, b, c}),
        degenerate(glm::abs(glm::determinant(containment_check_matrix)) < CHECK_TRESHHOLD), is_root(make_root),
        left_child_edge(vtx_idxs.x, vtx_idxs.y, UINT32_MAX),
        right_child_edge(vtx_idxs.y, vtx_idxs.z, UINT32_MAX),
        parent_edge(vtx_idxs.z, vtx_idxs.x, UINT32_MAX) {
        if (!degenerate) containment_check_matrix = glm::inverse(containment_check_matrix);
    }

    [[nodiscard]] bool contains(glm::vec3 p) const {
        if (degenerate) return false;
        glm::vec3 transform = containment_check_matrix * p;
        return (transform.x > 0.0f) && (transform.y > 0.0f) && (transform.z > 0.0f);
    }

    struct TriangleSegmentIntersectionData {
        unsigned char count = 0;
        struct Intersection {
            Edge edge;
            std::optional<glm::vec3> position;
        };
        std::optional<Intersection> intersections[3];

        explicit operator bool() const { return (count > 0); }
    };

    [[nodiscard]] TriangleSegmentIntersectionData intersect_segment(glm::vec3 A, glm::vec3 B) const {
        std::optional<glm::vec3> intersection;
        TriangleSegmentIntersectionData data;
        if ((intersection = get_sphere_line_intersection(
            vertices[parent_edge.segment_from_idx], vertices[parent_edge.segment_to_idx], A, B
            )).has_value()) {
            data.intersections[data.count].emplace(parent_edge, intersection);
            data.count++;
        }
        if ((intersection = get_sphere_line_intersection(
            vertices[left_child_edge.segment_from_idx], vertices[left_child_edge.segment_to_idx], A, B
            )).has_value()) {
            data.intersections[data.count].emplace(left_child_edge, intersection);
            data.count++;
        }
        if ((intersection = get_sphere_line_intersection(
            vertices[right_child_edge.segment_from_idx], vertices[right_child_edge.segment_to_idx], A, B
            )).has_value()) {
            data.intersections[data.count].emplace(right_child_edge, intersection);
            data.count++;
        }
        return data;
    }

    [[nodiscard]] bool is_leaf() const {
        return (left_child_edge.neighbor_id == UINT32_MAX) && (right_child_edge.neighbor_id == UINT32_MAX) &&
            (!is_root || (bottom_child_edge.neighbor_id == UINT32_MAX));
    }
    [[nodiscard]] bool is_degenerate() const {
        return degenerate;
    }

    bool operator<(const TriangleNode& rhs) const { return (this->id < rhs.id); }
    bool operator>(const TriangleNode& rhs) const { return (this->id > rhs.id); }
    bool operator==(const TriangleNode& rhs) const { return (this->id == rhs.id); }

    void add_right_child(unsigned int id) { right_child_edge.neighbor_id = id; }
    void add_left_child(unsigned int id) { left_child_edge.neighbor_id = id; }
    void add_bottom_child(unsigned int id) { bottom_child_edge.neighbor_id = id; }
    void add_parent(unsigned int id) { parent_edge.neighbor_id = id; }
};


class TriangleTree {
    std::vector<TriangleNode> nodes;
public:
    TriangleTree(const std::vector<glm::vec3>& polyline_vertices) {
        if (polyline_vertices.size() < 3) return;
        nodes.reserve(polyline_vertices.size()-2);

        // Generate triangle indices by subdividing segments
        size_t n = polyline_vertices.size();

        glm::u32vec3 first_triangle_idxs {0, n/3, (2*n)/3};
        nodes.emplace_back(0, first_triangle_idxs,
            polyline_vertices[first_triangle_idxs.x],
            polyline_vertices[first_triangle_idxs.y],
            polyline_vertices[first_triangle_idxs.z], true
            );

        std::queue<glm::u32vec2> segment_subdiv_queue;
        // We insert backwards for index consistency; the last segment will be the first to gain a new child
        segment_subdiv_queue.emplace(first_triangle_idxs.z, n);
        segment_subdiv_queue.emplace(first_triangle_idxs.y, first_triangle_idxs.z);
        segment_subdiv_queue.emplace(first_triangle_idxs.x, first_triangle_idxs.y);

        while (!segment_subdiv_queue.empty()) {
            glm::u32vec2 curr_segment = segment_subdiv_queue.front();
            segment_subdiv_queue.pop();
            if ((curr_segment.y - curr_segment.x) < 2) continue;
            unsigned int new_idx = curr_segment.x + ((curr_segment.y - curr_segment.x) / 2);
            triangle_queue.emplace(curr_segment.x, new_idx, (curr_segment.y)%n); // TODO: force to be ccw
            segment_subdiv_queue.emplace(new_idx, curr_segment.y);
            segment_subdiv_queue.emplace(curr_segment.x, new_idx);
        }

        // Convert into a set of triangles
        // Add first triangle
        nodes.emplace_back(nodes.size(), first_triangle_idxs,
            polyline_vertices[first_triangle_idxs.x],
            polyline_vertices[first_triangle_idxs.y],
            polyline_vertices[first_triangle_idxs.z]
            );
        triangle_queue.pop();
        if (triangle_queue.size() >= 1)

        while (!triangle_queue.empty()) {
            glm::u32vec3 triangle = triangle_queue.front();
            nodes.emplace_back(nodes.size(), triangle,
                polyline_vertices[triangle.x],
                polyline_vertices[triangle.y],
                polyline_vertices[triangle.z]
                );
        }
    }
};

class TriangleIntersector {
    glm::mat3x3 vertices_m;
    glm::mat3x3 containment_check_matrix;

    std::vector<TriangleIntersector> children;
    bool degenerate;
    bool intersect_parity; // EVEN=FALSE, ODD=TRUE

public:
    TriangleIntersector(glm::vec3 a, glm::vec3 b, glm::vec3 c, bool parity = false) : vertices_m(a, b, c), containment_check_matrix(),
    degenerate(glm::abs(glm::determinant(containment_check_matrix)) < CHECK_TRESHHOLD), intersect_parity(parity) {
        if (!degenerate) containment_check_matrix = glm::inverse(containment_check_matrix);
    }

    [[nodiscard]] bool is_leaf_intersector() { return children.empty(); }

    [[nodiscard]] bool viewable_from(const glm::vec3& p, int v_idx) {
        return !get_sphere_line_intersection(p, vertices_m[v_idx], vertices_m[(v_idx+1)%3], vertices_m[(v_idx+2)%3]).has_value();
    }

    [[nodiscard]] bool contains_p(const glm::vec3& p) {
        if (degenerate) return false;
        glm::vec3 containment_vec = containment_check_matrix * p;
        return (containment_vec.x > CHECK_TRESHHOLD && containment_vec.y > CHECK_TRESHHOLD && containment_vec.z > CHECK_TRESHHOLD);
    }

    void intersect(TriangleIntersector& other) {
        // TODO: Make system to reduce redundant shared children
        // Early return if either is degenerate (no point in going further)
        if (degenerate || other.degenerate) return;

        // Test for any interaction. If none, then nothing to do. Note that even if they have children,
        // No interactions between the parents implies the children also cannot interact.
        // TODO: switch 0.0f to appropriate small threshold
        glm::mat3x3 this_containment_matrix = this->containment_check_matrix * other.vertices_m;
        bool this_contains_other_a = (this_containment_matrix[0].x > 0.0f) && (this_containment_matrix[0].y > 0.0f) && (this_containment_matrix[0].z > 0.0f);
        bool this_contains_other_b = (this_containment_matrix[1].x > 0.0f) && (this_containment_matrix[1].y > 0.0f) && (this_containment_matrix[1].z > 0.0f);
        bool this_contains_other_c = (this_containment_matrix[2].x > 0.0f) && (this_containment_matrix[2].y > 0.0f) && (this_containment_matrix[2].z > 0.0f);

        glm::mat3x3 other_containment_matrix = other.containment_check_matrix * this->vertices_m;
        bool other_contains_this_a = (other_containment_matrix[0].x > 0.0f) && (other_containment_matrix[0].y > 0.0f) && (other_containment_matrix[0].z > 0.0f);
        bool other_contains_this_b = (other_containment_matrix[1].x > 0.0f) && (other_containment_matrix[1].y > 0.0f) && (other_containment_matrix[1].z > 0.0f);
        bool other_contains_this_c = (other_containment_matrix[2].x > 0.0f) && (other_containment_matrix[2].y > 0.0f) && (other_containment_matrix[2].z > 0.0f);

        bool contains_other_vtxs = this_contains_other_a || this_contains_other_b || this_contains_other_c;
        bool other_contains_vtxs = other_contains_this_a || other_contains_this_b || other_contains_this_c;

        auto intersect_00 = get_sphere_line_intersection(vertices_m[0], vertices_m[1], other.vertices_m[0], other.vertices_m[1]);
        auto intersect_01 = get_sphere_line_intersection(vertices_m[0], vertices_m[1], other.vertices_m[1], other.vertices_m[2]);
        auto intersect_02 = get_sphere_line_intersection(vertices_m[0], vertices_m[1], other.vertices_m[2], other.vertices_m[0]);
        auto intersect_10 = get_sphere_line_intersection(vertices_m[1], vertices_m[2], other.vertices_m[0], other.vertices_m[1]);
        auto intersect_11 = get_sphere_line_intersection(vertices_m[1], vertices_m[2], other.vertices_m[1], other.vertices_m[2]);
        auto intersect_12 = get_sphere_line_intersection(vertices_m[1], vertices_m[2], other.vertices_m[2], other.vertices_m[0]);
        auto intersect_20 = get_sphere_line_intersection(vertices_m[2], vertices_m[0], other.vertices_m[0], other.vertices_m[1]);
        auto intersect_21 = get_sphere_line_intersection(vertices_m[2], vertices_m[0], other.vertices_m[1], other.vertices_m[2]);
        auto intersect_22 = get_sphere_line_intersection(vertices_m[2], vertices_m[0], other.vertices_m[2], other.vertices_m[0]);

        int intersect_count =
            (int)intersect_00.has_value() + (int)intersect_01.has_value() + (int)intersect_02.has_value() +
            (int)intersect_10.has_value() + (int)intersect_11.has_value() + (int)intersect_12.has_value() +
            (int)intersect_20.has_value() + (int)intersect_21.has_value() + (int)intersect_22.has_value();
        bool intersections_exist = intersect_count > 0;

        if (!(intersections_exist || contains_other_vtxs || other_contains_vtxs)) return;

        // Some interaction exists. If this or other has children, break them down instead.
        if (!this->is_leaf_intersector()) {
            for (auto& child: children) child.intersect(other);
            return;
        }
        if (!other.is_leaf_intersector()) {
            for (auto& child: other.children) child.intersect(*this);
            return;
        }

        // Both this and other are leaves, and they do in fact interact some.
        if ((!intersections_exist) && contains_other_vtxs) { // other is inside this
            this->subdivide_with_contained_intersector(other);
            return;
        }
        if ((!intersections_exist) && other_contains_vtxs) {
            // Other completely contains this
            other.subdivide_with_contained_intersector(*this);
            return;
        }

        // Intersections exist. Find one and orient triangle to reduce case load.
        // Determine which of the sub-triangulating cases we require and determine a triangulation
        int this_containment_count = (int)this_contains_other_a + (int)this_contains_other_b + (int)this_contains_other_c;
        int other_containment_count = (int)other_contains_this_a + (int)other_contains_this_b + (int)other_contains_this_c;

        int this01_intersect_count = (int)(intersect_00.has_value()) + (int)(intersect_01.has_value()) + (int)(intersect_02.has_value());
        int this12_intersect_count = (int)(intersect_10.has_value()) + (int)(intersect_11.has_value()) + (int)(intersect_12.has_value());
        int this20_intersect_count = (int)(intersect_20.has_value()) + (int)(intersect_21.has_value()) + (int)(intersect_22.has_value());

        int other01_intersect_count = (int)(intersect_00.has_value()) + (int)(intersect_10.has_value()) + (int)(intersect_20.has_value());
        int other12_intersect_count = (int)(intersect_01.has_value()) + (int)(intersect_11.has_value()) + (int)(intersect_21.has_value());
        int other20_intersect_count = (int)(intersect_02.has_value()) + (int)(intersect_12.has_value()) + (int)(intersect_22.has_value());

        // TODO: Use the indexable variables as the actual values
        std::optional<glm::vec3> intersect_[3][3] = {
            {intersect_00, intersect_01, intersect_02},
            {intersect_10, intersect_11, intersect_12},
            {intersect_20, intersect_21, intersect_22}
        };

        int A, B, C, X, Y, Z;
        int AB, BC, CA, XY, YZ, ZX;

        // CASE 0/0/4
        if ((this_containment_count == 0) && (other_containment_count == 0) && (intersect_count == 4)) {
            // Orient this as ABC and other as XYZ, with extra case reduction.
            B = (this01_intersect_count == 0) ? 2 : (this12_intersect_count == 0) ? 0 : 1;
            C = (B+1)%3;
            A = (B+2)%3;

            Y = (other01_intersect_count == 0) ? 2 : (other12_intersect_count == 0) ? 0 : 1;
            Z = (Y+1)%3;
            X = (Y+2)%3;

            AB = A;
            BC = B;
            CA = C;
            XY = X;
            YZ = Y;
            ZX = Z;

            auto potential_intersection_1 = get_sphere_line_intersection(this->vertices_m[C], intersect_[AB][XY].value(), other.vertices_m[Y], other.vertices_m[Z]);
            auto potential_intersection_2 = get_sphere_line_intersection(other.vertices_m[Z], intersect_[AB][XY].value(), this->vertices_m[B], this->vertices_m[C]);
            if (potential_intersection_1.has_value()) {
                std::swap(X, Z);
                XY = Y;
                YZ = Z;
                ZX = X;
            }
            if (potential_intersection_2.has_value()) {
                std::swap(A, C);
                AB = B;
                BC = C;
                CA = A;
            }

            // Produce Children
            this->children.emplace_back(this->vertices_m[C], this->vertices_m[A], intersect_[AB][XY].value(), intersect_parity);
            this->children.emplace_back(this->vertices_m[C], intersect_[AB][XY].value(), intersect_[BC][XY].value(), intersect_parity);
            this->children.emplace_back(this->vertices_m[B], intersect_[AB][YZ].value(), intersect_[BC][YZ].value(), intersect_parity);

            other.children.emplace_back(other.vertices_m[Z], other.vertices_m[X], intersect_[AB][XY].value(), other.intersect_parity);
            other.children.emplace_back(other.vertices_m[Z], intersect_[AB][XY].value(), intersect_[AB][YZ].value(), other.intersect_parity);
            other.children.emplace_back(other.vertices_m[Y], intersect_[BC][YZ].value(), intersect_[BC][XY].value(), other.intersect_parity);

            this->children.emplace_back(intersect_[AB][XY].value(), intersect_[BC][XY].value(), intersect_[BC][YZ].value(), !intersect_parity);
            this->children.emplace_back(intersect_[AB][XY].value(), intersect_[BC][YZ].value(), intersect_[AB][YZ].value(), !intersect_parity);
            other.children.emplace_back(intersect_[AB][XY].value(), intersect_[BC][XY].value(), intersect_[BC][YZ].value(), !other.intersect_parity);
            other.children.emplace_back(intersect_[AB][XY].value(), intersect_[BC][YZ].value(), intersect_[AB][YZ].value(), !other.intersect_parity);
        }

        // CASE 0/0/6
        else if ((this_containment_count == 0) && (other_containment_count == 0) && (intersect_count == 6)) {
            // Orient this as ABC and other as XYZ.
            A = 0;
            B = 1;
            C = 2;

            AB = A;
            BC = B;
            CA = C;

            YZ = (!intersect_[AB][0].has_value()) ? 0 : (!intersect_[AB][1].has_value()) ? 1 : 2;
            XY = (!intersect_[BC][0].has_value()) ? 0 : (!intersect_[BC][1].has_value()) ? 1 : 2;
            ZX = (!intersect_[CA][0].has_value()) ? 0 : (!intersect_[CA][1].has_value()) ? 1 : 2;

            X = (5-XY-ZX)%3;
            Y = (5-XY-YZ)%3;
            Z = (5-YZ-ZX)%3;

            // Produce children
            this->children.emplace_back(this->vertices_m[A], intersect_[AB][XY].value(), intersect_[CA][XY].value(), this->intersect_parity);
            this->children.emplace_back(this->vertices_m[B], intersect_[BC][ZX].value(), intersect_[AB][ZX].value(), this->intersect_parity);
            this->children.emplace_back(this->vertices_m[C], intersect_[CA][YZ].value(), intersect_[BC][YZ].value(), this->intersect_parity);

            other.children.emplace_back(other.vertices_m[X], intersect_[AB][XY].value(), intersect_[AB][ZX].value(), other.intersect_parity);
            other.children.emplace_back(other.vertices_m[Y], intersect_[CA][YZ].value(), intersect_[CA][XY].value(), other.intersect_parity);
            other.children.emplace_back(other.vertices_m[Z], intersect_[BC][ZX].value(), intersect_[BC][YZ].value(), other.intersect_parity);

            this->children.emplace_back(intersect_[AB][ZX].value(), intersect_[BC][YZ].value(), intersect_[CA][XY].value(), !this->intersect_parity);
            this->children.emplace_back(intersect_[AB][ZX].value(), intersect_[CA][XY].value(), intersect_[AB][XY].value(), !this->intersect_parity);
            this->children.emplace_back(intersect_[BC][YZ].value(), intersect_[AB][ZX].value(), intersect_[BC][ZX].value(), !this->intersect_parity);
            this->children.emplace_back(intersect_[CA][XY].value(), intersect_[BC][YZ].value(), intersect_[CA][YZ].value(), !this->intersect_parity);
            other.children.emplace_back(intersect_[AB][ZX].value(), intersect_[BC][YZ].value(), intersect_[CA][XY].value(), !other.intersect_parity);
            other.children.emplace_back(intersect_[AB][ZX].value(), intersect_[CA][XY].value(), intersect_[AB][XY].value(), !other.intersect_parity);
            other.children.emplace_back(intersect_[BC][YZ].value(), intersect_[AB][ZX].value(), intersect_[BC][ZX].value(), !other.intersect_parity);
            other.children.emplace_back(intersect_[CA][XY].value(), intersect_[BC][YZ].value(), intersect_[CA][YZ].value(), !other.intersect_parity);
        }

        // CASE 1/0/2
        else if ((this_containment_count == 1) && (other_containment_count == 0) && (intersect_count == 2)) {
            // Orient this as ABC and other as XYZ.
            AB = (this01_intersect_count == 2) ? 0 : (this12_intersect_count == 2) ? 1 : 2;
            BC = (AB+1)%3;
            CA = (AB+2)%3;

            A = AB;
            B = BC;
            C = CA;

            Y = this_contains_other_a ? 0 : this_contains_other_b ? 1 : 2;
            Z = (Y+1)%3;
            X = (Y+2)%3;

            XY = X;
            YZ = Y;
            ZX = Z;

            auto potential_intersection = get_sphere_line_intersection(this->vertices_m[A], intersect_[AB][XY].value(), other.vertices_m[Y], other.vertices_m[Z]);
            if (potential_intersection.has_value()) {
                std::swap(X, Z);
                XY = Y;
                YZ = Z;
                ZX = X;
            }

            // Produce children (technically two non-isomorphic forms here, but both triangulations will be valid).
            this->children.emplace_back(this->vertices_m[A], other.vertices_m[Y], intersect_[AB][XY].value(), intersect_parity);
            this->children.emplace_back(this->vertices_m[C], this->vertices_m[A], other.vertices_m[Y], intersect_parity);
            this->children.emplace_back(this->vertices_m[B], this->vertices_m[C], other.vertices_m[Y], intersect_parity);
            this->children.emplace_back(this->vertices_m[B], other.vertices_m[Y], intersect_[AB][YZ].value(), intersect_parity);

            other.children.emplace_back(other.vertices_m[Z], other.vertices_m[X], intersect_[AB][YZ].value(), other.intersect_parity);
            other.children.emplace_back(other.vertices_m[X], intersect_[AB][XY].value(), intersect_[AB][YZ].value(), other.intersect_parity);

            this->children.emplace_back(other.vertices_m[Y], intersect_[AB][YZ].value(), intersect_[AB][XY].value(), !intersect_parity);
            this->children.emplace_back(other.vertices_m[Y], intersect_[AB][YZ].value(), intersect_[AB][XY].value(), !other.intersect_parity);
        }
        else if ((this_containment_count == 0) && (other_containment_count == 1) && (intersect_count == 2)) {
            // Orient this as XYZ and other as ABC.
        }

        // CASE 1/0/4
        else if ((this_containment_count == 1) && (other_containment_count == 0) && (intersect_count == 4)) {
            // Orient this as ABC and other as XYZ, with extra case reduction.
        }
        else if ((this_containment_count == 0) && (other_containment_count == 1) && (intersect_count == 4)) {
            // Orient this as XYZ and other as ABC, with extra case reduction.
        }

        // CASE 1/1/2
        else if ((this_containment_count == 1) && (other_containment_count == 1) && (intersect_count == 2)) {
            // Orient this as ABC and other as XYZ.
        }

        // CASE 1/1/4
        else if ((this_containment_count == 1) && (other_containment_count == 1) && (intersect_count == 4)) {
            // Orient this as ABC and other as XYZ, with extra case reduction
        }

        // CASE 2/0/2
        else if ((this_containment_count == 2) && (other_containment_count == 0) && (intersect_count == 2)) {
            // Orient this as ABC and other as XYZ.

        }
        else if ((this_containment_count == 0) && (other_containment_count == 2) && (intersect_count == 2)) {
            // Orient this as XYZ and other as ABC.

        }

        // CASE 2/1/2
        else if ((this_containment_count == 2) && (other_containment_count == 1) && (intersect_count == 2)) {
            // Orient this as ABC and other as XYZ.

        }
        else if ((this_containment_count == 1) && (other_containment_count == 2) && (intersect_count == 2)) {
            // Orient this as XYZ and other as ABC.

        }

        // Shouldn't be possible
        else {
            throw std::exception();
        }
    }

    void subdivide_with_contained_intersector(TriangleIntersector& other) {
        other.intersect_parity = !other.intersect_parity; // Invert other's parity

        auto this0_sees_other0 = other.viewable_from(this->vertices_m[0], 0);
        auto this0_sees_other1 = other.viewable_from(this->vertices_m[0], 1);
        auto this0_sees_other2 = other.viewable_from(this->vertices_m[0], 2);
        auto this1_sees_other0 = other.viewable_from(this->vertices_m[1], 0);
        auto this1_sees_other1 = other.viewable_from(this->vertices_m[1], 1);
        auto this1_sees_other2 = other.viewable_from(this->vertices_m[1], 2);
        auto this2_sees_other0 = other.viewable_from(this->vertices_m[2], 0);
        auto this2_sees_other1 = other.viewable_from(this->vertices_m[2], 1);
        auto this2_sees_other2 = other.viewable_from(this->vertices_m[2], 2);

        int other0_connectivity = (int)this0_sees_other0 + (int)this1_sees_other0 + (int)this2_sees_other0;
        int other1_connectivity = (int)this0_sees_other1 + (int)this1_sees_other1 + (int)this2_sees_other1;
        int other2_connectivity = (int)this0_sees_other2 + (int)this1_sees_other2 + (int)this2_sees_other2;

        // There is an inner vertex which only sees 1 outer vertex.
        if ((other0_connectivity == 1) || (other1_connectivity == 1) || (other2_connectivity == 1)) {
            // identify isolated vertex and which outer vertex it connects to
            int isolated_other_idx = (other0_connectivity == 1) ? 0 : (other1_connectivity == 1) ? 1 : 2;
            int connected_idx = UINT32_MAX;
            switch (isolated_other_idx) {
                case 0: connected_idx = this0_sees_other0 ? 0 : this1_sees_other0 ? 1 : 2; break;
                case 1: connected_idx = this0_sees_other1 ? 0 : this1_sees_other1 ? 1 : 2; break;
                case 2: connected_idx = this0_sees_other2 ? 0 : this1_sees_other2 ? 1 : 2; break;
                default: break;
            }
            int& t0 = connected_idx;
            int& o0 = isolated_other_idx;
            // Attempt to connect the other two to each other. Start wwith t1-o1 and t2-o2, and hope they don't cross.
            int t1 = (connected_idx+1)%3;
            int t2 = (connected_idx+2)%3;
            int o1 = (isolated_other_idx+1)%3;
            int o2 = (isolated_other_idx+2)%3;

            auto potential_intersection = get_sphere_line_intersection(this->vertices_m[t1], other.vertices_m[o1], this->vertices_m[t2], other.vertices_m[o2]);
            if (potential_intersection.has_value()) std::swap(t1, t2);

            // Add children now that we can determine valid triangles
            children.emplace_back(this->vertices_m[t0], other.vertices_m[o0], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t0], this->vertices_m[t1], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t0], other.vertices_m[o2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], this->vertices_m[t0], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t2], other.vertices_m[o1], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t1], this->vertices_m[t2], other.vertices_m[o1], intersect_parity);
            return;
        }

        // Each inner vertex can see at least two outer vertices.
        int t0, t1, t2, o0, o1, o2;
        int total_connectivity = other0_connectivity + other1_connectivity + other2_connectivity;

        if (total_connectivity == 9) { // All can see all
            t0 = 0;
            t1 = 1;
            t2 = 2;
            o0 = 0;
            o1 = 1;
            o2 = 2;

            auto potential_intersection_1 = get_sphere_line_intersection(this->vertices_m[t0], other.vertices_m[o1], this->vertices_m[t2], other.vertices_m[o2]);
            auto potential_intersection_2 = get_sphere_line_intersection(this->vertices_m[t0], other.vertices_m[o0], this->vertices_m[t2], other.vertices_m[o2]);
            auto potential_intersection_3 = get_sphere_line_intersection(this->vertices_m[t1], other.vertices_m[o1], this->vertices_m[t0], other.vertices_m[o0]);
            auto potential_intersection_4 = get_sphere_line_intersection(this->vertices_m[t2], other.vertices_m[o2], this->vertices_m[t1], other.vertices_m[o1]);

            if (potential_intersection_1.has_value()) { // Rotates all values by 1
                o0 = 1;
                o1 = 2;
                o2 = 0;
            }
            else if (potential_intersection_2.has_value()) std::swap(o0, o2);
            else if (potential_intersection_3.has_value()) std::swap(o1, o0);
            else if (potential_intersection_4.has_value()) std::swap(o2, o1);

            // Now can guarantee that triangulation will work.
            children.emplace_back(this->vertices_m[t0], other.vertices_m[o0], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t0], this->vertices_m[t1], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t1], other.vertices_m[o1], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t1], this->vertices_m[t2], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t2], other.vertices_m[o2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], this->vertices_m[t0], other.vertices_m[o0], intersect_parity);
        }

        else if (total_connectivity == 8) { // Two can see all, one can see only two. Make them t0 and o0
            o0 = (other0_connectivity == 2) ? 0 : (other1_connectivity == 2) ? 1 : 2;
            o1 = (o0+1)%3;
            o2 = (o0+2)%3;

            switch (o0) {
            case 0: t0 = !this0_sees_other0 ? 0 : !this1_sees_other0 ? 1 : 2; break;
            case 1: t0 = !this0_sees_other1 ? 0 : !this1_sees_other1 ? 1 : 2; break;
            case 2: t0 = !this0_sees_other2 ? 0 : !this1_sees_other2 ? 1 : 2; break;
            default: t0 = UINT32_MAX;
            }
            t1 = (t0+1)%3;
            t2 = (t0+2)%3;

            // Check if t1 can connect to o1 and t2 to o2. If not, swap them.
            auto potential_intersection_1 = get_sphere_line_intersection(this->vertices_m[t1], other.vertices_m[o0], this->vertices_m[t2], other.vertices_m[o2]);
            auto potential_intersection_2 = get_sphere_line_intersection(this->vertices_m[t2], other.vertices_m[o0], this->vertices_m[t1], other.vertices_m[o1]);
            if (potential_intersection_1.has_value() || potential_intersection_2.has_value()) std::swap(t1, t2);

            // Now we can guarantee this triangulation
            children.emplace_back(this->vertices_m[t0], other.vertices_m[o1], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t0], this->vertices_m[t1], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t1], other.vertices_m[o0], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t1], this->vertices_m[t2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], other.vertices_m[o2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], this->vertices_m[t0], other.vertices_m[o2], intersect_parity);
        }

        else if (total_connectivity == 7) { // Two can see only two, one can see all. Say o2 sees all.
            o2 = (other0_connectivity == 3) ? 0 : (other1_connectivity == 3) ? 1 : 2;
            o0 = (o2+1)%3;
            o1 = (o2+2)%3;

            switch (o0) { // o0 cannot see t0
            case 0: t0 = !this0_sees_other0 ? 0 : !this1_sees_other0 ? 1 : 2; break;
            case 1: t0 = !this0_sees_other1 ? 0 : !this1_sees_other1 ? 1 : 2; break;
            case 2: t0 = !this0_sees_other2 ? 0 : !this1_sees_other2 ? 1 : 2; break;
            default: t0 = UINT32_MAX;
            }
            switch (o1) { // o1 cannot see t1
            case 0: t1 = !this0_sees_other0 ? 0 : !this1_sees_other0 ? 1 : 2; break;
            case 1: t1 = !this0_sees_other1 ? 0 : !this1_sees_other1 ? 1 : 2; break;
            case 2: t1 = !this0_sees_other2 ? 0 : !this1_sees_other2 ? 1 : 2; break;
            default: t1 = UINT32_MAX;
            }
            // t2 is the other one. Can see all.
            t2 = 3-t0-t1;

            children.emplace_back(this->vertices_m[t0], other.vertices_m[o2], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t0], this->vertices_m[t1], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t1], other.vertices_m[o2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t1], this->vertices_m[t2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], other.vertices_m[o0], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t2], this->vertices_m[t0], other.vertices_m[o1], intersect_parity);
        }

        else { // Total_connectivity == 6. Each o_i does not see a unique t_j.
            o0 = 0;
            o1 = 1;
            o2 = 2;
            t0 = !this0_sees_other0 ? 0 : !this1_sees_other0 ? 1 : 2;
            t1 = !this0_sees_other1 ? 0 : !this1_sees_other1 ? 1 : 2;
            t2 = !this0_sees_other2 ? 0 : !this1_sees_other2 ? 1 : 2;

            // This is enough to determine valid triangulation.
            children.emplace_back(this->vertices_m[t0], other.vertices_m[o2], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t0], this->vertices_m[t1], other.vertices_m[o2], intersect_parity);
            children.emplace_back(this->vertices_m[t1], other.vertices_m[o2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t1], this->vertices_m[t2], other.vertices_m[o0], intersect_parity);
            children.emplace_back(this->vertices_m[t2], other.vertices_m[o0], other.vertices_m[o1], intersect_parity);
            children.emplace_back(this->vertices_m[t2], this->vertices_m[t0], other.vertices_m[o1], intersect_parity);
        }
    }
};



#endif //TRIANGLE_TREE_H
