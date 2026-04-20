#version 460

layout (vertices = 3) out;

// in vec4 tescColor[];
in vec2 tescUV[];
// out vec4 tessColor[];
out vec2 tessUV[];

uniform float inv_min_arc_dist;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    // tessColor[gl_InvocationID] = tescColor[gl_InvocationID];
    tessUV[gl_InvocationID] = tescUV[gl_InvocationID];

    if (gl_InvocationID == 0) {
        vec3 aPos = vec3(gl_in[0].gl_Position);
        vec3 bPos = vec3(gl_in[1].gl_Position);
        vec3 cPos = vec3(gl_in[2].gl_Position);
        // vec3 dPos = vec3(gl_in[3].gl_Position);
        gl_TessLevelOuter[0] = ceil(inv_min_arc_dist * acos(dot(aPos, bPos)));
        gl_TessLevelOuter[1] = ceil(inv_min_arc_dist * acos(dot(bPos, cPos)));
        gl_TessLevelOuter[2] = ceil(inv_min_arc_dist * acos(dot(cPos, aPos))); // For triangles
        // gl_TessLevelOuter[2] = ceil(inv_min_arc_dist * acos(dot(cPos, dPos))); // For quads (currently disabled)
        // gl_TessLevelOuter[3] = ceil(inv_min_arc_dist * acos(dot(dPos, aPos)));

        // If used for irregular quads, switch to using max(max(0, 1), max(2, 3))
        // gl_TessLevelInner[0] = max(1.0, max(gl_TessLevelOuter[0], gl_TessLevelOuter[1]) - 1.0); // For quads
        gl_TessLevelInner[0] = max(1.0, max(max(gl_TessLevelOuter[0], gl_TessLevelOuter[1]), gl_TessLevelOuter[2]) - 1.0);
    }
}