#version 330 core

// Input from the vertex shader
in vec4 fragColor;

// Output color
layout(location = 0) out vec4 color;
layout(location = 1) out float mask;

uniform bool highlighted;

void main() {
    color = fragColor;
    mask = highlighted ? 1.0 : 0.0;
}
