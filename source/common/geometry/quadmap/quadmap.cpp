//
// Created by Nathan on 1/29/2026.
//

#include "quadmap.h"
#include <iostream>

bool Quadmap::is_active_id(QuadMapId id) {
    return active_ids.contains(id);
}

bool Quadmap::is_full() const {
    return (feature_count == MaxFeatureCt);
}

QuadMapId Quadmap::activate_new_id() {
    if (is_full()) return InvalidId;

    // temp variable to store old lowest_unused_id (which will become active)
    QuadMapId new_id = lowest_unused_id;
    active_ids.insert(new_id);

    // Set new lowest_unused_id
    feature_count++;
    if (deactivated_ids.empty()) { lowest_unused_id = feature_count; }
    else {
        // Ensures that any holes from deactivation are filled first
        lowest_unused_id = deactivated_ids.front();
        deactivated_ids.pop_front();
    }
    return new_id;
}

void Quadmap::remove_id(QuadMapId id) {
    // Safety check
    if (!is_active_id(id)) {
        std::cerr << "Attempted to remove quadmap object with id " << id << ", but id is not active" << std::endl;
        return;
    }
    active_ids.erase(id);
    feature_count--;

    if (deactivated_ids.empty() || (id < lowest_unused_id)) {
        deactivated_ids.push_front(id);
        lowest_unused_id = id;
        return;
    }

    // Deactivated id list is maintained to ensure that any id holes are filled
    // TODO: Make into binary search if performance is a problem
    auto curr = deactivated_ids.begin()+1;
    while (curr != deactivated_ids.end()) {
        if (id == *curr) {
            std::cerr << "Attempted to add deactivated quadmap object with id " << id << " to deactivated object list, but that id is already in the list" << std::endl;
            return;
        }
        if (id < *curr) {
            break;
        }
        ++curr;
    }
    deactivated_ids.insert(curr, id);
}

QuadMapId Quadmap::add_new_point(const SpherePoint& point) {

}