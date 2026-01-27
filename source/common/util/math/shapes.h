#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "sphere_mesh.h"
#include <optional>

SphereMeshData generate_cuboid_face_mesh_data(unsigned int vertices_per_edge);
std::pair<float, float> xyz_to_lat_lon(const glm::vec3& xyz);
glm::vec3 lat_lon_to_xyz(float latitude, float longitude, float radius);
glm::vec3 sphere_click_xyz(const glm::vec3& position, const glm::vec3& up, float sphere_radius,
                                             const glm::vec2& screen_coords, float window_width, float window_height,
                                             float fov_y);
std::pair<float, float> sphere_click_lat_lon(const glm::vec3& position, const glm::vec3& up, float sphere_radius,
                                             const glm::vec2& screen_coords, float window_width, float window_height,
                                             float fov_y);

#endif //SHAPES_H
