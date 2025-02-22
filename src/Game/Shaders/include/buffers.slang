// Set index used for per frame data
static const uint32_t SET_INDEX_PERFRAME = 0;

// TODO: Add
struct PerFrameData {
    float4x4 projection; // Camera projection matrix
    float4x4 view;       // Camera view matrix
    float4x4 viewProj;   // Camera pre computed view projection matrix
    float3 viewPosition; // Camera position
    float4 ambientLight;
    bool useBlinnPhong;
    bool useGammeCorrection;
    float gamma;
}

[[vk::binding(0, SET_INDEX_PERFRAME)]] ConstantBuffer<PerFrameData> perFrame;
