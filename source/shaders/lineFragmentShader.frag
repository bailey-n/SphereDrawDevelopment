#version 460

// Input from the tess evaluation shader
in vec4 fragColor;

// Output color
out vec4 color;

void main() {
    color = fragColor;
}
