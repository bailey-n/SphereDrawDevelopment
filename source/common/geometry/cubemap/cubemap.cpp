//
// Created by Nathan on 1/29/2026.
//

#include "cubemap.h"
#include <iostream>

uint32_t Cubemap::layer_size(const LayerPrimitiveInfo &info) const {
    uint32_t layer_pos = get_global_object_render_position(info.id);
    uint32_t last_pos = get_global_object_render_position(info.end_id);
    return last_pos - layer_pos;
}

void Cubemap::remove_element_from_parent_layer(CubeMapId cmap_id) {
    auto parent_layer = primitive_map.at(cmap_id).parent_layer;
    if (parent_layer == InvalidId) return;
    auto& end_elem = layer_map.at(parent_layer).end_id;
    if (end_elem == cmap_id) {
        end_elem = draw_order[get_global_object_render_position(end_elem)-1];
    }
}

void Cubemap::recursive_layer_insert(CubeMapId cmap_id, CubeMapId layer, uint32_t position) {
    // TODO: Change to support recursive layer insertion
    if (position == -1) {
        if (layer == InvalidId) draw_order.emplace_back(cmap_id);
        else {
            position = get_global_object_render_position(layer_map.at(layer).end_id) + 1;
            layer_map.at(layer).end_id = cmap_id;
            draw_order.insert(draw_order.begin() + position, cmap_id);
        }
    }
    else {
        if (layer == InvalidId) {
            draw_order.insert(draw_order.begin() + position, cmap_id);
        }
        else {
            auto global_position = get_global_object_render_position(layer) + 1 + position;
            if (position == layer_size(layer_map.at(layer))) { layer_map.at(layer).end_id = cmap_id; }
            draw_order.insert(draw_order.begin() + global_position, cmap_id);
        }
    }
}

void Cubemap::clamp_position(CubeMapId layer, uint32_t &position) {
    if (layer == InvalidId) { position = std::min(position, (uint32_t)draw_order.size()); }
    else { position = std::min(position, layer_size(layer_map.at(layer))); }
}

void Cubemap::reset() {
    for (auto& face: cube_faces) face.reset();
    draw_order.clear();
    layer_map.clear();
    primitive_map.clear();

    deactivated_ids.clear();
    active_ids.clear();
    lowest_unused_id = 0;
    feature_count = 0;
}

bool Cubemap::is_active_id(CubeMapId id) {
    return active_ids.contains(id);
}

bool Cubemap::is_full() const {
    return (feature_count == MaxFeatureCt);
}

CubeMapId Cubemap::activate_new_id() {
    if (is_full()) return InvalidId;

    // temp variable to store old lowest_unused_id (which will become active)
    CubeMapId new_id = lowest_unused_id;
    active_ids.insert(new_id);

    // Set new lowest_unused_id
    feature_count++;
    if (deactivated_ids.empty()) { lowest_unused_id = feature_count; }
    else {
        // Ensures that any holes from deactivation are filled first
        lowest_unused_id = deactivated_ids.front();
        deactivated_ids.pop_front();
    }
    return new_id;
}

void Cubemap::remove_id(CubeMapId id) {
    // Safety check
    if (!is_active_id(id)) {
        std::cerr << "Attempted to remove cubemap object with id " << id << ", but id is not active" << std::endl;
        return;
    }
    active_ids.erase(id);
    feature_count--;

    if (deactivated_ids.empty() || (id < lowest_unused_id)) {
        deactivated_ids.push_front(id);
        lowest_unused_id = id;
        return;
    }

    // Deactivated id list is maintained to ensure that any id holes are filled
    // TODO: Make into binary search if performance is a problem
    auto curr = deactivated_ids.begin()+1;
    while (curr != deactivated_ids.end()) {
        if (id == *curr) {
            std::cerr << "Attempted to add deactivated cubemap object with id " << id << " to deactivated object list, but that id is already in the list" << std::endl;
            return;
        }
        if (id < *curr) {
            break;
        }
        ++curr;
    }
    deactivated_ids.insert(curr, id);
}

void Cubemap::draw(const Camera &camera) const {
    for (const auto id: draw_order) {
        const auto& primitive = primitive_map.at(id);
        if (primitive.type != ObjectType::InvalidObject && primitive.type != ObjectType::renderLayer) {
            if (primitive.face_flags & flagNorth) cube_faces[North].draw(primitive.id, primitive.type, camera);
            if (primitive.face_flags & flagWest) cube_faces[West].draw(primitive.id, primitive.type, camera);
            if (primitive.face_flags & flagMeridian) cube_faces[Meridian].draw(primitive.id, primitive.type, camera);
            if (primitive.face_flags & flagEast) cube_faces[East].draw(primitive.id, primitive.type, camera);
            if (primitive.face_flags & flagAntiMeridian) cube_faces[AntiMeridian].draw(primitive.id, primitive.type, camera);
            if (primitive.face_flags & flagSouth) cube_faces[South].draw(primitive.id, primitive.type, camera);
        }
    }
}

ObjectType Cubemap::get_object_type(CubeMapId cmap_id) const {
    if (!primitive_map.contains(cmap_id)) return InvalidObject;
    return primitive_map.at(cmap_id).type;
}

CubeMapId Cubemap::get_object_layer(CubeMapId cmap_id) const {
    if (!primitive_map.contains(cmap_id)) return InvalidId;
    return primitive_map.at(cmap_id).parent_layer;
}

uint32_t Cubemap::get_global_object_render_position(CubeMapId cmap_id) const {
    if (!primitive_map.contains(cmap_id)) return InvalidId;
    for (unsigned int i = 0; i < draw_order.size(); i++) {
        if (cmap_id == draw_order[i]) return i;
    }
    return InvalidId;
}

uint32_t Cubemap::get_local_object_render_position(CubeMapId cmap_id) const {
    if (!primitive_map.contains(cmap_id)) return InvalidId;
    uint32_t global_draw_position = get_global_object_render_position(cmap_id);
    CubeMapId object_layer = get_object_layer(cmap_id);
    if (object_layer == InvalidId) return global_draw_position;
    uint32_t layer_draw_position = get_global_object_render_position(object_layer);
    return global_draw_position - layer_draw_position - 1;
}

CubeMapId Cubemap::add_new_point(const PointPrimitive &point, CubeMapId layer, uint32_t position) {
    // Ensure layer exists
    if (layer != InvalidId && !primitive_map.contains(layer)) return InvalidId;
    clamp_position(layer, position);

    CubeFaceNum face = get_face(point.p);

    // TODO: Account for layer and position insertion
    CubeMapId render_id = activate_new_id();
    if (render_id == InvalidId) return InvalidId;

    CubeFaceFlags flags;
    switch (face) {
        case North: flags = flagNorth; break;
        case West: flags = flagWest; break;
        case Meridian: flags = flagMeridian; break;
        case East: flags = flagEast; break;
        case AntiMeridian: flags = flagAntiMeridian; break;
        case South: flags = flagSouth; break;
    }

    // Add point
    primitive_map.try_emplace(render_id, renderPoint, render_id, layer, flags);
    cube_faces[face].add_new_point_primitive(render_id, point);
    recursive_layer_insert(render_id, layer, position);

    return render_id;
}

bool Cubemap::remove_point(CubeMapId cmap_id) {
    if (!active_ids.contains(cmap_id)) return false;
    auto& info = primitive_map.at(cmap_id);
    if (info.type != ObjectType::renderPoint) return false;
    auto flags = info.face_flags;

    if (flags & flagNorth) { cube_faces[North].remove_point_primitive(cmap_id); }
    if (flags & flagWest) { cube_faces[West].remove_point_primitive(cmap_id); }
    if (flags & flagMeridian) { cube_faces[Meridian].remove_point_primitive(cmap_id); }
    if (flags & flagEast) { cube_faces[East].remove_point_primitive(cmap_id); }
    if (flags & flagAntiMeridian) { cube_faces[AntiMeridian].remove_point_primitive(cmap_id); }
    if (flags & flagSouth) { cube_faces[South].remove_point_primitive(cmap_id); }

    remove_element_from_parent_layer(cmap_id);
    draw_order.erase(draw_order.begin()+get_global_object_render_position(cmap_id));
    primitive_map.erase(cmap_id);
    remove_id(cmap_id);

    return true;
}

CubeMapId Cubemap::add_new_line(const PolylinePrimitive& line, CubeMapId layer, uint32_t position) {
    if (layer != InvalidId && !primitive_map.contains(layer)) return InvalidId;
    clamp_position(layer, position);


}