
struct PointLight {
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float constant;
    float linear;
    float quadratic;
};

// @brief Compute the light contribution of a point light.
// @param light         The light use to compute lighting.
// @param diffuseColor  The diffuse color of the surface.
// @param specularColor The specular color of the surface.
// @param pos           The position of the vertex/fragment.
// @param normal        The normal vector of the surface/fragment.
// @param viewPosition  The view position (Camera direction)
// @return The light color contribution.
//
// @Note \p pos, \p normal and \p viewPosition must be in the same space.
//
float4 CalcPointLight(PointLight light, float3 diffuseColor, float3 specularColor, float3 pos, float3 normal, float3 viewPosition) {

    // view direction, from fragment to camera
    const float3 viewDir = normalize(viewPosition - pos);

    // light direction, from fragment to light
    const float3 lightDir = normalize(light.position.xyz - pos);

    // distance between light and vertex/fragment
    const float distance = length(light.position - float4(pos, 1.0f));

    const float attenuation = 1.0 / (light.constant +
                                     light.linear * distance +
                                     light.quadratic * (distance * distance));

    // ambient contribution
    float3 ambient = light.ambient.rgb * push.ambient.rgb * diffuseColor;

    // diffuse contribution
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    const float diffuseFactor = max(dot(normal, lightDir), 0.0);
    float3 diffuse = light.diffuse.rgb * diffuseFactor * push.diffuse.rgb * diffuseColor;

    // specular contribution
    const float3 reflectDir = reflect(-lightDir, normal);
    const float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
    float3 specular = light.specular.rgb * specularFactor * specularColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return float4(ambient + diffuse + specular, 1.0f);
}

struct PerFrameData
{
    float4x4 projection;
    float4x4 view;
    float3 viewPosition;
    float4 ambientLight;
}

struct PushData {
    float4x4 model;
    float4x4 normalMatrix;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float2 texScale;
    float shininess;
};

struct LightData {
    uint       nbLight;
    PointLight lights[512];
};

[[vk::binding(0, 0)]] ConstantBuffer<PerFrameData> perFrame;
[[vk::binding(1, 0)]] ConstantBuffer<LightData>   lightData;
[[vk::binding(2, 1)]] Sampler2D diffuseMap;
[[vk::binding(3, 1)]] Sampler2D specularMap;
[[vk::binding(4, 1)]] Sampler2D normalMap;
[vk::push_constant]   PushData push;

struct VSInput {
    float3 inPosition;
    float3 inNormal;
    float3 inTangentU;
    float2 inTex;
}

struct VSOutput {
    float4 position : SV_Position;
    float3 outPosition;
    float3 outNormal;
    float3 outTangent;
    float2 outTex;
}

struct PSOutput {
    float4 color : COLOR0;
}

[Shader("vertex")]
VSOutput vs_main(const VSInput input) {
    VSOutput output;

    var MVP = mul(perFrame.projection, mul(perFrame.view, push.model));
    var worldPos = mul(push.model, float4(input.inPosition, 1.0f));

    output.outPosition = worldPos.xyz; // world space position
    output.outTex      = input.inTex * push.texScale;
    output.outNormal   = mul(push.normalMatrix, float4(input.inNormal, 0)).xyz;
    output.outTangent  = mul(push.normalMatrix, float4(input.inTangentU, 0)).xyz;
    output.position    = mul(MVP, float4(input.inPosition, 1.0f));
    return output;
}

[Shader("pixel")]
PSOutput ps_main(const VSOutput input) {
    float3 normal    = normalize(input.outNormal);
    float3 tangent   = normalize(input.outTangent);
    float3 biTangent = normalize(cross(tangent, normal));
    float3x3 TBN = float3x3(tangent, biTangent, normal);

    float3 normalTangentSpace = normalMap.Sample(input.outTex).rgb;
    normalTangentSpace = normalize(normalTangentSpace * 2 - 1);
    float3 normalWorldSpace = normalize(mul(normalTangentSpace, TBN));

    float4 diffuseColor  = diffuseMap.Sample(input.outTex);
    float4 specularColor = specularMap.Sample(input.outTex);
    float4 result = float4 (0,0,0,0);
    for(uint i = 0; i < lightData.nbLight; i++) {
        result += CalcPointLight(lightData.lights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition);
    }
    PSOutput output;
    output.color = result;

    //float gamma = 2.2;
    //output.color.rgb = pow(output.color.rgb, float3(1.0/gamma));
    return output;
}
