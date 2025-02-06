//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

// inputs
layout (location = 0) in vec2 inTex;

// outputs
layout (location = 0) out vec4 color;

// uniforms
layout (binding = 0, set = 0) uniform sampler2D sampler0;

layout( push_constant, std140 ) uniform constants {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec4 color;
} push;


void main()
{
    color = push.color * texture(sampler0, inTex);
}
