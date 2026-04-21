import GE;

[shader("vertex")]
float4 Vert() : SV_Position {
    return float4(0);
}

[shader("fragment")]
float4 Frag() : SV_Target {
    return GetBindless<ReadonlyImage2D>(0).Load(int3(0,0,0));
}