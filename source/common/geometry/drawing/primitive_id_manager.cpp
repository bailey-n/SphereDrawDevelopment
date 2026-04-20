//
// Created by Nathan on 2/5/2026.
//

#include "primitive_id_manager.h"
#include <climits>
#include <chrono>
#include <iostream>

std::set<unsigned int> PrimIDManager::used_ids = {};
std::mt19937 PrimIDManager::generator((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());
std::uniform_int_distribution<unsigned int> PrimIDManager::id_range(0u, UINT32_MAX-1u);
unsigned int PrimIDManager::id_randomizer_time = (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count();
unsigned int PrimIDManager::id_randomizer_rng = PrimIDManager::id_range(PrimIDManager::generator);

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

//Clears all used_ids
void PrimIDManager::reset() {
    used_ids.clear();
}

unsigned int PrimIDManager::get_unused_id() {
    // Change later if this runs into performance issues.
    unsigned int guess = (UINT_MAX>>1) - used_ids.size();
    guess ^= id_randomizer_rng;
    id_randomizer_rng *= id_randomizer_time;
    if (used_ids.contains(guess)) {
        id_randomizer_time = (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count();
        while (used_ids.contains(guess)) {
            // std::cout << "Re-Guessing..." << std::endl;
            guess = 2*guess + 1;
        }
    }
    return guess;
}