//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

layout (location = 0) in  vec2 inUV;
layout (location = 0) out vec4 color;

void main()
{
    color = vec4(inUV.xy, 0.0, 1.0);
}
