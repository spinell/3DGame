//
// Generate a triangle quad without vertex buffer.
//
#version 450

layout( push_constant, std140 ) uniform constants {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec4 color;
} push;

vec4 vertices[3] = {
    vec4(1,   1, 0, 1), // bottom right
    vec4(0,  -1, 0, 1), // top middle
    vec4(-1,  1, 0, 1), // bottom left
};

void main() {
    gl_Position = push.projection * push.view * push.model * vertices[gl_VertexIndex];
}
