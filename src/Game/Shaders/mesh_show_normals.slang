#include "include/buffers.slang"

struct PushData {
    float4x4 model;
    float4x4 normalMatrix;
};
[vk::push_constant]   PushData push;

//
// Vertex Shader
//

struct VSInput {
    float3 position;
    float3 normal;
    float3 tangent;
}

struct VSOutput {
    float3 position; // world space position
    float3 normal;
    float3 tangent;
    float3 bitangent;
}

[Shader("vertex")]
VSOutput vs_main(const VSInput input) {
    VSOutput output;
    output.position  = mul(push.model, float4(input.position, 1.0f)).xyz;
    output.normal    = mul(push.normalMatrix, float4(input.normal, 0)).xyz;
    output.tangent   = mul(push.normalMatrix, float4(input.tangent, 0)).xyz;
    output.bitangent = cross(output.tangent, output.normal);
    return output;
}

//
// Geometry Shader
//

struct GSOutput {
    float4 position : SV_Position;
    float4 color    : COLOR0;
}

[Shader("geometry")]
[maxvertexcount(6)]
void gs_main(triangle VSOutput input[3], inout LineStream<GSOutput> outputStream) {
	GSOutput o;

    float normal_length = 0.05f;

    for(int i = 0; i < 3; i++) {
        float3 P = input[i].position.xyz;
        float3 N = normalize(input[i].normal.xyz);
        float3 T = normalize(input[i].tangent.xyz);
        float3 B = normalize(input[i].bitangent.xyz);

        //
        // Display normal
        //
        o.position  = mul(perFrame.viewProj, float4(P, 1.0));
        o.color     = float4(0.0f, 1.0f, 0.0f, 1.0f);
        outputStream.Append(o);

        o.position  = mul(perFrame.viewProj, float4(P + N * normal_length, 1.0));
        o.color     = float4(0.0f, 1.0f, 0.0f, 1.0f);
        outputStream.Append(o);

        outputStream.RestartStrip();

        //
        // Display tangent
        //
        o.position  = mul(perFrame.viewProj, float4(P, 1.0));
        o.color     = float4(1.0f, 0.0f, 0.0f, 1.0f);
        outputStream.Append(o);

        o.position  = mul(perFrame.viewProj, float4(P + T * normal_length, 1.0));
        o.color     = float4(1.0f, 0.0f, 0.0f, 1.0f);
        outputStream.Append(o);

        outputStream.RestartStrip();

        //
        // Display bitangent
        //
        o.position  = mul(perFrame.viewProj, float4(P, 1.0));
        o.color     = float4(0.0f, 0.0f, 1.0f, 1.0f);
        outputStream.Append(o);

        o.position  = mul(perFrame.viewProj, float4(P + B * normal_length, 1.0));
        o.color     = float4(0.0f, 0.0f, 1.0f, 1.0f);
        outputStream.Append(o);

        outputStream.RestartStrip();
    }
}

//
// Pixel Shader
//

[Shader("pixel")]
float4 ps_main(const GSOutput input) {
    return input.color;
}
