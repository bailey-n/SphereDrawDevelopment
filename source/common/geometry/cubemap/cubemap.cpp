//
// Created by Nathan on 1/29/2026.
//

#include "cubemap.h"
#include <iostream>

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
    for (const auto& primitive: primitive_info) {
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
    if (!id_map.contains(cmap_id)) return InvalidObject;
    return primitive_info.at(id_map.at(cmap_id)).type;
}

CubeMapId Cubemap::get_object_layer(CubeMapId cmap_id) const {
    if (!id_map.contains(cmap_id)) return InvalidId;
    return primitive_info.at(id_map.at(cmap_id)).parent_layer;
}

uint32_t Cubemap::get_global_object_render_position(CubeMapId cmap_id) const {
    if (!id_map.contains(cmap_id)) return InvalidId;
    return primitive_info.at(id_map.at(cmap_id)).draw_position;
}

uint32_t Cubemap::get_local_object_render_position(CubeMapId cmap_id) const {
    if (!id_map.contains(cmap_id)) return InvalidId;
    uint32_t global_draw_position = get_global_object_render_position(cmap_id);
    CubeMapId object_layer = get_object_layer(cmap_id);
    if (object_layer == InvalidId) return global_draw_position;
    uint32_t layer_draw_position = primitive_info.at(id_map.at(object_layer)).draw_position;
    return global_draw_position - layer_draw_position;
}

CubeMapId Cubemap::add_new_point(const PointPrimitive &point, CubeMapId layer, uint32_t position) {
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

    primitive_info.emplace_back(
            renderPoint,
            render_id,
            InvalidId,
            primitive_info.size(),
            flags
            );

    cube_faces[face].add_new_point_primitive(render_id, point);

    return render_id;
}