#include <cmath>
#include "planet.h"
#include "shapes.h"
#include "shader_manager.h"

void Planet::init() {
    shaderManager shader_manager(vert_shader, frag_shader);
    mesh.set_program(shader_manager[{vert_shader, frag_shader}]);
}

Planet::Planet() :
radius(1.0),
mesh(cube_map,
     generate_cuboid_face_mesh_data(23),
     scale(glm::mat4(1.0), glm::vec3(radius, radius, radius))
     ) {
    init();
}

Planet::~Planet() = default;

Planet::Planet(const double radius) :
radius(radius),
mesh(cube_map,
     generate_cuboid_face_mesh_data(23),
     scale(glm::mat4(1.0), glm::vec3(radius, radius, radius))
     ) {
    init();
}

void Planet::draw(const Camera& camera) const {
    mesh.draw(camera, 1.0);
}

double Planet::get_radius() const {
    return radius;
}