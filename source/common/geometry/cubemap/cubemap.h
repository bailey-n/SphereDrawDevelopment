//
// Created by Nathan on 1/29/2026.
//

#ifndef SPHEREDRAW_CUBEMAP_H
#define SPHEREDRAW_CUBEMAP_H

#include <vector>
#include "drawing/primitive.h"
#include "drawing/layer.h"
#include "primitive.h"
#include "layer.h"
#include <cstdint>
#include <climits>
#include <set>
#include <queue>
#include <map>

#include "cubemap_util.h"
#include "cubeface.h"

class Cubemap {
    // Attributes/methods for assigning reference ids for external code
    std::set<CubeMapId> active_ids;
    std::deque<CubeMapId> deactivated_ids;
    CubeMapId lowest_unused_id = 0;
    uint32_t feature_count = 0;

    bool is_active_id(CubeMapId id);
    [[nodiscard]] bool is_full() const;
    [[nodiscard]] CubeMapId activate_new_id();
    void remove_id(CubeMapId id);

    std::vector<CubeMapId> draw_order;
    std::map<CubeMapId, LayerPrimitiveInfo> layer_map;
    std::map<CubeMapId, DrawnPrimitiveInfo> primitive_map;
    CubeFace cube_faces[6];

    [[nodiscard]] uint32_t layer_size(const LayerPrimitiveInfo& info) const;
    void remove_element_from_parent_layer(CubeMapId cmap_id);

public:
    void draw(const Camera& camera) const;

    // Resets the cubemap and removes all rendered objects
    void reset();

    // Gets the type of the object with the given id.
    // Returns 0 if it is a layer, 1 if it is a point, 2 if it is a line, 3 if it is a polygon.
    // Returns -1 if there is no object with that id.
    [[nodiscard]] ObjectType get_object_type(CubeMapId cmap_id) const;

    // Gets the id of the layer an object is within.
    // Returns a valid layer id if it is within a layer, or InvalidId if the object is in the root layer or the id is invalid.
    [[nodiscard]] CubeMapId get_object_layer(CubeMapId cmap_id) const;

    // Gets the position of the object in the render list, regardless of its layer.
    // Returns 0 for the first object rendered (the bottom layer), with the maximum return value being for the topmost rendered object.
    // Returns -1 if the object is not in the render list.
    [[nodiscard]] uint32_t get_global_object_render_position(CubeMapId cmap_id) const;

    // Gets the position of the object in render list relative to the first object in the layer it is within.
    // If the object is not within a layer (so is in root layer), it returns the same as get_global_object_render_position().
    // Returns -1 if the object is not in the render list.
    [[nodiscard]] uint32_t get_local_object_render_position(CubeMapId cmap_id) const;

    // Gets the type of object which is currently being constructed.
    // Returns 1 if it is a point, 2 if it is a line, or 3 if it is a polygon.
    // Returns -1 if no object is currently being constructed.
    ObjectType get_active_construction_type() const;

    // Gets the cmap_id of the object which is currently being constructed.
    // Returns InvalidId if no object is currently being constructed.
    CubeMapId get_active_construction_id() const;

    // Gets the number of vertices in the active construct.
    // Returns -1 if there is no object currently being constructed.
    uint32_t get_active_construction_size() const;

    // Notifies if a new object is under active construction.
    // Returns true if there is an object under construction, false otherwise.
    bool active_construction() const;

    // Adds a vertex to the current object under active construction.
    // Returns the new size of the construct, or -1 if there is no object under active construction.
    uint32_t add_construct_vertex(const glm::vec3& new_vtx);

    // Adds new point, given point data. If you want it to be added to the end of a layer, specify the layer id.
    // If you want it to be in a specific position in the layer, specify the layer id and position.
    CubeMapId add_new_point(const PointPrimitive& point, CubeMapId layer=InvalidId, uint32_t position=-1);

    // Removes a point. Returns true if deletion was successful (so cmap_id is a valid point id), and false otherwise (if cmap_id is not a valid point id).
    bool remove_point(CubeMapId cmap_id);

    // Adds new line, given line primitive data. If you want it to be added to the end of a layer, specify the layer id.
    // If you want it to be in a specific position in the layer, specify the layer id and position (will clamp to back if position >= layer size).
    CubeMapId add_new_line(const PolylinePrimitive& line, CubeMapId layer=InvalidId, uint32_t position=-1);

    // Removes a point. Returns true if deletion was successful (so cmap_id is a valid line id), and false otherwise (if cmap_id is not a valid line id).
    bool remove_line(CubeMapId cmap_id);

    // Begins the construction of a new polyline, starting with its first vertex. Ends any current constructions and converts them to a normal object.
    CubeMapId start_line_construction(const glm::vec3& first_vtx, const glm::vec4& color, float width, bool closed=false);

    // Signals the completion of a new polyline construct.
    // Returns false if there is no line under active construction, or the line is degenerate (e.g., has only 1 vertex).
    bool finish_current_new_line();

    // Adds new polygon, given polygon primitive data. If you want it to be added to the end of a layer, specify the layer id.
    // If you want it to be in a specific position in the layer, specify the layer id and position (will clamp to back if position >= layer size).
    CubeMapId add_new_polygon(const PolygonPrimitive& polygon, CubeMapId layer=InvalidId, uint32_t position=-1);

    // Removes a point. Returns true if deletion was successful (so cmap_id is a valid polygon id), and false otherwise (if cmap_id is not a valid polygon id).
    bool remove_polygon(CubeMapId cmap_id);

    // Begins the construction of a new polygon, starting with its first vertex. Ends any current constructions and converts them to a normal object.
    CubeMapId start_new_polygon(const glm::vec3& first_pos, const glm::vec4& color);

    // Signals the completion of a new polygon construct.
    // Returns false if there is no line under active construction, or the polygon is degenerate (e.g., has only 1 or 2 vertices).
    bool finish_current_new_polygon();

    // Adds new layer, given layer primitive data. If you want it to be added to the end of a parent layer, specify the parent layer id.
    // If you want it to be in a specific position in the parent layer, specify the layer id and position (will clamp to back if position >= parent layer size).
    CubeMapId add_new_layer(const Layer& new_layer, CubeMapId parent_layer=InvalidId, uint32_t position=-1);

    // Removes a layer, moving all internal contents up into its parent layer (or the root layer if it does not have one).
    // Returns true if deletion was successful (so cmap_id is a valid layer id), and false otherwise (if cmap_id is not a valid layer id).
    bool remove_layer(CubeMapId cmap_id);

    // Removes a layer, and all primitives and layers within it.
    // Returns true if deletion was successful (so cmap_id is a valid layer id), and false otherwise (if cmap_id is not a valid layer id).
    bool recursive_remove_layer(CubeMapId cmap_id);

    // Adds new, empty layer, with no layer primitive data. If you want it to be added to the end of a parent layer, specify the parent layer id.
    // If you want it to be in a specific position in the parent layer, specify the layer id and position (will clamp to back if position >= parent layer size).
    CubeMapId new_empty_layer(CubeMapId parent_layer=InvalidId, uint32_t position=-1);
};


#endif //SPHEREDRAW_CUBEMAP_H
