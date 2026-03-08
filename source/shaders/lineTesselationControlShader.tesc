#version 460

layout (vertices = 3) out;

in vec4 tescColor[];
out vec4 tessColor[];

uniform float inv_min_arc_dist;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tessColor[gl_InvocationID] = tescColor[gl_InvocationID];

    if (gl_InvocationID == 0) {
        vec3 aPos = vec3(gl_in[0].gl_Position);
        vec3 bPos = vec3(gl_in[1].gl_Position);
        vec3 cPos = vec3(gl_in[2].gl_Position);
        gl_TessLevelOuter[0] = ceil(inv_min_arc_dist * acos(dot(aPos, bPos)));
        gl_TessLevelOuter[1] = ceil(inv_min_arc_dist * acos(dot(bPos, cPos)));
        gl_TessLevelOuter[2] = ceil(inv_min_arc_dist * acos(dot(cPos, aPos)));

        gl_TessLevelInner[0] = max(1.0, max(max(gl_TessLevelOuter[0], gl_TessLevelOuter[1]), gl_TessLevelOuter[2]) - 1.0);
    }
}