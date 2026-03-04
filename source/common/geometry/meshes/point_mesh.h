//
// Created by Nathan on 2/14/2026.
//

#ifndef SPHEREDRAW_POINT_MESH_H
#define SPHEREDRAW_POINT_MESH_H

#include <vector>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#include "vertex_attribute.h"
#include <string>
#include <climits>
#include "basic_mesh.h"

// May have to group into collection later to speed up draw calls
class PointMesh {
    BasicMesh mesh;
    GLuint program;

    glm::vec3 position;
    glm::vec4 color;
    float radius;

public:
    PointMesh(glm::vec3 position, glm::vec4 color, float radius);
    void draw(const Camera& camera) const;
};


#endif //SPHEREDRAW_POINT_MESH_H
