//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

layout (location = 0) out vec4 color;

layout( push_constant ) uniform constants {
    layout(offset=16) vec4 color;
} push;


void main()
{
    color = push.color;
}
