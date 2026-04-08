#version 460 core

// Input from the vertex shader
in vec3 fragPosition;
in vec2 fragUV;

uniform float textureOpacity;
uniform sampler2D textureSampler;

// Output color
layout(location = 0) out vec4 color;
layout(location = 1) out float mask;

void main() {
    color = textureOpacity*texture(textureSampler, fragUV);
    mask = 0.0;
}
