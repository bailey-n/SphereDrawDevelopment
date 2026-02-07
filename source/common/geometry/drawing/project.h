#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "primitive.h"
#include "layer.h"

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
    uint32_t nextPrimitiveID() { return nextPrimitiveID_++;}

    // Primitive insertion helpers
    //Adds a primitive to the project and also appends it to the default layer's draw order
    // Returns the primitive ID
    uint32_t addPrimitiveToDefaultLayer(std::unique_ptr<Primitive> p){
        ensureDefaultLayer();
        const uint32_t id = p->getID();
        primitives[id] = std::move(p);
        layers[0].primitiveIDs.push_back(id);
        modifiedUtc = createdUtc; //placeholder, we'll set real timestamps later
        return id;
    }

    //Adds a primitive but does not attach it to a layer (should be useful for loading)
    void addPrimitive(std::unique_ptr<Primitive> p){
        primitives[p->getID()] = std::move(p);
    }

private:
    uint32_t nextPrimitiveID_ = 1;
    uint32_t nextLayerID_ = 1;
};