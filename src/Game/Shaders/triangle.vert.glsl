//
// Generate a triangle quad without vertex buffer.
//
#version 450

layout( push_constant ) uniform constants {
    vec4 offset;
} push;

vec4 vertices[3] = {
    vec4(1,   1, 0, 1), // bottom right
    vec4(0,  -1, 0, 1), // top middle
    vec4(-1,  1, 0, 1), // bottom left
};

void main() {
    gl_Position = vertices[gl_VertexIndex] * vec4(push.offset.zw, 1, 1) + vec4(push.offset.xy, 1, 1);
}
