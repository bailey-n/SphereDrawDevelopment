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
}

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

void CubeFace::queue_draw(CubeMapId id) {
    draw_order.emplace(id);
}

void CubeFace::draw(const Camera& camera, bool update, CubeMapId selected_mesh) {
    std::vector<DrawnMesh*> to_draw;
    to_draw.reserve(draw_order.size());
    std::optional<DrawnMesh*> highlight_mesh;
    while (!draw_order.empty()) {
        auto next_id = draw_order.front();
        if (next_id != selected_mesh) to_draw.emplace_back(&drawn_meshes.at(next_id));
        else highlight_mesh.emplace(&drawn_meshes.at(next_id));
        draw_order.pop();
    }
    if (update) {
        mesh->update_texture(reference_mesh, to_draw, highlight_mesh);
    }
    mesh->draw(camera);
}

std::vector<CubeMapId> CubeFace::get_drawn_elements_at(glm::vec3 pos) const {
    std::vector<CubeMapId> results;
    for (const auto& [_id, _drawn_mesh]: drawn_meshes) {
        if (_drawn_mesh.contains_point(pos)) results.emplace_back(_id);
    }
    return results;
}

