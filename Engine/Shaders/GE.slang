#pragma once
static const constexpr float PI = 3.14159265359;

static const constexpr uint __GE_Scene_Data_Index__ = 0;
static const constexpr uint __GE_Engine_Global_Data_Index__ = 1;

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float4x4 viewProj;
    float4x4 invViewProj;
    float4 pos;
    float2 extent;
    float2 _pad0;
}

struct PointLight {
    float4 position;
    uint color; // uint8_t4
    float intensity;
    float radius;
    float _pad0;
}

struct SpotLight {
    float4 position;
    float4 direction;
    uint color; // uint8_t4
    float intensity;
    float radius;
    float _pad0;
}

struct DirectionalLight {
    float4 direction;
    uint color; // uint8_t4
    float intensity;
    float _pad0;
    float _pad1;
}

struct SceneData {
    CameraData cameraData;

    DirectionalLight directionalLight;
    Ptr<PointLight, Access::Read> pointLightBuffer;
    Ptr<SpotLight, Access::Read> spotLightBuffer;
    
    uint lutIndex;
    uint enviromentMapIndex;

    uint gBufferDepth;
    uint gBufferNormal;
    uint gBufferAlbedoMetallic;
    uint gBufferEmissive;
    uint gBufferRoughness;
    uint gBufferEntityId;

    uint pointLightCount;
    uint spotLightCount;
};

// TODO: better name
struct EngineGlobalData {
    uint defaultSampler;
    uint whiteTexture;
    uint blackTexture;
};

interface IBindlessDescriptor {};

__generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias UniformBuffer = ConstantBuffer<T, L>;

__generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias ReadonlyBuffer = StructuredBuffer<T, L>;

__generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias WritableBuffer = RWStructuredBuffer<T, L>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImage1D = Texture1D<T>;
__generic<T : ITexelElement = float4>
typealias WritableImage1D = RWTexture1D<T>;
__generic<T : ITexelElement = float4>
typealias WriteonlyImage1D = WTexture1D<T>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImage1DArray = Texture1DArray<T>;
__generic<T : ITexelElement = float4>
typealias WritableImage1DArray = RWTexture1DArray<T>;
__generic<T : ITexelElement = float4>
typealias WriteonlyImage1DArray = WTexture1DArray<T>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImage2D = Texture2D<T>;
__generic<T : ITexelElement = float4>
typealias WritableImage2D = RWTexture2D<T>;
__generic<T : ITexelElement = float4>
typealias WriteonlyImage2D = WTexture2D<T>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImage2DArray = Texture2DArray<T>;
__generic<T : ITexelElement = float4>
typealias WritableImage2DArray = RWTexture2DArray<T>;
__generic<T : ITexelElement = float4>
typealias WriteonlyImage2DArray = WTexture2DArray<T>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImage3D = Texture3D<T>;
__generic<T : ITexelElement = float4>
typealias WritableImage3D = RWTexture3D<T>;
__generic<T : ITexelElement = float4>
typealias WriteonlyImage3D = WTexture3D<T>;

__generic<T : ITexelElement = float4>
typealias ReadonlyImageCube = TextureCube<T>;

typealias Sampler = SamplerState;

__generic<T : ITexelElement = float4>
extension ReadonlyImage1D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WritableImage1D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WriteonlyImage1D<T> : IBindlessDescriptor {}

__generic<T : ITexelElement = float4>
extension ReadonlyImage1DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WritableImage1DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WriteonlyImage1DArray<T> : IBindlessDescriptor {}

__generic<T : ITexelElement = float4>
extension ReadonlyImage2D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WritableImage2D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WriteonlyImage2D<T> : IBindlessDescriptor {}

__generic<T : ITexelElement = float4>
extension ReadonlyImage2DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WritableImage2DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WriteonlyImage2DArray<T> : IBindlessDescriptor {}

__generic<T : ITexelElement = float4>
extension ReadonlyImage3D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WritableImage3D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement = float4>
extension WriteonlyImage3D<T> : IBindlessDescriptor {}

__generic<T : ITexelElement = float4>
extension ReadonlyImageCube<T> : IBindlessDescriptor {}

extension Sampler : IBindlessDescriptor {}

[vk::binding(__GE_Scene_Data_Index__, 0)]
[require(spirv)]
ConstantBuffer<SceneData> __GE_Scene_Data__;

[vk::binding(__GE_Engine_Global_Data_Index__, 0)]
[require(spirv)]
ConstantBuffer<EngineGlobalData> __GE_Engine_Global_Data_Buffer__;

[vk::binding(0, 1)]
[require(spirv)]
__DynamicResource<__DynamicResourceKind.General> __GE_SpirV_Bindless_Readonly_Image_Array__[];

[vk::binding(0, 2)]
[require(spirv)]
__DynamicResource<__DynamicResourceKind.General> __GE_SpirV_Bindless_Writable_Image_Array__[];

[vk::binding(0, 3)]
[require(spirv)]
__DynamicResource<__DynamicResourceKind.Sampler> __GE_SpirV_Bindless_Sampler_Array__[];

T GetBindless<T>(uint handle)
    where T : IOpaqueDescriptor, IBindlessDescriptor {
    __target_switch
    {
    case spirv:
        if (T.kind == DescriptorKind.Sampler)
            return __GE_SpirV_Bindless_Sampler_Array__[handle].asOpaqueDescriptor<T>();
        else if (T.kind == DescriptorKind.Texture) {
            if (T.This.descriptorAccess == T.This.descriptorAccess::Read) {
                return __GE_SpirV_Bindless_Readonly_Image_Array__[handle].asOpaqueDescriptor<T>();
            }
            else {
                return __GE_SpirV_Bindless_Writable_Image_Array__[handle].asOpaqueDescriptor<T>();
            }
        }
        // unreachable
        return __GE_SpirV_Bindless_Writable_Image_Array__[handle].asOpaqueDescriptor<T>();

    case hlsl: {
        if (T.kind == DescriptorKind.Sampler) {
            return __loadSamplerDescriptorFromHeap<T>(handle);
        }
        else {
            return __loadResourceDescriptorFromHeap<T>(handle);
        }
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

SceneData GetSceneData() {
    __target_switch
    {
    case spirv:
        return __GE_Scene_Data__;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<SceneData, DefaultDataLayout>>(__GE_Scene_Data_Index__);
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

CameraData GetCamaraData() {
    return GetSceneData().cameraData;
}

EngineGlobalData GetEngineGlobalData() {
    __target_switch
    {
    case spirv:
        return __GE_Engine_Global_Data_Buffer__;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<EngineGlobalData, DefaultDataLayout>>(__GE_Engine_Global_Data_Index__);
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

Sampler GetDefaultSampler() {
    return GetBindless<Sampler>(GetEngineGlobalData().defaultSampler);
}

ReadonlyImage2D GetWhiteTexture() {
    return GetBindless<ReadonlyImage2D>(GetEngineGlobalData().whiteTexture);
}

ReadonlyImage2D GetBlackTexture() {
    return GetBindless<ReadonlyImage2D>(GetEngineGlobalData().blackTexture);
}

ReadonlyImage2D GetGBufferAlbedoMetallic() {
    return GetBindless<ReadonlyImage2D>(GetSceneData().gBufferAlbedoMetallic);
}

ReadonlyImage2D<float3> GetGBufferNormal() {
    return GetBindless<ReadonlyImage2D<float3>>(GetSceneData().gBufferNormal);
}

ReadonlyImage2D<float> GetGBufferDepth() {
    return GetBindless<ReadonlyImage2D<float>>(GetSceneData().gBufferDepth);
}

// TODO(0x): write
ReadonlyImage2D<uint> GetGBufferStencil() {
    return GetBindless<ReadonlyImage2D<uint>>(GetSceneData().gBufferDepth);
}

ReadonlyImage2D<float> GetGBufferRoughness() {
    return GetBindless<ReadonlyImage2D<float>>(GetSceneData().gBufferRoughness);
}

ReadonlyImage2D GetGBufferEmissive() {
    return GetBindless<ReadonlyImage2D>(GetSceneData().gBufferEmissive);
}

ReadonlyImage2D<uint> GetGBufferEntityId() {
    return GetBindless<ReadonlyImage2D<uint>>(GetSceneData().gBufferEntityId);
}

float3 CalcPositionFromDepth(float depth, float2 uv) {
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 worldPos = mul(GetCamaraData().invViewProj, clipPos);

    return worldPos.xyz / worldPos.w;
}

// github.com/KhronosGroup/ToneMapping/blob/main/PBR_Neutral/pbrNeutral.glsl
constexpr float3 PBRNeutralToneMapping(float3 color) {
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    let x = min(color.r, min(color.g, color.b));
    let offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    let peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression)
        return color;

    let d = 1.0 - startCompression;
    let newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    let g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return lerp(color, newPeak * float3(1, 1, 1), g);
}
