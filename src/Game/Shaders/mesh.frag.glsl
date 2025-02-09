//
// Generate a fullscreen quad without vertex buffer.
//
#version 450

// inputs
layout (location = 0) in vec3 inPosition; // position in world space
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTex;

// outputs
layout (location = 0) out vec4 outColor;

// uniforms
layout( set=0, binding=0, std140 ) uniform PerFrameData {
    mat4 projection;
    mat4 view;
    vec3 viewPosition;
    vec4 ambientLight;
};

struct PointLight {
    vec4  position;
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float constant;
    float linear;
    float quadratic;
};

layout( set=0, binding=1, std140 ) uniform LightData {
    PointLight light;
};

layout (set = 1, binding = 2) uniform sampler2D sampler0;

layout( push_constant, std140 ) uniform constants {
    mat4  model;
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    vec2  texScale;
    float shininess;
} push;


vec4 CalcPointLight(PointLight light, vec3 normal, vec3 pos, vec3 viewDir) {
    // light position, from fragment to light
    vec3 lightDir = normalize(light.position.xyz - inPosition);

    float distance    = length(light.position - vec4(inPosition, 1.0f));
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec4 ambient = light.ambient * push.ambient * texture(sampler0, inTex);

    // diffuse
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    vec4  diffuse       = light.diffuse * (diffuseFactor * push.diffuse * texture(sampler0, inTex));

    // specular
    vec3  reflectDir      = reflect(-lightDir, normal);
    float specularFactor  = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
    vec4  specular        = light.specular * (specularFactor * push.specular);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    vec4 color =  ambient + diffuse + specular;

    return color;
}

void main() {
    vec3 normal  = normalize(inNormal);
    vec3 viewDir = normalize(viewPosition - inPosition);
    vec4 result  = CalcPointLight(light, normal, inPosition, viewDir);
    outColor =  result;
#if 0
    vec3 normal   = normalize(inNormal);
    // revert the light direction
    // Shader expect the light direction comming from the light source.
    // Lighting calculation expect light direction pointing to the light.
    vec3 lightDir = -light.direction.xyz;

    // ambient
    vec4 ambient = light.ambient * push.ambient * texture(sampler0, inTex);

    // diffuse
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    vec4  diffuse       = light.diffuse * (diffuseFactor * push.diffuse * texture(sampler0, inTex));

    // specular
    vec3  viewDir         = normalize(viewPosition - inPosition);
    vec3  reflectDir      = reflect(-lightDir, normal);
    float specularFactor  = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
    vec4  specular        = light.specular * (specularFactor * push.specular);

    outColor = ambient + diffuse + specular;
#endif
}
