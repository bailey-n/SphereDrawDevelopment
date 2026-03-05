#version 460 core

// Input from the vertex shader
in vec3 fragPosition;
in vec2 fragUV;

uniform float textureOpacity;
uniform sampler2D textureSampler;

// Output color
out vec4 color;

void main() {
    color = textureOpacity*texture(textureSampler, fragUV);
}
