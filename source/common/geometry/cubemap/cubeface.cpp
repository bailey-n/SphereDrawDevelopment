//
// Created by Nathan on 2/14/2026.
//

#include "cubeface.h"
#include "vertex_manipulation.h"

CubeFace::CubeFace() : face(North) {}

void CubeFace::init(CubeFaceNum face) {
    this->face = face;
    reference_mesh.emplace(face);
    mesh.emplace(face);
}


void CubeFace::reset() {
    drawn_meshes.clear();
    point_meshes.clear();
    line_meshes.clear();
}

// void CubeFace::add_new_point_primitive(CubeMapId id, const PointPrimitive &point_data) {
//     point_meshes.try_emplace(id, point_data.p, point_data.color, point_data.size);
// }

void CubeFace::add_new_point_primitive(CubeMapId id, const PointPrimitive& point_data) {
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::u32vec3> temp_indices;
    make_sphere_circle(point_data.p, point_data.size, temp_positions, temp_indices);
    std::vector<glm::vec4> temp_colors(temp_positions.size(), point_data.color);

    // TODO: Add processing to ensure that max arc size is < 35.26 degrees

    drawn_meshes.try_emplace(id, face, temp_positions, temp_colors, temp_indices);
}

void CubeFace::remove_point_primitive(CubeMapId id) {
    drawn_meshes.erase(id);
    // point_meshes.erase(id);
}

void CubeFace::add_new_line_primitive(CubeMapId id, const PolylinePrimitive& line_data, const std::vector<LineBuilderVertexInfo> &line_builder_info) {
    line_meshes.try_emplace(id);
    line_meshes.at(id).emplace_back(line_builder_info, line_data.color, line_data.width);
}

void CubeFace::remove_line_primitive(CubeMapId id) {
    line_meshes.erase(id);
}

void CubeFace::queue_draw(CubeMapId id, ObjectType type, const Camera &camera) {
    // switch (type) {
    //     case renderPoint:
    //         point_meshes.at(id).draw(camera);
    //         break;
    //     case renderLine:
    //         for (const auto& mesh: line_meshes.at(id)) mesh.draw(camera);
    //         break;
    //     case renderPolygon:
    //         break;
    //     case InvalidObject:
    //     case renderLayer:
    //         break;
    // }
    draw_order.emplace(id);
}

void CubeFace::draw(const Camera& camera) {
    std::vector<DrawnMesh*> to_draw;
    to_draw.reserve(draw_order.size());
    while (!draw_order.empty()) {
        to_draw.emplace_back(&drawn_meshes.at(draw_order.front()));
        draw_order.pop();
    }
    mesh->update_texture(reference_mesh, to_draw);
    mesh->draw(camera);
}
