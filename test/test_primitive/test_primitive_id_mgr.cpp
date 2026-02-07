//
// Created by Nathan on 2/6/2026.
//

#include "test_primitive_id_mgr.h"
#include "common/geometry/drawing/primitive_id_manager.h"
#include <queue>
#include <set>
#include <iostream>
#include "timer.h"

unsigned int generate_new_id() {
    unsigned int new_id = PrimIDManager::get_unused_id();
    PrimIDManager::add_id(new_id);
    return new_id;
}
void id_generator_speed_test() {
    Timer::start();
    constexpr unsigned int NUM_TESTS = 1000000;
    for (int i = 0; i < NUM_TESTS; i++) {
        PrimIDManager::add_id(PrimIDManager::get_unused_id());
    }
    Timer::end(4, "");
    // Previous result: 1240ms/million
}
bool test_id_funcs() {
    constexpr unsigned int NUM_TESTS = 100000;
    std::queue<unsigned int> ids;
    for (int i = 0; i < NUM_TESTS; i++) {
        unsigned int id = PrimIDManager::get_unused_id();
        if (!PrimIDManager::add_id(id)) return false;
        if (!PrimIDManager::id_is_used(id)) return false;
        ids.push(id);
    }
    for (int i = 0; i < NUM_TESTS; i++) {
        unsigned int id = ids.front();
        if (!PrimIDManager::remove_id(id)) return false;
        if (PrimIDManager::id_is_used(id)) return false;
        ids.pop();
    }
    // id_generator_speed_test();
    return true;
}