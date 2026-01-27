#ifndef PLANET_H
#define PLANET_H

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <deque>
#include "application_action.h"
#include "shader_manager.h"
#include "camera.h"
#include "sphere_mesh.h"

class Planet {
    double radius;

    const std::string vert_shader = "sphereVertexShader.glsl";
    const std::string frag_shader = "sphereFragmentShader.glsl";
    const std::string cube_map = "textures/Earth_cube_map.png";

    SphereMesh mesh;

    void init();

public:
    Planet();
    ~Planet();
    explicit Planet(double radius);
    void draw(const Camera& camera) const;
    [[nodiscard]] double get_radius() const;
};



#endif //PLANET_H
