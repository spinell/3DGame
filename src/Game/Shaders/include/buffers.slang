// Set index used for per frame data
static const uint32_t SET_INDEX_PERFRAME = 0;

// TODO: Add
struct PerFrameData {
    float4x4 projection; // Camera projection matrix
    float4x4 view;       // Camera view matrix
    float4x4 viewProj;   // Camera pre computed view projection matrix
    float3 viewPosition; // Camera position
    float4 gWorldFrustumPlanes[6];
    float4 ambientLight;
    bool useBlinnPhong;
    bool useGammeCorrection;
    float gamma;
}

[[vk::binding(0, SET_INDEX_PERFRAME)]] ConstantBuffer<PerFrameData> perFrame;

struct DirectionalLight {
    float4 color;
    float4 direction;
};
struct PointLight {
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float  range;
    float  intensity; // not used, to remove ?
    float  constant;  // not used, to remove ?
    float  linear;    // not used, to remove ?
    float  quadratic; // not used, to remove ?
};
struct SpotLight {
    float4 color;
    float4 position;
    float4 direction;
    float  range;
    float  cutOffInner;
    float  cutOffOuter;
};

struct LightData {
    uint       nbLight;
    uint       nbDirectionalLight;
    uint       nbSpotLight;
    PointLight       lights[512];
    DirectionalLight directionalLights[4];
    SpotLight        spotLights[4];
};

[[vk::binding(1, 0)]] ConstantBuffer<LightData>   lightData;
