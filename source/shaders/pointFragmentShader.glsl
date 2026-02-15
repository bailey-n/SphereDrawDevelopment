#version 330 core

// Input from the vertex shader
in vec4 fragColor;

// Output color
out vec4 color;

void main() {
    color = fragColor;
}
