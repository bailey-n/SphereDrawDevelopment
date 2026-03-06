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

// void CubeFace::add_new_point_primitive(CubeMapId id, const PointPrimitive& point_data) {
//     std::vector<glm::vec3> temp_positions;
//     std::vector<glm::u32vec3> temp_indices;
//     make_sphere_circle(point_data.p, point_data.size, temp_positions, temp_indices);
//     std::vector<glm::vec4> temp_colors(temp_positions.size(), point_data.color);
//
//     // TODO: Add processing to ensure that max arc size is < 35.26 degrees
//
//     drawn_meshes.try_emplace(id, face, temp_positions, temp_colors, temp_indices);
// }
//
// void CubeFace::remove_point_primitive(CubeMapId id) {
//     drawn_meshes.erase(id);
//     // point_meshes.erase(id);
// }
//
// void CubeFace::add_new_line_primitive(CubeMapId id, const PolylinePrimitive& line_data) {
//     std::vector<glm::vec3> temp_positions;
//     std::vector<glm::u32vec3> temp_indices;
//     make_sphere_line(line_data.verts, line_data.width, temp_positions, temp_indices);
//     std::vector<glm::vec4> temp_colors(temp_positions.size(), line_data.color);
//
//     drawn_meshes.try_emplace(id, face, temp_positions, temp_colors, temp_indices);
//
//     // line_meshes.try_emplace(id);
//     // line_meshes.at(id).emplace_back(line_builder_info, line_data.color, line_data.width);
// }
//
// void CubeFace::remove_line_primitive(CubeMapId id) {
//     drawn_meshes.erase(id);
//     // line_meshes.erase(id);
// }

bool CubeFace::add_new_drawn_primitive(CubeMapId id, const glm::vec4& color, const std::vector<glm::vec3>& positions, const std::vector<glm::u32vec3>& indices) {
    if (drawn_meshes.contains(id)) return false;
    std::vector<glm::vec4> temp_colors(positions.size(), color);
    drawn_meshes.try_emplace(id, face, positions, temp_colors, indices);
    // If all indices were culled, delete the mesh.
    if (!drawn_meshes.at(id).renderable()) {
        drawn_meshes.erase(id);
        return false;
    }
    return true;
}

bool CubeFace::remove_drawn_primitive(CubeMapId id) {
    if (drawn_meshes.contains(id)) {
        drawn_meshes.erase(id);
        return true;
    }
    return false;
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
    if (!draw_order.empty()) {
        std::vector<DrawnMesh*> to_draw;
        to_draw.reserve(draw_order.size());
        while (!draw_order.empty()) {
            to_draw.emplace_back(&drawn_meshes.at(draw_order.front()));
            draw_order.pop();
        }
        mesh->update_texture(reference_mesh, to_draw);
    }
    mesh->draw(camera);
}
