#include "GE.slang"

struct VertexIn {
    float4 position;
    float4 tangent;
    float4 normal;
    float2 uv;
};

struct VertexOut {
    float4 position : SV_Position;
    float4 vertPos;
    float3x3 tbn;
    float2 uv;
};

struct Material {
    float4x4 model;
    uint albedoTextureIndex;
    uint roughnessTextureIndex;
    uint normalTextureIndex;
    uint emissiveTextureIndex;
};

[[vk::push_constant]] Material pc;

[shader("vertex")]
VertexOut VertMain(VertexIn in) {
    VertexOut out;
    out.uv = in.uv;
    out.vertPos = mul(pc.model, in.position);

    var T = normalize(mul(pc.model, float4(in.tangent.xyz, 0)).xyz);
    let N = normalize(mul(pc.model, float4(in.normal.xyz, 0)).xyz);
    T = normalize(T - dot(T, N) * N);
    let B = cross(T, N);

    out.tbn = transpose(float3x3(T, B, N));

    let proj = GetSceneData().cameraData.proj;
    let view = GetSceneData().cameraData.view;
    let projView = mul(proj, view);

    out.position = mul(projView, out.vertPos);
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
    out.normal.xyz = 
        normalize(mul(vertOut.tbn,
            (GetBindless<ReadonlyImage2D<float3>>(pc.normalTextureIndex).Sample(GetDefaultSampler(), vertOut.uv) * 2) - 1)
            );
    // out.normal.xyz = (out.normal.xyz * 0.5) + 0.5;
    out.albedoMetallic.xyz = GetBindless<ReadonlyImage2D<float3>>(pc.albedoTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.albedoMetallic.w = 0.5; // GetBindless<ReadonlyImage2D>(pc.roughnessTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.roughness = 0.5;// GetBindless<ReadonlyImage2D<float>>(pc.roughnessTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.emissive = GetBindless<ReadonlyImage2D>(pc.emissiveTextureIndex).Sample(GetDefaultSampler(), vertOut.uv);
    out.entityId = 0;
    return out;
}