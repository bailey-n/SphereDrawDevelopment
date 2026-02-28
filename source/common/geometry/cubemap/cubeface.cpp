//
// Created by Nathan on 2/14/2026.
//

#include "cubeface.h"

CubeFace::CubeFace() = default;

void CubeFace::reset() {
    point_meshes.clear();
    line_meshes.clear();
}

void CubeFace::add_new_point_primitive(CubeMapId id, const PointPrimitive &point_data) {
    point_meshes.try_emplace(id, point_data.p, point_data.color, point_data.size);
}

void CubeFace::remove_point_primitive(CubeMapId id) {
    point_meshes.erase(id);
}

void CubeFace::add_new_line_primitive(CubeMapId id, const std::vector<LineBuilderVertexInfo> &line_builder_info) {
    line_meshes.try_emplace(id, line_builder_info);
}

void CubeFace::remove_line_primitive(CubeMapId id) {
    line_meshes.erase(id);
}

void CubeFace::draw(CubeMapId id, ObjectType type, const Camera &camera) const {
    switch (type) {
        case renderPoint:
            point_meshes.at(id).draw(camera);
            break;
        case renderLine:
            line_meshes.at(id).draw(camera);
            break;
        case renderPolygon:
            break;
        case InvalidObject:
        case renderLayer:
            break;
    }
}