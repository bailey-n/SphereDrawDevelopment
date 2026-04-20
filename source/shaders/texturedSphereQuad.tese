#version 460 core

layout (triangles, equal_spacing, ccw) in;

// in vec4 tessColor[];
in vec2 tessUV[];
// out vec4 fragColor;
out vec2 fragUV;

uniform mat4 MVP;

void main() {
    vec3 pos = normalize(
//        (1.0-gl_TessCoord.x) * vec3(gl_in[0].gl_Position) + gl_TessCoord.x * vec3(gl_in[1].gl_Position) +
//        (1.0-gl_TessCoord.y) * vec3(gl_in[0].gl_Position) + gl_TessCoord.y * vec3(gl_in[3].gl_Position)
        gl_TessCoord.x * vec3(gl_in[0].gl_Position) +
        gl_TessCoord.y * vec3(gl_in[1].gl_Position) +
        gl_TessCoord.z * vec3(gl_in[2].gl_Position)
    );
    gl_Position = MVP * vec4(pos, 1.0);

    fragUV = (
//        (1.0-gl_TessCoord.x) * tessUV[0] + gl_TessCoord.x * tessUV[1] +
//        (1.0-gl_TessCoord.y) * tessUV[0] + gl_TessCoord.y * tessUV[3]
        gl_TessCoord.x * tessUV[0] +
        gl_TessCoord.y * tessUV[1] +
        gl_TessCoord.z * tessUV[2]
    );
}