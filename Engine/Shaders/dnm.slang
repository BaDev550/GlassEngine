#include "GE.slang"

Texture2D texture;

[shader("fragment")]
float4 dnm(uint index: _1, int3 coord: _2) : SV_Target {
    return getDescriptorFromHandle<Texture2D>(index).Load(coord) + texture.Load(coord);
}