//
// Created by Nathan on 3/30/2026.
//

#ifndef LINELOOP_SEGMENT_H
#define LINELOOP_SEGMENT_H

#include "opengl_include.h"
#include <vector>
#include <climits>

struct LLSegmentId {
    unsigned int prev_id = UINT32_MAX;
    unsigned int next_id = UINT32_MAX;
};

struct LLSegmentIntersection {
    glm::vec3 pos;
    unsigned int pos_id;
    LLSegmentId segment_1;
    LLSegmentId segment_2;
};

struct LinkedLineLoopSegment {
    LLSegmentId id;
    LLSegmentId prev_segment;
    LLSegmentId next_segment;
};

class BaseLineLoopSegment {
    static std::vector<glm::vec3>* positions;
    LLSegmentId id;
    std::vector<LLSegmentIntersection> intersections;

public:
    void try_intersect(const BaseLineLoopSegment& other);
    void sort_intersections();
    std::vector<LinkedLineLoopSegment> decompose();
};

#endif //LINELOOP_SEGMENT_H
