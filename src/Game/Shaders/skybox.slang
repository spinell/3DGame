#include "include/buffers.slang"

[[vk::binding(0, 1)]] SamplerCube skybox;

struct VSOutput {
    float4 posH : SV_Position;
    float3 uvw  : TEXTURE;
}

[shader("vertex")]
VSOutput vs_main(float3 inPosL : POSITION) {
    // Remove the translation, the skybox is always at (0,0,0)
    float3x3 view = float3x3(perFrame.view);

    VSOutput output;
    // Set z = w = 1 (sky always on far plane)
    output.posH = mul( perFrame.projection, float4(mul(view, inPosL),1.0f ) ).xyww;

    // Use local vertex position as cubemap lookup vector.
    // Flip Z position to fallow Vulkan rules.
    // When sampling, a cubemap, Vulkan use right hand system with +z pointing into the screen.
    output.uvw = inPosL;
    output.uvw.z *= -1;
    return output;
}

[shader("pixel")]
float4 ps_main(VSOutput input) : SV_Target0 {

    float4 color = skybox.Sample(input.uvw);
    if(perFrame.useGammeCorrection) {
        color.rgb = pow(color.rgb, float3(1.0/perFrame.gamma));
    }
    return color;
}
