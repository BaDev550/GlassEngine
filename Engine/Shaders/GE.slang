static const constexpr uint __GE_Scene_Data_Index__ = 0;
static const constexpr uint __GE_Engine_Global_Data_Index__ = 1;

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float4 pos;
}

struct SceneData {
    CameraData cameraData;

    uint64_t pointLightBufferGPUAddress;
    uint64_t spotLightBufferGPUAddress;
    uint64_t directionalLightBufferGPUAddress;
    
    uint32_t lutIndex;
    uint32_t enviromentMapIndex;

    uint32_t pointLightCount;
    uint32_t spotLightCount;
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

typealias ReadonlyImage1D = Texture1D;
typealias WritableImage1D = RWTexture1D;
typealias WriteonlyImage1D = WTexture1D;

typealias ReadonlyImage1DArray = Texture1DArray;
typealias WritableImage1DArray = RWTexture1DArray;
typealias WriteonlyImage1DArray = WTexture1DArray;

typealias ReadonlyImage2D = Texture2D;
typealias WritableImage2D = RWTexture2D;
typealias WriteonlyImage2D = WTexture2D;

typealias ReadonlyImage2DArray = Texture2DArray;
typealias WritableImage2DArray = RWTexture2DArray;
typealias WriteonlyImage2DArray = WTexture2DArray;

typealias ReadonlyImage3D = Texture3D;
typealias WritableImage3D = RWTexture3D;
typealias WriteonlyImage3D = WTexture3D;

typealias ReadonlyImageCube = TextureCube;

typealias Sampler = SamplerState;

extension ReadonlyImage1D : IBindlessDescriptor {}
extension WritableImage1D : IBindlessDescriptor {}
extension WriteonlyImage1D : IBindlessDescriptor {}

extension ReadonlyImage1DArray : IBindlessDescriptor {}
extension WritableImage1DArray : IBindlessDescriptor {}
extension WriteonlyImage1DArray : IBindlessDescriptor {}

extension ReadonlyImage2D : IBindlessDescriptor {}
extension WritableImage2D : IBindlessDescriptor {}
extension WriteonlyImage2D : IBindlessDescriptor {}

extension ReadonlyImage2DArray : IBindlessDescriptor {}
extension WritableImage2DArray : IBindlessDescriptor {}
extension WriteonlyImage2DArray : IBindlessDescriptor {}

extension ReadonlyImage3D : IBindlessDescriptor {}
extension WritableImage3D : IBindlessDescriptor {}
extension WriteonlyImage3D : IBindlessDescriptor {}

extension ReadonlyImageCube : IBindlessDescriptor {}

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
    __target_switch
    {
    case spirv:
        return __GE_Scene_Data__.cameraData;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<SceneData, DefaultDataLayout>>(__GE_Scene_Data_Index__).cameraData;
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
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