#include "GE.slang"

struct VertexIn {
    float4 position;
    float4 normal;
    float2 uv;
};

struct VertexOut {
    float4 position : SV_Position;
    float4 vertPos;
    float3 normal;
    float2 uv;
};

struct Material {
    float4x4 model;
    uint albedoTextureIndex;
    uint roughnessTextureIndex;
    uint normalTextureIndex;
    uint emissiveTextureIndex;
    uint entityId;
};

[[vk::push_constant]] Material pc;

[shader("vertex")]
VertexOut VertMain(VertexIn in) {
    VertexOut out;
    out.uv = in.uv;
    out.normal = in.normal.xyz;
    out.vertPos = mul(pc.model, in.position);
    out.position = mul(GetCamaraData().proj, mul(GetCamaraData().view, out.vertPos));
    return out;
};

struct FragOut {
    float4 normal : SV_Target0; // r10g10b10a2
    float4 position : SV_Target1; // r16g16b16a16 is temp
    float4 albedoMetallic : SV_Target2;
    float4 emissive : SV_Target3; // a is intensive
    float roughness : SV_Target4;
    uint entityId : SV_Target5;
}

[shader("fragment")]
[earlydepthstencil]
FragOut FragMain(VertexOut vertOut) : SV_Target {
    FragOut out;
    out.position = vertOut.vertPos;
    out.normal.xyz = (normalize(vertOut.normal) * 0.5) + 0.5; // GetBindless<ReadonlyImage2D>(pc.normalTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.albedoMetallic.xyz = GetBindless<ReadonlyImage2D<float3>>(pc.albedoTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.albedoMetallic.w = 0.5; // GetBindless<ReadonlyImage2D>(pc.roughnessTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.roughness = GetBindless<ReadonlyImage2D<float>>(pc.roughnessTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.emissive = GetBindless<ReadonlyImage2D>(pc.emissiveTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.entityId = pc.entityId;
    return out;
}