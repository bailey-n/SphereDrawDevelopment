#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Uniforms
uniform mat4 MVP; // Combined Model-View-Projection matrix

// Output to the fragment shader
out vec3 fragPosition;
out vec3 fragColor;
out vec3 fragNorm;
out vec2 fragUV;

void main() {
    // Transform the vertex position
    gl_Position = MVP * vec4(position, 1.0);
    fragPosition = position;
    fragColor = color;
    fragNorm = normal;
    fragUV = uv;
}
