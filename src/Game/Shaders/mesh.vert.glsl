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
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outTex;


layout( set=0, binding=0, std140 ) uniform PerFrameData {
    mat4 projection;
    mat4 view;
    vec3 viewPosition;
    vec4 ambientLight;
};


layout( push_constant, std140 ) uniform constants {
    mat4  model;
    mat4  normalMatrix;
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    vec2  texScale;
    float shininess;
} push;


void main() {

    outPosition = vec3(push.model * vec4(inPosition, 1.0f)); // world space position
    outNormal   = mat3(push.normalMatrix) * inNormal;
    outTex      = inTex * push.texScale;
    gl_Position = projection * view * push.model * vec4(inPosition, 1.0f);
    gl_Position.y = -gl_Position.y;
}
