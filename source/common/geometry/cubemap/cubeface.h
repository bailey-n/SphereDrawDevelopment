//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_H
#define SPHEREDRAW_CUBEFACE_H

#include "opengl_include.h"
#include "cubemap_util.h"
#include "primitive.h"
#include "cubeface_mesh.h"
#include "drawn_mesh.h"
#include "reference_texture_mesh.h"
#include <map>
#include <queue>
#include <optional>

class CubeFace {
    CubeFaceNum face;

    std::optional<ReferenceTextureMesh> reference_mesh;
    std::optional<CubeFaceMesh> mesh;

    std::map<CubeMapId, DrawnMesh> drawn_meshes;
    std::queue<CubeMapId> draw_order;

public:
    CubeFace();
    void init(CubeFaceNum face);
    void reset();
    bool add_new_drawn_primitive(CubeMapId id, const glm::vec4& color, const std::vector<glm::vec3>& positions, const std::vector<glm::u32vec3>& indices);
    bool remove_drawn_primitive(CubeMapId id);

    void queue_draw(CubeMapId id);
    void draw(const Camera& camera, bool updated, CubeMapId selected_mesh = UINT32_MAX);

    [[nodiscard]] std::vector<CubeMapId> get_drawn_elements_at(glm::vec3 pos) const;
};


#endif //SPHEREDRAW_CUBEFACE_H
