#pragma once
#include "GE.slang"

// learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.1.lighting/1.1.pbr.fs
float3 FresnelSchlick(const float3 F0, const float VdotH) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotH, 0.1, 1.0), 5.0);
}

// github.com/KhronosGroup/glTF-Sample-Renderer/blob/bec106e53da4a6a398aa3205f0f96563519a657e/source/Renderer/shaders/brdf.glsl
float V_GGX(const float NdotL, const float NdotV, const float alphaRoughnessSq) {
    const float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    const float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    const float GGX = GGXV + GGXL;
    return GGX > 0.0 ? 0.5 / GGX : 0.0;
}

// github.com/KhronosGroup/glTF-Sample-Renderer/blob/bec106e53da4a6a398aa3205f0f96563519a657e/source/Renderer/shaders/brdf.glsl
float D_GGX(const float NdotH, const float alphaRoughnessSq) {
    const float f = (NdotH * NdotH) * (alphaRoughnessSq - 1.0) + 1.0;
    return alphaRoughnessSq / (PI * f * f);
}

// github.com/KhronosGroup/glTF-Sample-Renderer/blob/bec106e53da4a6a398aa3205f0f96563519a657e/source/Renderer/shaders/brdf.glsl
float3 BRDF_specularGGX(const float alphaRoughnessSq, const float NdotL, const float NdotV, const float NdotH) {
    return float3(V_GGX(NdotL, NdotV, alphaRoughnessSq) * D_GGX(NdotH, alphaRoughnessSq));
}

// TODO: oren nayar diffuse brdf
float3 BRDF_diffuseLambert(const float3 albedo) {
    return albedo / PI;
}

float3 CalcLight(const float3 albedo, const float3 F0, const float alphaRoughnessSq, const float metallic, const float3 N, const float3 V, const float3 L, const float3 radiance) {
    let H = normalize(V + L);
    let VdotH = max(dot(V, H), 0.0f);
    let NdotL = max(dot(N, L), 0.0f);
    let NdotV = max(dot(N, V), 0.0f);
    let NdotH = max(dot(N, H), 0.0f);

    let F = FresnelSchlick(F0, VdotH);
    
    let specular = F * BRDF_specularGGX(alphaRoughnessSq, NdotL, NdotV, NdotH);
    let diffuse = BRDF_diffuseLambert(albedo);

    let kS = F;
    let kD = (float3(1) - kS) * (1 - metallic);
    let result = ((kD * diffuse) + specular) * radiance * NdotL;

    return result;
}