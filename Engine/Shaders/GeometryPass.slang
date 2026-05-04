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
    uint entityId;
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

struct FragOut {
    float4 normal : SV_Target0; // r10g10b10a2
    float4 albedoMetallic : SV_Target1;
    float4 emissive : SV_Target2; // a is intensive
    float roughness : SV_Target3;
    uint entityId : SV_Target4;
}

[shader("fragment")]
[earlydepthstencil]
FragOut FragMain(VertexOut vertOut) : SV_Target {
    FragOut out;
    out.normal = GetBindless<ReadonlyImage2D>(pc.normalTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.albedoMetallic = GetBindless<ReadonlyImage2D>(pc.albedoTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.entityId = pc.entityId;
    return out;
}