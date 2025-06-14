#include "include/buffers.slang"

float3 CalcDirectionalLight(DirectionalLight light, float3 diffuseColor, float3 specularColor, float shininess, float3 pos, float3 normal, float3 viewPosition, bool blinnPhong) {
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
            specularFactor = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        }else{
            const float3 reflectDir = reflect(-lightDir, normal);
            specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        }
        specular = light.color.rgb * specularFactor * specularColor;
    }
    return float3(diffuse + specular);
}


struct TerrainSetting {
    float tessFactor0;
    float tessFactor1;
    float tessFactor2;
    float tessFactor3;
    float insideTessFactor0;
    float insideTessFactor1;
    float minDistance; // When distance is minimum, tessalation is maximum.
    float maxDistance; // When distance is maximum, tessalation is minimum.
    // Exponents for power of 2 tessellation.
    // The tessellation range is [2^(gMinTess), 2^(gMaxTess)].
    // Since the maximum tessellation is 64, this means gMaxTess can be at most 6 since 2^6 = 64.
    float minTess;
	float maxTess;
}

[[vk::binding(0, 1)]] Sampler2D heighMap;
[[vk::binding(1, 1)]] ConstantBuffer<TerrainSetting> terrainSetting;
[[vk::binding(2, 1)]] Sampler2D diffuseLayer[5];
[[vk::binding(3, 1)]] Sampler2D normalLayer[5];
[[vk::binding(4, 1)]] Sampler2D specularLayer[5];
[[vk::binding(5, 1)]] Sampler2D blendMap;

struct TerrainVertex {
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
    float2 boundY   : TEXCOORD1; // inBoundsY;
}

struct VSOutput {
    float3 posW   : POSITION;
    float2 uv     : TEXCOORD0;
    float2 boundY : TEXCOORD1; // inBoundsY;
}

[shader("vertex")]
VSOutput vs_main(const TerrainVertex input) {
    VSOutput output;
#if 1
	// Displace the patch corners to world space.
    // This is to make the eye to patch distance calculation more accurate.
    const float height = heighMap.SampleLevel(input.uv, 0).r;
    output.posW = float3(input.position.x, height, input.position.z);
#else
    output.posW = input.position;
#endif
    output.uv       = input.uv;
    output.boundY   = input.boundY;
    return output;
}

// =============================================================================
//                              Hull Shader
// =============================================================================

struct HSOutput {
	float3 position;
	float2 uv;
};

struct HS_CONSTANT_DATA_OUTPUT {
    float tessFactor[4]       : SV_TessFactor;
    float insideTessFactor[2] : SV_InsideTessFactor;
}

// Returns true if the box is completely behind (in negative half space) of plane.
bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane) {
    float3 n = abs(plane.xyz);

    // This is always positive.
    float r = dot(extents, n);

    // signed distance from center point to plane.
    float s = dot( float4(center, 1.0f), plane );

    // If the center point of the box is a distance of e or more behind the
    // plane (in which case s is negative since it is behind the plane),
    // then the box is completely in the negative half space of the plane.
    return (s + r) < 0.0f;
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6]) {
    for(int i = 0; i < 6; ++i) {
        // If the box is completely behind any of the frustum planes
        // then it is outside the frustum.
        if( AabbBehindPlaneTest(center, extents, frustumPlanes[i]) ) {
            return true;
        }
    }

    return false;
}

///
float CalcTessFactor(float3 position, float3 camPosition) {
#if 1
    const float d = distance(position, camPosition);
#else
    // max norm in xz plane (useful to see detail levels from a bird's eye).
    const float d = max( abs(position.x - camPosition.x), abs(position.z - camPosition.z) );
#endif
    // Clamps the specified value within the range of 0 to 1.
    const float s = saturate((d - terrainSetting.minDistance) / (terrainSetting.maxDistance - terrainSetting.minDistance));
    return pow(2, lerp(terrainSetting.maxTess, terrainSetting.minTess, s));
}

HS_CONSTANT_DATA_OUTPUT ConstantsHS(InputPatch<VSOutput, 4> patch) {
    HS_CONSTANT_DATA_OUTPUT output;

    //
    // Frustum culling
    //
    // We store the patch BoundsY in the first control point.
    const float minY = patch[0].boundY.x;
    const float maxY = patch[0].boundY.y;

    // Build axis-aligned bounding box.
    //  patch[2] is lower-left corner
    //  patch[1] is upper-right corner.
    const float3 vMin = float3(patch[2].posW.x, minY, patch[2].posW.z);
    const float3 vMax = float3(patch[1].posW.x, maxY, patch[1].posW.z);

	const float3 boxCenter  = 0.5f*(vMin + vMax);
	const float3 boxExtents = 0.5f*(vMax - vMin);
    if( AabbOutsideFrustumTest(boxCenter, boxExtents, perFrame.gWorldFrustumPlanes) ) {
        output.tessFactor[0] = 0.0f;
        output.tessFactor[1] = 0.0f;
        output.tessFactor[2] = 0.0f;
        output.tessFactor[3] = 0.0f;

        output.insideTessFactor[0] = 0.0f;
        output.insideTessFactor[1] = 0.0f;

        return output;
    }

    // It is important to do the tess factor calculation based on the
    // edge properties so that edges shared by more than one patch will
    // have the same tessellation factor.  Otherwise, gaps can appear.

    // compute the middle of each edges
    const float3 edge0 = 0.5f * (patch[0].posW + patch[2].posW);
    const float3 edge1 = 0.5f * (patch[0].posW + patch[1].posW);
    const float3 edge2 = 0.5f * (patch[1].posW + patch[3].posW);
    const float3 edge3 = 0.5f * (patch[2].posW + patch[3].posW);

    // compute the middle of the patch
    const float3 center  = 0.25f * (patch[0].posW + patch[1].posW +
                                    patch[2].posW + patch[3].posW);
    output.tessFactor[0] = CalcTessFactor(edge0, perFrame.viewPosition);
    output.tessFactor[1] = CalcTessFactor(edge1, perFrame.viewPosition);
    output.tessFactor[2] = CalcTessFactor(edge2, perFrame.viewPosition);
    output.tessFactor[3] = CalcTessFactor(edge3, perFrame.viewPosition);
    output.insideTessFactor[0] = CalcTessFactor(center, perFrame.viewPosition);
    output.insideTessFactor[1] = output.insideTessFactor[0];
    return output;
}

[shader("hull")]
[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantsHS")]
[maxtessfactor("64.0f")]
HSOutput hs_main(
    InputPatch<VSOutput, 4> inputPatch,
    uint i : SV_OutputControlPointID,
    uint patchId : SV_PrimitiveID)
{
    HSOutput output;

    // Pass through shader.
    output.position = inputPatch[i].posW;
    output.uv       = inputPatch[i].uv;
    return output;
}

// =============================================================================
//                              Domain Shader
// The domainshader is called for every vertex createdby the tesselator.
// It like the vertex shader after tesselation.
// The domain shader is invoked once per output coordinate from the Tessellator (TS) stage.
// The Domain Shader (DS) stage outputs the vertex position of a subdivided point in the output patch.
// =============================================================================

struct DSOutput {
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 uv   : TEXTURE0;
};

[shader("domain")]
[domain("quad")]
DSOutput ds_main(HS_CONSTANT_DATA_OUTPUT data, float2 uv : SV_DomainLocation, const OutputPatch<HSOutput, 4> quad) {
    DSOutput output;

    // Bilinear interpolation
    // Compute the position of generated vertices.
    const float3 v1 = lerp(quad[0].position, quad[1].position, uv.x);
    const float3 v2 = lerp(quad[2].position, quad[3].position, uv.x);
    const float3 p  = lerp(v1, v2, uv.y);

    // Bilinear interpolation
    // Compute the texture coordinate of generated vertices.
    const float2 uv1 = lerp(quad[0].uv, quad[1].uv, uv.x);
    const float2 uv2 = lerp(quad[2].uv, quad[3].uv, uv.x);
    const float2 tex = lerp(uv1, uv2, uv.y);

    // displacement mapping
    const float height = heighMap.SampleLevel(tex, 0).r;

    output.posW = float3(p.x, height, p.z);
    output.posH = mul(perFrame.viewProj, float4(output.posW, 1));
    output.uv   = tex;
    return output;
}

// =============================================================================
//                              Pixel Shader
// =============================================================================

[shader("pixel")]
float4 ps_main(DSOutput input) {

    const float4 color  = diffuseLayer[0].Sample(input.uv * 200);
    const float4 color1 = diffuseLayer[1].Sample(input.uv * 200);
    const float4 color2 = diffuseLayer[2].Sample(input.uv * 200);
    const float4 color3 = diffuseLayer[3].Sample(input.uv * 200);
    const float4 color4 = diffuseLayer[4].Sample(input.uv * 200);
    const float4 t      = blendMap.Sample(input.uv);

    const float4 specular  = specularLayer[0].Sample(input.uv * 200).rgba;
    const float4 specular1 = specularLayer[1].Sample(input.uv * 200).rgba;
    const float4 specular2 = specularLayer[2].Sample(input.uv * 200).rgba;
    const float4 specular3 = specularLayer[3].Sample(input.uv * 200).rgba;
    const float4 specular4 = specularLayer[4].Sample(input.uv * 200).rgba;

    // Blend the layers on top of each other.
    float4 texColor = color;
    texColor = lerp(texColor, color1, t.r);
    texColor = lerp(texColor, color2, t.g);
    texColor = lerp(texColor, color3, t.b);
    texColor = lerp(texColor, color4, t.a);

    float4 specularColor = specular;
    specularColor = lerp(specularColor, specular1, t.r);
    specularColor = lerp(specularColor, specular2, t.g);
    specularColor = lerp(specularColor, specular3, t.b);
    specularColor = lerp(specularColor, specular4, t.a);

    //
    // Estimate normal and tangent using central differences.
    //
    const float texelCellSpaceU = 1.0f / 2049.0;;
    const float texelCellSpaceV = 1.0f / 2049.0;;
    const float worldCellSpace  = 1;
    const float2 leftTex   = input.uv + float2(-texelCellSpaceU, 0.0f);
    const float2 rightTex  = input.uv + float2( texelCellSpaceU, 0.0f);
    const float2 bottomTex = input.uv + float2(0.0f,  texelCellSpaceV);
    const float2 topTex    = input.uv + float2(0.0f, -texelCellSpaceV);

    const float leftY   = heighMap.SampleLevel(leftTex, 0 ).r;
    const float rightY  = heighMap.SampleLevel(rightTex, 0 ).r;
    const float bottomY = heighMap.SampleLevel(bottomTex, 0 ).r;
    const float topY    = heighMap.SampleLevel(topTex, 0 ).r;

    // Compute the tangent, bitangent and normal of the surface
    const float3 tangentWorld   = normalize( float3 (2.0f * worldCellSpace, rightY - leftY, 0.0f) );
	const float3 biTangentWorld = normalize( float3 (0.0f, bottomY - topY, 2.0f * worldCellSpace) );
	const float3 normalWorld    = cross(tangentWorld, biTangentWorld);

    // Build the TBN matrice in world space
    const float3x3 TBN = float3x3( tangentWorld, biTangentWorld, normalWorld );

    const float3 normalLayer0 = mul(TBN, normalLayer[0].Sample(input.uv * 200).rgb * 2.0 - 1.0);
    const float3 normalLayer1 = mul(TBN, normalLayer[1].Sample(input.uv * 200).rgb * 2.0 - 1.0);
    const float3 normalLayer2 = mul(TBN, normalLayer[2].Sample(input.uv * 200).rgb * 2.0 - 1.0);
    const float3 normalLayer3 = mul(TBN, normalLayer[3].Sample(input.uv * 200).rgb * 2.0 - 1.0);
    const float3 normalLayer4 = mul(TBN, normalLayer[4].Sample(input.uv * 200).rgb * 2.0 - 1.0);

    // blend all normal map layer together
    float3 normal = normalLayer0;
    normal = lerp(normal, normalLayer1, t.r);
    normal = lerp(normal, normalLayer2, t.g);
    normal = lerp(normal, normalLayer3, t.b);
    normal = lerp(normal, normalLayer4, t.a);
    normal = normalize(normal);

    float4 result = perFrame.ambientLight * texColor;
    for(uint i = 0; i < lightData.nbDirectionalLight; i++) {
        const float3 diffuseAndSpecular = CalcDirectionalLight(lightData.directionalLights[i], texColor.rgb, specularColor.rgb , 25, input.posW, normal, perFrame.viewPosition, perFrame.useBlinnPhong);
        result += float4(diffuseAndSpecular, 1.0);
    }

    if(perFrame.useGammeCorrection) {
        result.rgb = pow(result.rgb, float3(1.0/perFrame.gamma));
    }

    return result;
}
