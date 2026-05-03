#include "GE.slang"

static const constexpr float2 Pos[3] = {
    float2(-10.0, -10.0),
    float2(-10.0, 10.0),
    float2( 40.0, 10.0)
};

[shader("vertex")]
float4 VertMain(uint vertIndex: SV_VertexID) : SV_Position {
    return float4(Pos[vertIndex], 0.5, 1);
};

[shader("fragment")]
[earlydepthstencil]
float4 FragMain(float2 pixelCoord: SV_Position) : SV_Target {
    return float4(GetGBufferAlbedoMetallic().Load(int3((int)pixelCoord.x, (int)pixelCoord.y, 0)).xyz, 1);
}