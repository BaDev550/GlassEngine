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
    out.vertPos = in.position;
    out.uv = in.uv;
    out.normal = in.normal.xyz;
    return out;
};

struct Material {
    uint albedoIndex;
    uint samplerIndex;
};

[[vk::push_constant]] Material pc;

[shader("fragment")]
float4 FragMain(VertexOut vertOut) : SV_Target {
    const let sampler = GetBindless<Sampler>(pc.samplerIndex);

    return GetBindless<ReadonlyImage2D>(pc.albedoIndex).Sample(sampler, vertOut.uv);
}