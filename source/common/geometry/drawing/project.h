#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include "primitive.h"
#include "layer.h"
#include "primitive_id_manager.h"
#include "cubemap/cubemap.h"


// Represents a SphereDraw document in memory
class Project {
public:
    // Metadata
    int formatVersion = 1;
    std::string createdUtc; //ISO 8601 string timestamp
    std::string modifiedUtc;

    //Ordered layers
    std::vector<Layer> layers;

    //Primitive storage (lookup by ID)
    std::unordered_map<uint32_t, std::unique_ptr<Primitive>> primitives;

    Project(){
        PrimIDManager::reset();
        ensureDefaultLayer();
    }

    //Ensure there is at least one layer, call after load if needed
    void ensureDefaultLayer(){
        if(layers.empty()){
            Layer def;
            def.id = nextLayerID_++;
            def.name = "Default";
            layers.push_back(def);
        }
    }

    //ID generation helpers
    uint32_t nextPrimitiveID() {
        // Get an unused id, then immediately reserve it.
        uint32_t id = static_cast<uint32_t>(PrimIDManager::get_unused_id());
        // In the (unlikely) case of collision, keep trying.
        while (!PrimIDManager::add_id(id)) {
            id = static_cast<uint32_t>(PrimIDManager::get_unused_id());
        }
        return id;
    }

    //deletes a primitive and removes its id
    bool deletePrimitive(uint32_t id) {
        auto it = primitives.find(id);
        if (it == primitives.end()) return false;

        // Remove references from all layers
        for (auto& layer : layers) {
            auto& ids = layer.primitiveIDs;
            ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
        }

        primitives.erase(it);
        PrimIDManager::remove_id(id);
        return true;
    }

    std::string makeDefaultPrimitiveName(PrimitiveType type, uint32_t exclude_id = 0) const {
        return primitiveTypeDisplayName(type) + " " +
               std::to_string(countPrimitivesOfType(type, exclude_id) + 1);
    }



    // Primitive insertion helpers
    //Adds a primitive to the project and also appends it to the default layer's draw order
    // Returns the primitive ID
    uint32_t addPrimitiveToDefaultLayer(std::unique_ptr<Primitive> p){
        ensureDefaultLayer();
        ensurePrimitiveHasName(*p);

        const uint32_t id = p->getID();
        primitives[id] = std::move(p);
        layers[0].primitiveIDs.push_back(id);

        //makes primitive additions appear immediately
        if (cubemap_) {
            Primitive* base = primitives[id].get();
            if (!base) {
                return id;
            }

            if (base->getType() == PrimitiveType::Point) {
                if (auto* pt = dynamic_cast<PointPrimitive*>(base)) {
                    cubemap_->add_new_point(*pt);
                }
            }
            else if (base->getType() == PrimitiveType::Polyline) {
                if (auto* line = dynamic_cast<PolylinePrimitive*>(base)) {
                    cubemap_->add_new_line(*line);
                }
            }
            //TODO: Once we have polygon rending finish this, only adds polygon outline for now while waiting on polygon rendering
            else if (base->getType() == PrimitiveType::Polygon) {
                if (auto* polygon = dynamic_cast<PolygonPrimitive*>(base)) {
                    PolylinePrimitive outline = makeOutlineFallbackForPolygon(*polygon);
                    cubemap_->add_new_line(outline);
                }
            }
        }

        return id;
    }

    //Adds a primitive but does not attach it to a layer (should be useful for loading)
    void addPrimitive(std::unique_ptr<Primitive> p){
        ensurePrimitiveHasName(*p);
        primitives[p->getID()] = std::move(p);
    }

    // CUBEMAP HELPERS

    // Attach a renderer (non-owning). Project does NOT manage this lifetime.
    void attachCubemap(Cubemap* cubemap) { cubemap_ = cubemap; }

    Cubemap* getCubemap() const { return cubemap_; }

    // Rebuild render-side cubemap from current in-memory project data.
    // First milestone: points only (lines/polygons can be added later).
    void rebuildAttachedCubemapFromProject() {
        if (!cubemap_) return;

        cubemap_->reset();

        // Render in layer order, then primitive order within each layer
        for (const auto& layer : layers) {
            // Respect visibility for rendering
            if (!layer.visible) continue;

            for (uint32_t pid : layer.primitiveIDs) {
                auto it = primitives.find(pid);
                if (it == primitives.end() || !it->second) continue;

                Primitive* base = it->second.get();

                if (base->getType() == PrimitiveType::Point) {
                    auto* pt = dynamic_cast<PointPrimitive*>(base);
                    if (pt) {
                        cubemap_->add_new_point(*pt);
                    }
                }
                else if (base->getType() == PrimitiveType::Polyline) {
                    auto* line = dynamic_cast<PolylinePrimitive*>(base);
                    if (line) {
                        cubemap_->add_new_line(*line);
                    }
                }
                //TODO: Once we have polygon rending finish this, only adds polygon outline for now while waiting on polygon rendering
                else if (base->getType() == PrimitiveType::Polygon) {
                    auto* polygon = dynamic_cast<PolygonPrimitive*>(base);
                    if (polygon) {
                        PolylinePrimitive outline = makeOutlineFallbackForPolygon(*polygon);
                        cubemap_->add_new_line(outline);
                    }
                }
            }
        }
    }

    void selectPrimitiveAt(glm::vec3 pos) {
        std::vector<std::pair<uint32_t, CubeMapId>> selected_objects = cubemap_->get_drawn_elements_at(pos);
        for (auto object: selected_objects) {
            std::cout << object.first << std::endl;
        }
        if (!selected_objects.empty()) {
            auto id = selected_objects.front();
            cubemap_->select(id.second);
        }
        else {
            cubemap_->deselect();
        }
    }

private:

    static PolylinePrimitive makeOutlineFallbackForPolygon(const PolygonPrimitive& polygon) {
        PolylinePrimitive outline(polygon.getID());
        outline.setName(polygon.getName());
        outline.color = polygon.color;
        outline.width = 0.007f;
        outline.verts = polygon.verts;

        // Close the loop explicitly for the current line renderer.
        if (outline.verts.size() >= 3) {
            outline.verts.push_back(outline.verts.front());
        }

        return outline;
    }

    static std::string primitiveTypeDisplayName(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Point: return "Point";
            case PrimitiveType::Polyline: return "Polyline";
            case PrimitiveType::Polygon: return "Polygon";
            default: return "Primitive";
        }
    }

    size_t countPrimitivesOfType(PrimitiveType type, uint32_t exclude_id = 0) const {
        size_t count = 0;
        for (const auto& [id, primitive] : primitives) {
            if (!primitive) continue;
            if (id == exclude_id) continue;
            if (primitive->getType() == type) {
                ++count;
            }
        }
        return count;
    }

    void ensurePrimitiveHasName(Primitive& primitive) {
        if (primitive.getName().empty()) {
            primitive.setName(makeDefaultPrimitiveName(primitive.getType(), primitive.getID()));
        }
    }

    uint32_t nextLayerID_ = 1;
    Cubemap* cubemap_ = nullptr; // non-owning pointer to the active renderer-side cubemap
};