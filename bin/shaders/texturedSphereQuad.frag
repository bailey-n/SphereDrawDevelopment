#version 460 core

in vec2 fragUV;

uniform sampler2D textureSampler;
uniform sampler2D maskSampler;
uniform vec2 texelSize;
uniform vec4 borderColor;
uniform int borderWidth;
uniform bool drawHighlight;

out vec4 color;

void main() {
    vec4 surfaceColor = texture(textureSampler, fragUV);

    if (drawHighlight) {
        float maskValue = texture(maskSampler, fragUV).r;
        surfaceColor = mix(surfaceColor, borderColor, 0.2 * maskValue);
        float dilated = 0.0;
        for (int x = -borderWidth; x <= borderWidth; x++) {
            for (int y = -borderWidth; y <= borderWidth; y++) {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
                float offset_sample = (distance(offset.x, offset.y) < borderWidth) ? texture(maskSampler, fragUV + offset).r : 0.0;
                dilated = max(dilated, offset_sample);
            }
        }

        float drawBorder = clamp(dilated - maskValue, 0.0, 1.0);
        surfaceColor = mix(surfaceColor, borderColor, drawBorder * borderColor.a);
    }

    color = surfaceColor;
}
