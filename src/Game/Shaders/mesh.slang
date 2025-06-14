#include "include/buffers.slang"


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
float3 CalcPointLight(PointLight light, float3 diffuseColor, float3 specularColor, float3 pos, float3 normal, float3 viewPosition, bool blinnPhong) {

    // distance between light and vertex/fragment
    const float distance = length(light.position.xyz - pos);

    if(distance > light.range) {
        // vertex/fragment out of light range.
        // no light contribution.
        return float3(0,0,0);
    }

    // light direction, from fragment to light
    const float3 lightDir = normalize(light.position.xyz - pos);

    // diffuse contribution
    // If the angle between both vectors is greater than 90 degrees then the
    // result of the dot product will actually become negative and we end up
    // with a negative diffuse component.
    const float diffuseFactor = max(dot(normal, lightDir), 0.0);
    if(diffuseFactor > 0) {

        float attenuation = clamp(1 - (distance * distance) / (light.range * light.range), 0, 1);
        attenuation *= lerp(attenuation, 1.0, 0.5);

        float3 diffuse = light.diffuse.rgb * diffuseFactor * push.diffuse.rgb * diffuseColor;

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
        float3 specular = light.specular.rgb * specularFactor * specularColor;

        diffuse *= attenuation;
        specular *= attenuation;
        return float3(diffuse + specular);
    }
    return float3(0,0,0);
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
            float attenuation = clamp(1 - (distanceLightToSurface * distanceLightToSurface) / (light.range * light.range), 0, 1);
            attenuation *= lerp(attenuation, 1.0, 0.5);

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

            diffuse *= attenuation;
            specular *= attenuation;
        }
    }
    return diffuse + specular;
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

    var MVP = mul(perFrame.viewProj, push.model);
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
    float4 result = perFrame.ambientLight * diffuseColor;
    for(uint i = 0; i < lightData.nbDirectionalLight; i++) {
        const float3 diffuseAndSpecular = CalcDirectionalLight(lightData.directionalLights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(diffuseAndSpecular, 1.0);
    }
    for(uint i = 0; i < lightData.nbLight; i++) {
        const float3 diffuseAndSpecular = CalcPointLight(lightData.lights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(diffuseAndSpecular, 1.0);
    }
    for(uint i = 0; i < lightData.nbSpotLight; i++) {
        const float3 diffuseAndSpecular = CalcSpotLight(lightData.spotLights[i], diffuseColor.rgb, specularColor.rgb, input.outPosition, normalWorldSpace, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(diffuseAndSpecular, 1.0);
    }
    //
    // tone mapping
    //
    //float exposure = 5.0f;
    //result.rgb = float3(1.0) - exp(-result.rgb * exposure);

    if(perFrame.useGammeCorrection) {
        result.rgb = pow(result.rgb, float3(1.0/perFrame.gamma));
    }

    PSOutput output;
    output.color = result;
    return output;
}
