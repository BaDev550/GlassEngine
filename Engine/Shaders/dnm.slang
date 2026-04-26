#include "GE.slang"

struct VertexIn {
    float4 position;
    float4 normal;
    float2 uv;
};

struct VertexOut {
    float4 vertPos : SV_Position;
    float3 normal;
    float2 uv;
};

[shader("vertex")]
VertexOut VertMain(VertexIn in) {
    VertexOut out;
    out.uv = in.uv;
    out.normal = in.normal.xyz;
    out.vertPos = in.position;
    out.vertPos = mul(__EngineGlobalCameraBuffer__.proj, mul(__EngineGlobalCameraBuffer__.view, in.position));
    return out;
};

struct Material {
    uint albedoTextureIndex;
    uint roughnessTextureIndex;
    uint normalTextureIndex;
};

[[vk::push_constant]] Material pc;

[shader("fragment")]
float4 FragMain(VertexOut vertOut) : SV_Target {
    // return float4(0);
    return GetBindless<ReadonlyImage2D>(pc.albedoTextureIndex).Sample(GetBindless<Sampler>(0), vertOut.uv);
}