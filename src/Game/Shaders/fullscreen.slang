//
// Generate fullscreen quad without vertex buffer.
// This will generate 3 vertices to fill the screen.
// Each vertice will have texture coordinate.
//

// Vertex shader ouputs
struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
}

// Pixel shader ouputs
struct PSOutput {
    float4 color : COLOR;
}

[shader("vertex")]
VSOutput vs_main(uint index: SV_VertexID) {
    VSOutput output;
#if 1
    // https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
    // Generate vertices in clock-wise order.
    // Vertex0 : pos(-1,-1) uv(0, 0) top-left
    // Vertex1 : pos( 3,-1) uv(2, 0) top-right
    // Vertex2 : pos(-1, 3) uv(0, 2) bottom-left
    output.uv = float2((index << 1) & 2, index & 2);
    output.position = float4(output.uv * 2.0f + -1.f, 0, 1);
#else
    // Generate vertices in counter-clock-wise order.
    // Vertex0 : pos(-1, 1) uv(0, 0) bottom-left
    // Vertex1 : pos( 3, 1) uv(2, 0) bottom-right
    // Vertex2 : pos(-1,-3) uv(0, 2) top-left
    output.uv = float2(index & 2, (index << 1) & 2);
    output.position = float4(output.uv * 2.0f + -1.f, 0, 1);
#endif
    return output;
}

[Shader("pixel")]
PSOutput ps_main(VSOutput input) {
    PSOutput output;
    output.color = float4(input.uv, 0.0, 1.0);
    return output;
}
