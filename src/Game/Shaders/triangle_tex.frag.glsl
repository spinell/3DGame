//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

layout (location = 0) in  vec2 inUV;
layout (location = 0) out vec4 outColor;

layout (binding = 0, set = 0) uniform sampler2D sampler0;

layout( set=0, binding=1 ) uniform constants {
    mat4 projection;
    mat4 model;
    vec4 color;
};


void main()
{
    outColor = color * texture(sampler0, inUV);
}
