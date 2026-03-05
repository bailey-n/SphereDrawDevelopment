//
// Created by Nathan on 2/27/2026.
//

#ifndef VERTEX_MANIPULATION_H
#define VERTEX_MANIPULATION_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

constexpr float MAX_SUBDIV_WIDTH = M_PI_2 - std::acos(std::sqrt(1.0 / 3.0));

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

inline glm::vec3 slerp(const glm::vec3& u, const glm::vec3& v, float t);
std::vector<glm::vec3> subdivide_line(const glm::vec3&  u, const glm::vec3& v, double max_subdiv_angle);
void make_sphere_circle(glm::vec3 center, float radius, std::vector<glm::vec3>& positions, std::vector<glm::u32vec3>& indices);
void make_sphere_line(const std::vector<glm::vec3>& input_verts, float width, std::vector<glm::vec3>& positions, std::vector<glm::u32vec3>& indices);

#endif //VERTEX_MANIPULATION_H
