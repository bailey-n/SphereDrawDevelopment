//
// Created by Nathan on 2/5/2026.
//

#ifndef SPHEREDRAW_PRIMITIVE_ID_MANAGER_H
#define SPHEREDRAW_PRIMITIVE_ID_MANAGER_H

#include <set>
#include <random>

class PrimIDManager {
    static std::set<unsigned int> used_ids;
    static std::mt19937 generator;
    static std::uniform_int_distribution<unsigned int> id_range;
    static unsigned int id_randomizer_time;
    static unsigned int id_randomizer_rng;
public:
    static bool id_is_used(unsigned int id);
    static bool add_id(unsigned int id);
    static bool remove_id(unsigned int id);
    static unsigned int get_unused_id();
};


#endif //SPHEREDRAW_PRIMITIVE_ID_MANAGER_H
