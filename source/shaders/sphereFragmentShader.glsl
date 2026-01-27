#version 330 core

// Input from the vertex shader
in vec3 fragPosition;
in vec3 fragColor;
in vec3 fragNorm;
in vec2 fragUV;

uniform vec3 cameraPosition;
uniform float textureOpacity;
uniform sampler2D textureSampler;

// Output color
out vec4 color;

void main() {
    // Set the fragment color
    vec3 fragmentToLight = normalize(cameraPosition - fragPosition);
    vec3 norm = normalize(fragNorm);

    float diffuseScalingFactor = max(0, dot(fragmentToLight, norm));
    float baseColorFactor = (1.0 - textureOpacity) * min(1.0, diffuseScalingFactor + 0.5);

    color = textureOpacity*texture(textureSampler, fragUV) + vec4(baseColorFactor*fragColor, 1.0);
}
