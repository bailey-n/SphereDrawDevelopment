//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_H
#define SPHEREDRAW_CUBEFACE_H

#include <GL/glew.h>
#include "point_mesh.h"
#include "line_mesh.h"
#include "cubemap_util.h"
#include "drawing/primitive.h"
#include <map>
#include <queue>
#include "cubeface_mesh.h"
#include "drawn_mesh.h"
#include <optional>
#include "reference_texture_mesh.h"

class CubeFace {
    CubeFaceNum face;

    std::optional<ReferenceTextureMesh> reference_mesh;
    std::optional<CubeFaceMesh> mesh;

    std::map<CubeMapId, DrawnMesh> drawn_meshes;
    std::queue<CubeMapId> draw_order;

    std::map<CubeMapId, PointMesh> point_meshes;
    std::map<CubeMapId, std::vector<LineMesh>> line_meshes;
public:
    CubeFace();
    void init(CubeFaceNum face);
    void reset();
    void add_new_point_primitive(CubeMapId id, const PointPrimitive& point_data);
    void remove_point_primitive(CubeMapId id);
    void add_new_line_primitive(CubeMapId id, const PolylinePrimitive& line_data, const std::vector<LineBuilderVertexInfo>& line_builder_info);
    void remove_line_primitive(CubeMapId id);

    void queue_draw(CubeMapId id, ObjectType type, const Camera& camera);
    void draw(const Camera& camera);
};


#endif //SPHEREDRAW_CUBEFACE_H
