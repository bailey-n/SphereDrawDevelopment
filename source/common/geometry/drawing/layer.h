#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Layer {
    uint32_t id = 0; // Layer indentifier
    std::string name = "Default"; // default layer name
    bool visible = true;
    bool locked = false;

    // Ordered draw order (IDs refer to primitives stored elsewhere)
    std::vector<uint32_t> primitiveIDs;
};