import GE;

[shader("vertex")]
float4 VertMain() : SV_Position {
    return float4(0);
}

[shader("fragment")]
float4 FragMain() : SV_Target {
    return GetBindless<ReadonlyImage2D>(0).Load(int3(0,0,0));
}