
struct PerFrameData
{
    float4x4 projection;
    float4x4 view;
    float3 viewPosition;
    float4 ambientLight;
}
struct PointLight {
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float constant;
    float linear;
    float quadratic;
};
struct PushData {
    float4x4 model;
    float4x4 normalMatrix;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float2 texScale;
    float shininess;
};

[[vk::binding(0, 0)]] ConstantBuffer<PerFrameData> perFrame;
[[vk::binding(1, 0)]] ConstantBuffer<PointLight> light;
[[vk::binding(2, 1)]] Sampler2D diffuseMap;
[[vk::binding(3, 1)]] Sampler2D specularMap;
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
    output.outTex = input.inTex * push.texScale;
    output.outNormal = mul(push.model, float4(input.inNormal, 0)).xyz;
    output.position = mul(MVP, float4(input.inPosition, 1.0f));
    return output;
}

float4 CalcPointLight(PointLight light, float3 normal, float3 pos, float3 viewDir, float2 outTex) {
    // light position, from fragment to light
    float3 lightDir = normalize(light.position.xyz - pos);

    float distance = length(light.position - float4(pos, 1.0f));
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    float3 ambient = light.ambient.rgb * push.ambient.rgb * diffuseMap.Sample(outTex).rgb;

    // diffuse
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    float3 diffuse = light.diffuse.rgb * (diffuseFactor * push.diffuse.rgb * diffuseMap.Sample(outTex).rgb);

    // specular
    float3 reflectDir = reflect(-lightDir, normal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
    float3 specular = light.specular.rgb * specularFactor * specularMap.Sample(outTex).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    float4 color = float4(ambient + diffuse + specular, 1.0f);

    return color;
}

[Shader("pixel")]
PSOutput ps_main(const VSOutput input) {
    float3 normal = normalize(input.outNormal);
    float3 viewDir = normalize(perFrame.viewPosition - input.outPosition);
    float4 result = CalcPointLight(light, normal, input.outPosition, viewDir, input.outTex);
    PSOutput output;
    output.color = result; //sampler0.Sample(input.outTex) * light.ambient;

    float gamma = 2.2;
    output.color.rgb = pow(output.color.rgb, float3(1.0/gamma));
    return output;
}
