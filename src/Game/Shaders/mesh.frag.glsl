//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

// inputs
layout (location = 0) in vec2 inTex;

// outputs
layout (location = 0) out vec4 color;

// uniforms
layout (set = 0, binding = 1) uniform sampler2D sampler0;

layout( push_constant, std140 ) uniform constants {
    mat4 model;
    vec4 color;
} push;


void main()
{
    color = push.color * texture(sampler0, inTex);
}
