//
// Created by Nathan on 1/29/2026.
//

#ifndef SPHEREDRAW_QUADMAP_H
#define SPHEREDRAW_QUADMAP_H

#include <vector>
#include "drawable.h"
#include "sphere_point.h"
#include <cstdint>
#include <climits>
#include <set>
#include <queue>

using QuadMapId = uint32_t;
// Reserve max uint32 value for invalid ids, so we can only have uint32_max - 1 features
#define InvalidId UINT32_MAX
#define MaxFeatureCt 0xfffffffeU

class Quadmap {
    // Attributes/methods for assigning reference ids for external code
    std::set<QuadMapId> active_ids;
    std::deque<QuadMapId> deactivated_ids;
    QuadMapId lowest_unused_id = 0;
    uint32_t feature_count = 0;

    bool is_active_id(QuadMapId id);
    [[nodiscard]] bool is_full() const;
    [[nodiscard]] QuadMapId activate_new_id();
    void remove_id(QuadMapId id);

public:
    QuadMapId add_new_point(const SpherePoint& point);
    QuadMapId add_new_line();
    QuadMapId add_new_polygon();

};


#endif //SPHEREDRAW_QUADMAP_H
