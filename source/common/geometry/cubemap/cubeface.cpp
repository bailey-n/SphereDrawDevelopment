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

void CubeFace::add_new_line_primitive(CubeMapId id, const PolylinePrimitive& line_data, const std::vector<LineBuilderVertexInfo> &line_builder_info) {
    line_meshes.try_emplace(id);
    line_meshes.at(id).emplace_back(line_builder_info, line_data.color, line_data.width);
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
            for (const auto& mesh: line_meshes.at(id)) mesh.draw(camera);
            break;
        case renderPolygon:
            break;
        case InvalidObject:
        case renderLayer:
            break;
    }
}