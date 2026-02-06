//
// Created by Nathan on 2/5/2026.
//

#include "primitive_id_manager.h"
#include <climits>

std::set<unsigned int> PrimIDManager::used_ids = {};

bool PrimIDManager::id_is_used(unsigned int id) {
    return used_ids.contains(id);
}
bool PrimIDManager::add_id(unsigned int id) {
    if (id_is_used(id)) return false;
    used_ids.insert(id);
    return true;
}
bool PrimIDManager::remove_id(unsigned int id) {
    if (!id_is_used(id)) return false;
    used_ids.erase(id);
    return true;
}
unsigned int PrimIDManager::get_unused_id() {
    // Change later if this runs into performance issues.
    unsigned int guess = UINT_MAX - used_ids.size();
    guess = guess * guess * guess * guess + 1;
    while (used_ids.contains(guess)) {
        guess = guess * guess + 1;
    }
    return guess;
}