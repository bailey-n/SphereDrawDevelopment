#version 460

layout (triangles, equal_spacing, ccw) in;

in vec4 tessColor[];
out vec4 fragColor;

uniform mat4 MVP;

void main() {
    vec3 pos = normalize(
        gl_TessCoord.x * vec3(gl_in[0].gl_Position) +
        gl_TessCoord.y * vec3(gl_in[1].gl_Position) +
        gl_TessCoord.z * vec3(gl_in[2].gl_Position)
    );
    gl_Position = MVP * vec4(pos, 1.0);

    fragColor = (
        gl_TessCoord.x * tessColor[0] +
        gl_TessCoord.y * tessColor[1] +
        gl_TessCoord.z * tessColor[2]
    );
}