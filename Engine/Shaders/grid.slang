#include "GE.slang"

static const float GridSize = 100.0f;
static const float3 Pos[4] = {
    float3(-1.0, 0.0, -1.0),
    float3( 1.0, 0.0, -1.0),
    float3( 1.0, 0.0,  1.0),
    float3(-1.0, 0.0,  1.0)
};

static const int Indices[6] = {
    0, 2, 1,
    2, 0, 3
};

struct VertexOutput {
    float4 position       : SV_Position;
    float  GridSize       : GRID_SIZE;
    float3 WorldPos       : WORLD_POS;
    float3 CameraPosition : CAMERA_POS;
};

[shader("vertex")]
VertexOutput VertMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    
    int Index = Indices[vertexID];
    float3 vPos3D = Pos[Index] * GridSize;

    vPos3D.x += GetCamaraData().pos.x;
    vPos3D.z += GetCamaraData().pos.z;

    output.GridSize = GridSize;
    output.CameraPosition = GetCamaraData().pos.xyz;
    output.WorldPos = vPos3D;

    float4 vPos = float4(vPos3D, 1.0f);
    
    output.position = mul(GetCamaraData().proj, mul(GetCamaraData().view, vPos));
    return output;
}

static const float GridCellSize = 0.15f;
static const float GridMinPixelsBetweenCells = 2.0f;
static const float4 GridColorThin = float4(0.15f, 0.15f, 0.15f, 1.0f);
static const float4 GridColorThick = float4(0.5f, 0.5f, 0.5f, 1.0f);

float log10(float x) {
    return log(x) / log(10.0f);
}

float satf(float x) {
    return saturate(x); // saturate replaces clamp(x, 0.0, 1.0)
}

float2 satv(float2 x) {
    return saturate(x);
}

float max2(float2 v) {
    return max(v.x, v.y);
}

float2 glsl_mod(float2 x, float y) {
    return x - y * floor(x / y);
}

[shader("fragment")]
float4 FragMain(VertexOutput input) : SV_Target
{
    float2 dvx = float2(ddx(input.WorldPos.x), ddy(input.WorldPos.x));
    float2 dvy = float2(ddx(input.WorldPos.z), ddy(input.WorldPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    float2 dudv = float2(lx, ly);
    float l = length(dudv);

    float LOD = max(0.0f, log10(l * GridMinPixelsBetweenCells / GridCellSize) + 1.0f);

    float GridCellSizeLod0 = GridCellSize * pow(10.0f, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0f;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0f;

    dudv *= 4.0f;

    float2 mod_div_dudv = glsl_mod(input.WorldPos.xz, GridCellSizeLod0) / dudv;
    float Lod0a = max2(float2(1.0f) - abs(satv(mod_div_dudv) * 2.0f - float2(1.0f)));

    mod_div_dudv = glsl_mod(input.WorldPos.xz, GridCellSizeLod1) / dudv;
    float Lod1a = max2(float2(1.0f) - abs(satv(mod_div_dudv) * 2.0f - float2(1.0f)));
    
    mod_div_dudv = glsl_mod(input.WorldPos.xz, GridCellSizeLod2) / dudv;
    float Lod2a = max2(float2(1.0f) - abs(satv(mod_div_dudv) * 2.0f - float2(1.0f)));

    float LOD_fade = frac(LOD); // fract -> frac
    float4 Color;

    if (Lod2a > 0.0f) {
        Color = GridColorThick;
        Color.a *= Lod2a;
    } else {
        if (Lod1a > 0.0f) {
            Color = lerp(GridColorThick, GridColorThin, LOD_fade); // mix -> lerp
            Color.a *= Lod1a;
        } else {
            Color = GridColorThin;
            Color.a *= (Lod0a * (1.0f - LOD_fade));
        }
    }
    
    float OpacityFalloff = (1.0f - satf(length(input.WorldPos.xz - input.CameraPosition.xz) / input.GridSize));

    Color.a *= OpacityFalloff;

    return Color;
}