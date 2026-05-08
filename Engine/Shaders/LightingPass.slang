#include "BRDF.slang"

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
float4 FragMain(const int2 pixelCoord: SV_Position) : SV_Target {
    let pos = GetGBufferPosition().Load(int3(pixelCoord, 0));
    let normal = (GetGBufferNormal().Load(int3(pixelCoord, 0)) * 2) - 1;
    let albedoMetallic = GetGBufferAlbedoMetallic().Load(int3(pixelCoord, 0));
    let roughness = GetGBufferRoughness().Load(int3(pixelCoord, 0));
    let emissive = GetGBufferEmissive().Load(int3(pixelCoord, 0));

    let alphaRoughness = roughness * roughness;
    let alphaRoughnessSq = alphaRoughness * alphaRoughness;
    let N = normalize(normal);
    let V = normalize(GetCamaraData().pos.xyz - pos);
    let F0 = lerp(float3(0.04), albedoMetallic.xyz, albedoMetallic.w);

    float3 color = {0,0,0};
    for (uint i = 0; i < GetSceneData().pointLightCount; i++) {
        let light = GetSceneData().pointLightBuffer[i];
    
        let distance = length(light.position.xyz - pos);
        let attenuation = 1.0 / (distance * distance);
        let radiance = unpackUnorm4x8ToFloat(light.color).xyz * attenuation;
    
        color += CalcLight(
            albedoMetallic.xyz,
            F0,
            alphaRoughnessSq,
            albedoMetallic.w,
            N,
            V,
            normalize(light.position.xyz - pos),
            radiance);
    }
    
    for (uint i = 0; i < GetSceneData().spotLightCount; i++) {
        let light = GetSceneData().spotLightBuffer[i];
    
        let distance = length(light.position.xyz - pos);
        let attenuation = 1.0 / (distance * distance);
        let radiance = unpackUnorm4x8ToFloat(light.color).xyz * attenuation;
    
        color += CalcLight(
            albedoMetallic.xyz, 
            F0, 
            alphaRoughnessSq, 
            albedoMetallic.w, 
            N, 
            V, 
            normalize(light.position.xyz - pos), 
            radiance);
    }

    { // directional light
        let light = GetSceneData().directionalLight;
    
        let radiance = unpackUnorm4x8ToFloat(light.color).xyz * light.intensity;

        color += CalcLight(
            albedoMetallic.xyz,
            F0,
            alphaRoughnessSq,
            albedoMetallic.w,
            N,
            V,
            normalize(light.direction.xyz),
            radiance);
    }

    return float4(PBRNeutralToneMapping(color), 1);
}