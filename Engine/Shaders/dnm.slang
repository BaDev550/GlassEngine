#include "GE.slang"

[shader("vertex")]
float4 VertMain() : SV_Position {
    return float4(0);
}

struct Material {
    uint albedoIndex;
    uint normalIndex;
    uint metallicRoughnessIndex;
};

struct PushConstants {
    Ptr<Material> materials;
};

[[vk::push_constant]] PushConstants pc;
[shader("fragment")]
float4 FragMain() : SV_Target {
    return GetBindless<ReadonlyImage2D>(
        pc.materials[5].albedoIndex
    ).Load(int3(0,0,0));
}