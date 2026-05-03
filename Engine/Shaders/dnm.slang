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

struct Material {
    float4x4 model;
    uint albedoTextureIndex;
    uint roughnessTextureIndex;
    uint normalTextureIndex;
};

[[vk::push_constant]] Material pc;

[shader("vertex")]
VertexOut VertMain(VertexIn in) {
    VertexOut out;
    out.uv = in.uv;
    out.normal = in.normal.xyz;
    out.vertPos = mul(GetCamaraData().proj, mul(GetCamaraData().view, mul(pc.model, in.position)));
    return out;
};

[shader("fragment")]
float4 FragMain(VertexOut vertOut) : SV_Target {
    return GetBindless<ReadonlyImage2D>(pc.albedoTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
}