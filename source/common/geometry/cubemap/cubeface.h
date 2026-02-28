//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_CUBEFACE_H
#define SPHEREDRAW_CUBEFACE_H

#include "cubemap_util.h"
#include "point_mesh.h"
#include "line_mesh.h"
#include "drawing/primitive.h"
#include <map>

class CubeFace {
    std::map<CubeMapId, PointMesh> point_meshes;
    std::map<CubeMapId, std::vector<LineMesh>> line_meshes;

public:
    CubeFace();
    void reset();
    void add_new_point_primitive(CubeMapId id, const PointPrimitive& point_data);
    void remove_point_primitive(CubeMapId id);
    void add_new_line_primitive(CubeMapId id, const PolylinePrimitive& line_data, const std::vector<LineBuilderVertexInfo>& line_builder_info);
    void remove_line_primitive(CubeMapId id);
    void draw(CubeMapId id, ObjectType type, const Camera& camera) const;
};


#endif //SPHEREDRAW_CUBEFACE_H
