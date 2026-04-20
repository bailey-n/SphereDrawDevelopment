#version 460 core

// Input vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 tescUV;

void main() {
    gl_Position = vec4(position, 1.0);
    tescUV = uv;
}
