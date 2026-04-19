#include "GE.slang"

Texture2D texture;

[shader("vertex")]
float4 Vert() : SV_Position {
    return float4(0);
}

[shader("fragment")]
float4 Frag() : SV_Target {
    return float4(0);
}