#version 330 core

// Input vertex attributes (from VBO)
layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec4 color;    // Vertex color

// Uniforms
uniform mat4 MVP; // Combined Model-View-Projection matrix

// Output to the fragment shader
out vec4 fragColor;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
    fragColor = color;
}
