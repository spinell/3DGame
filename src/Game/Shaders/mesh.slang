struct DirectionalLight {
    float4 color;
    float4 direction;
};
struct PointLight {
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    float4 color;
    float4 position;
    float4 direction;
    float  range;
    float  cutOffInner;
    float  cutOffOuter;
};

float3 CalcDirectionalLight(DirectionalLight light, float3 diffuseColor, float3 specularColor, float3 pos, float3 normal, float3 viewPosition, bool blinnPhong) {
    // Negate the light direction.
    // The light direction should be the direction from the light to the object/vertex/fragment.
    // Lighting calculation expect the light direction to be from the object to the light.
    const float3 lightDir = -normalize(light.direction.xyz);

    // diffuse contribution
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    // Note: diffuseFactor will be in range [0,1]
    const float diffuseFactor = max(dot(normal, lightDir), 0.0);

    float3 diffuse  = float3(0.0, 0.0, 0.0);
    float3 specular = float3(0.0, 0.0, 0.0);

    // diffuse and specular light contribution are added only
    // if the light hit directly the surface (diffuseFactor > 0.0).
    if(diffuseFactor > 0.0) {
        diffuse = light.color.rgb * diffuseFactor * diffuseColor;

        // view direction, from fragment to camera
        const float3 viewDir = normalize(viewPosition - pos);

        // specular contribution
        float specularFactor = 0.0f;
        if(blinnPhong) {
            const float3 halfwayDir = normalize(lightDir + viewDir);
            specularFactor = pow(max(dot(normal, halfwayDir), 0.0), push.shininess);
        }else{
            const float3 reflectDir = reflect(-lightDir, normal);
            specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
        }
        specular = light.color.rgb * specularFactor * specularColor;
    }

    return float3(diffuse + specular);
}

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
float4 CalcPointLight(PointLight light, float3 diffuseColor, float3 specularColor, float3 pos, float3 normal, float3 viewPosition, bool blinnPhong) {

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
    float specularFactor = 0.0f;
    if(blinnPhong) {
        const float3 halfwayDir = normalize(lightDir + viewDir);
        specularFactor = pow(max(dot(normal, halfwayDir), 0.0), push.shininess);
    }else{
        const float3 reflectDir = reflect(-lightDir, normal);
        specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
    }
    float3 specular = light.specular.rgb * specularFactor * specularColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return float4(ambient + diffuse + specular, 1.0f);
}

// TODO: add attenuation
float3 CalcSpotLight(SpotLight light, float3 diffuseColor, float3 specularColor, float3 pos, float3 normal, float3 viewPosition, bool blinnPhong) {

    // distance between light and vertex/fragment
    const float distanceLightToSurface = length(light.position - float4(pos, 1.0f));

    if(distanceLightToSurface > light.range) {
        return float3(0,0,0);
    }

    // light diffuse and specular contribution
    float3 diffuse  = float3(0,0,0);
    float3 specular = float3(0,0,0);

    // light direction, from fragment to light
    const float3 lightDir = normalize(light.position.xyz - pos);
    const float theta     = dot(lightDir, normalize(-light.direction.xyz));
    if(theta > light.cutOffOuter) {
        const float diffuseFactor = max(dot(normal, lightDir), 0.0);

        // diffuse and specular light contribution are added only
        // if the light hit directly the surface (diffuseFactor > 0.0).
        if(diffuseFactor > 0.0) {

            diffuse = light.color.rgb * diffuseFactor * diffuseColor;

            // view direction, from fragment to camera
            const float3 viewDir = normalize(viewPosition - pos);

            // specular contribution
            float specularFactor = 0.0f;
            if(blinnPhong) {
                const float3 halfwayDir = normalize(lightDir + viewDir);
                specularFactor = pow(max(dot(normal, halfwayDir), 0.0), push.shininess);
            }else{
                const float3 reflectDir = reflect(-lightDir, normal);
                specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), push.shininess);
            }
            specular = light.color.rgb * specularFactor * specularColor;

            // smooth ligh between inner and outer cutoff
            const float epsilon   = light.cutOffInner - light.cutOffOuter;
            const float intensity = clamp((theta - light.cutOffOuter) / epsilon, 0.0, 1.0);
            diffuse *= intensity;
            specular *= intensity;
        }
    }
    return diffuse + specular;
}


struct PerFrameData
{
    float4x4 projection;
    float4x4 view;
    float3 viewPosition;
    float4 ambientLight;
    bool useBlinnPhong;
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
    uint       nbDirectionalLight;
    uint       nbSpotLight;
    PointLight       lights[512];
    DirectionalLight directionalLights[4];
    SpotLight        spotLights[4];
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
    const float3 normal    = normalize(input.outNormal);
    const float3 tangent   = normalize(input.outTangent);
    const float3 biTangent = normalize(cross(tangent, normal));
    const float3x3 TBN = float3x3(tangent, biTangent, normal);

    float3 normalTangentSpace = normalMap.Sample(input.outTex).rgb;
    normalTangentSpace = normalize(normalTangentSpace * 2 - 1);
    const float3 normalWorldSpace = normalize(mul(normalTangentSpace, TBN));

    const float4 diffuseColor  = diffuseMap.Sample(input.outTex);
    const float4 specularColor = specularMap.Sample(input.outTex);
    float4 result = float4 (0,0,0,0);
    for(uint i = 0; i < lightData.nbDirectionalLight; i++) {
        const float3 ambient            = lightData.directionalLights[i].color.rgb * diffuseColor.rgb * 0.2;
        const float3 diffuseAndSpecular = CalcDirectionalLight(lightData.directionalLights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(ambient, 1.0);
        result += float4(diffuseAndSpecular, 1.0);
    }
    for(uint i = 0; i < lightData.nbLight; i++) {
        result += CalcPointLight(lightData.lights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
    }
    for(uint i = 0; i < lightData.nbSpotLight; i++) {
        const float3 diffuseAndSpecular = CalcSpotLight(lightData.spotLights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(diffuseAndSpecular, 1.0);
    }
    PSOutput output;
    output.color = result;

    //float gamma = 2.2;
    //output.color.rgb = pow(output.color.rgb, float3(1.0/gamma));
    return output;
}
