//
// Created by Nathan on 3/30/2026.
//

#include "lineloop_segment.h"
#include "cubemap_util.h"
#include "sweepline_util.h"

std::vector<glm::vec3>* BaseLineLoopSegment::positions = nullptr;

void BaseLineLoopSegment::try_intersect(const BaseLineLoopSegment& other) {
    if (!positions) return;
    auto intersect = get_sphere_line_intersection(
        positions->at(this->id.prev_id), positions->at(this->id.next_id),
        positions->at(other.id.prev_id), positions->at(other.id.next_id)
        );
    if (intersect.has_value()) {
        intersections.emplace_back(intersect.value(), (unsigned int)positions->size(), this->id, other.id);
        positions->emplace_back(intersect.value());
    }
}

void BaseLineLoopSegment::sort_intersections() {
    // auto temp_positions = positions;
    // std::sort(intersections.begin(), intersections.end(),
    //     [temp_positions](const LLSegmentIntersection& lhs, const LLSegmentIntersection& rhs) {
    //         glm::vec3 starting_pos = temp_positions->at(lhs.segment_1.prev_id);
    //     glm::dot(starting_pos, lhs.pos) > glm::dot(starting_pos, rhs.pos);
    // });
}
