#include <iostream>
#include <cstdio> // std::remove

#include <glm/glm.hpp>

#include "common/geometry/drawing/project.h"
#include "common/geometry/drawing/project_io.h"
#include "common/geometry/drawing/primitive_id_manager.h"

static bool fail(const std::string& msg) {
    std::cerr << "FAIL: " << msg << "\n";
    return false;
}

bool test_json_roundtrip() {
    const std::string path = "../../test/out/test_project_roundtrip.json";
    std::remove(path.c_str());

    // ---- Build a project ----
    Project p;
    p.createdUtc = "TEST_CREATED";
    p.modifiedUtc = "TEST_MODIFIED";

    // Add a point
    {
        auto pt = std::make_unique<PointPrimitive>(p.nextPrimitiveID());
        pt->p = glm::vec3(1.f, 0.f, 0.f);
        pt->color = glm::vec4(1.f, 0.f, 0.f, 1.f);
        p.addPrimitiveToDefaultLayer(std::move(pt));
    }

    // Add a polyline
    {
        auto ln = std::make_unique<PolylinePrimitive>(p.nextPrimitiveID());
        ln->color = glm::vec4(0.f, 1.f, 0.f, 1.f);
        ln->width = 2.5f;
        ln->verts.push_back(glm::vec3(0.f, 1.f, 0.f));
        ln->verts.push_back(glm::vec3(0.f, 0.f, 1.f));
        p.addPrimitiveToDefaultLayer(std::move(ln));
    }

    // ---- Save ----
    try {
        SaveProjectToFile(p, path);
    } catch (const std::exception& e) {
        return fail(std::string("SaveProjectToFile threw: ") + e.what());
    }

    // ---- Load ----
    Project loaded;
    try {
        loaded = LoadProjectFromFile(path);
    } catch (const std::exception& e) {
        return fail(std::string("LoadProjectFromFile threw: ") + e.what());
    }

    // ---- Assertions ----
    if (loaded.layers.empty()) return fail("No layers after load");
    if (loaded.layers[0].primitiveIDs.size() != 2) return fail("Expected 2 primitive IDs in default layer");
    if (loaded.primitives.size() != 2) return fail("Expected 2 primitives after load");

    for (auto id : loaded.layers[0].primitiveIDs) {
        if (loaded.primitives.find(id) == loaded.primitives.end()) {
            return fail("Layer references missing primitive id: " + std::to_string(id));
        }
    }

    // Ensure ID manager won't collide after load
    uint32_t newId = loaded.nextPrimitiveID();
    if (loaded.primitives.find(newId) != loaded.primitives.end()) {
        return fail("ID collision: nextPrimitiveID returned an existing ID");
    }

    return true;
}
