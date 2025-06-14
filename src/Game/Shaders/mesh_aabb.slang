#include "include/buffers.slang"

struct PushData {
    float4x4 model;   // model transformation
    float3   aabbMin; // the aabb min
    float3   aabbMax; // the aabb max
    float3   color;   // color of  the aabb
};

[vk::push_constant] PushData push;

struct VSOutput {
    float4 aabbMin : POSITION0;
    float4 aabbMax : POSITION1;
}

// Vertex shader that does nothing.
// The geometry shader generate the bounding box
[Shader("vertex")]
VSOutput vs_main() {
    VSOutput output;
    output.aabbMin = float4(push.aabbMin, 1.0f);
    output.aabbMax = float4(push.aabbMax, 1.0f);
    return output;
}

//
// Geometry shader that generate the bounding box based on the min and max.
//
struct GSOutput {
    float4 position : SV_POSITION;
};

[Shader("geometry")]
[maxvertexcount(18)]
void gs_main(point const VSOutput input[1], inout LineStream<GSOutput> stream) {
    const float4 min = input[0].aabbMin;
    const float4 max = input[0].aabbMax;

    const float4x4 MVP = mul(perFrame.viewProj, push.model);

    GSOutput output;

    //=================================================
    //	Generate the top of the box in one line strip
    //=================================================
    // top - left - back
    output.position  = mul(MVP, float4(min.x, max.y, min.z, 1.0f));
    stream.Append(output);

    // top - left - front
    output.position  = mul(MVP, float4(min.x, max.y, max.z, 1.0f));
    stream.Append(output);

    // top - right - front
    output.position  = mul(MVP, float4(max.x, max.y, max.z, 1.0f));
    stream.Append(output);

    // top - right - back
    output.position  = mul(MVP, float4(max.x, max.y, min.z, 1.0f));
    stream.Append(output);

    // top - left - back
    output.position  = mul(MVP, float4(min.x, max.y, min.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();

    //=================================================
    //	Generate the bottom of the box in one line strip
    //=================================================
    // bottom - left - back
    output.position  = mul(MVP, float4(min.x, min.y, min.z, 1.0f));
    stream.Append(output);

    // bottom - left - front
    output.position  = mul(MVP, float4(min.x, min.y, max.z, 1.0f));
    stream.Append(output);

    // bottom - right - front
    output.position  = mul(MVP, float4(max.x, min.y, max.z, 1.0f));
    stream.Append(output);

    // bottom - right - back
    output.position  = mul(MVP, float4(max.x, min.y, min.z, 1.0f));
    stream.Append(output);

    // bottom - left - back
    output.position  = mul(MVP, float4(min.x, min.y, min.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();

    //======================================
    //	Generate the four edges of the box
    //======================================

    // top - left - back
    output.position  = mul(MVP, float4(min.x, max.y, min.z, 1.0f));
    stream.Append(output);

    // bottom - left - back
    output.position  = mul(MVP, float4(min.x, min.y, min.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();

    // top - right - back
    output.position  = mul(MVP, float4(max.x, max.y, min.z, 1.0f));
    stream.Append(output);

    // bottom - right - back
    output.position  = mul(MVP, float4(max.x, min.y, min.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();

    // top - left - front
    output.position  = mul(MVP, float4(min.x, max.y, max.z, 1.0f));
    stream.Append(output);

    // bottom - left - front
    output.position  = mul(MVP, float4(min.x, min.y, max.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();

    // top - right - front
    output.position  = mul(MVP, float4(max.x, max.y, max.z, 1.0f));
    stream.Append(output);

    // bottom - right - front
    output.position  = mul(MVP, float4(max.x, min.y, max.z, 1.0f));
    stream.Append(output);

    stream.RestartStrip();
}

struct PSOutput {
    float4 color : COLOR0;
}

// Pixel shader, output the color received from the push constant.
[Shader("pixel")]
PSOutput ps_main(const VSOutput input) {

    float4 result = float4(push.color, 1.0);
    if(perFrame.useGammeCorrection) {
        result.rgb = pow(result.rgb, float3(1.0/perFrame.gamma));
    }

    PSOutput output;
    output.color = result;
    return output;
}
