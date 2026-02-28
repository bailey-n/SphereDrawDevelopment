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

inline glm::vec3 slerp(const glm::vec3& u, const glm::vec3& v, float t);
std::vector<glm::vec3> subdivide_line(const glm::vec3&  u, const glm::vec3& v, double max_subdiv_angle);
void make_sphere_circle(glm::vec3 center, float radius, std::vector<glm::vec3>& positions, std::vector<glm::u32vec3>& indices);


#endif //VERTEX_MANIPULATION_H
