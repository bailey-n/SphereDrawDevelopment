#version 460 core

// Input vertex attributes (from VBO)
layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec4 color;    // Vertex color

// Output to the fragment shader
out vec4 tescColor;

void main() {
    gl_Position = vec4(position, 1.0);
    tescColor = color;
}
