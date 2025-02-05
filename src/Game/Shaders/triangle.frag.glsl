//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

layout (location = 0) out vec4 color;

layout( push_constant, std140 ) uniform constants {
    mat4 projection;
    mat4 model;
    vec4 color;
} push;


void main()
{
    color = push.color;
}
