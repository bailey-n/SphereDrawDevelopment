#version 460 core

in vec2 fragUV;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    color = texture(textureSampler, fragUV);
}
