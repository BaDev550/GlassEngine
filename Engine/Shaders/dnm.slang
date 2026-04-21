import GE;

[shader("vertex")]
float4 VertMain() : SV_Position {
    return float4(0);
}

[shader("fragment")]
float4 FragMain() : SV_Target {

    switch (materialIndex) {
    case: 0: DefaultMaterial();
    case: 1: ClearCoat();
    case: 2 : Skin();
    case: 3 : UserShader0();
    case: 4 : UserShader1();
    }

    return GetBindless<ReadonlyImage2D>(0).Load(int3(0,0,0));
}