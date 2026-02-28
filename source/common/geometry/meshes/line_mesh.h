//
// Created by Nathan on 2/27/2026.
//

#ifndef SPHEREDRAW_LINE_MESH_H
#define SPHEREDRAW_LINE_MESH_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#include "vertex_attribute.h"
#include <string>
#include <climits>
#include "basic_mesh.h"
#include "point_mesh.h"
#include "cubemap_util.h"
#include <optional>

struct ConnectorQuad {
    glm::vec3 top_left;
    glm::vec3 bottom_left;
    glm::vec3 bottom_right;
    glm::vec3 top_right;

    ConnectorQuad(const glm::vec4& tl, const glm::vec4& bl, const glm::vec4& br, const glm::vec4& tr);
    [[nodiscard]] std::vector<glm::vec3> subdivide_left(double max_subdiv_angle) const;
    [[nodiscard]] std::vector<glm::vec3> subdivide_right(double max_subdiv_angle) const;
};

std::vector<ConnectorQuad> connect_vertices(const glm::vec3& left, const glm::vec3& right, float tan_w, double subdiv_angle);

class LineMesh {
    BasicMesh mesh;
    GLuint program;

    std::vector<glm::vec3> positions;
    glm::vec4 color;
    float width;

    static constexpr double MAX_SUBDIV_WIDTH = 0.01; // Radians

public:
    LineMesh(const std::vector<LineBuilderVertexInfo>& build_info, const glm::vec4& color, float width);
    void draw(const Camera& camera) const;
};


#endif //SPHEREDRAW_LINE_MESH_H
