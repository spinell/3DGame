//
// Generate a triangle quad without vertex buffer.
//
#version 450

layout (location = 0) out vec2 outUV;

layout( push_constant ) uniform constants {
    vec2 offset;
    vec2 size;
} push;

vec4 vertices[3] = {
    vec4(1,   1, 0, 1), // bottom right
    vec4(0,  -1, 0, 1), // top middle
    vec4(-1,  1, 0, 1), // bottom left
};
vec2 uv[3] = {
    vec2( 1,  0),  // bottom right
    vec2( 0, 0.5), // top middle
    vec2( 1,  1)   // bottom left
};

void main() {
    outUV = uv[gl_VertexIndex];
    gl_Position = vertices[gl_VertexIndex] * vec4(push.size.xy, 1, 1) + vec4(push.offset.xy, 1, 1);
}
