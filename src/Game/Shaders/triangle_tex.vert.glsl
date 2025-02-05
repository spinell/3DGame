//
// Generate a triangle quad without vertex buffer.
//
#version 450

layout (location = 0) out vec2 outUV;

layout( set=0, binding=1 ) uniform constants {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec4 color;
};

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
    gl_Position = projection * view * model * vertices[gl_VertexIndex];
}
