//
// Created by Nathan on 2/5/2026.
//

#ifndef SPHEREDRAW_PRIMITIVE_ID_MANAGER_H
#define SPHEREDRAW_PRIMITIVE_ID_MANAGER_H

#include <set>

class PrimIDManager {
    static std::set<unsigned int> used_ids;
public:
    static bool id_is_used(unsigned int id);
    static bool add_id(unsigned int id);
    static bool remove_id(unsigned int id);
    static unsigned int get_unused_id();
};


#endif //SPHEREDRAW_PRIMITIVE_ID_MANAGER_H
