//
// Created by Nathan on 1/29/2026.
//

#ifndef SPHEREDRAW_SPHERE_POINT_H
#define SPHEREDRAW_SPHERE_POINT_H

#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SpherePoint {
    glm::vec3 position;
    glm::u8vec4 color;
    glm::float32 radius;
    float latitude;
    float longitude;
};


#endif //SPHEREDRAW_SPHERE_POINT_H
