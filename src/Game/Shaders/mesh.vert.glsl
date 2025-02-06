//
// Generate a triangle quad without vertex buffer.
//
#version 450

// input
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangentU;
layout (location = 3) in vec2 inTex;

// ouput
layout (location = 0) out vec2 outTex;


layout( set=0, binding=0, std140 ) uniform PerFrameData {
    mat4 projection;
    mat4 view;
};

layout( push_constant, std140 ) uniform constants {
    mat4 model;
    vec4 color;
} push;


void main() {
    outTex =inTex;
    gl_Position = projection * view * push.model * vec4(inPosition, 1.0f);
    gl_Position.y = -gl_Position.y;
}
