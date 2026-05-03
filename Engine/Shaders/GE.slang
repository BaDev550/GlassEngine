static const constexpr uint __GE_Global_Camera_Buffer_Index__ = 0;
static const constexpr uint __GE_Global_Light_Buffer_Index__ = 1;
static const constexpr uint __GE_Defaults_Buffer_Index__ = 2;

// TODO: better name
struct CameraData {
    float4x4 proj;
    float4x4 view;
    float3 pos;
};

// TODO: better name
struct LightData {
    uint GGXLut;
};

// TODO: better name
struct Defaults {
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

[vk::binding(__GE_Global_Camera_Buffer_Index__, 0)]
[require(spirv)]
ConstantBuffer<CameraData> __GE_Global_Camera_Buffer__;

[vk::binding(__GE_Global_Light_Buffer_Index__, 0)]
[require(spirv)]
ConstantBuffer<LightData> __GE_Global_Light_Buffer__;

[vk::binding(__GE_Defaults_Buffer_Index__, 0)]
[require(spirv)]
ConstantBuffer<Defaults> __GE_Defaults_Buffer__;

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

CameraData GetCamaraData() {
    __target_switch
    {
    case spirv:
        return __GE_Global_Camera_Buffer__;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<CameraData, DefaultDataLayout>>(__GE_Global_Camera_Buffer_Index__);
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

LightData GetLightData() {
    __target_switch
    {
    case spirv:
        return __GE_Global_Light_Buffer__;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<LightData, DefaultDataLayout>>(__GE_Global_Light_Buffer_Index__);
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

Defaults GetDefaults() {
    __target_switch
    {
    case spirv:
        return __GE_Defaults_Buffer__;
    case hlsl: {
        return __loadResourceDescriptorFromHeap<UniformBuffer<Defaults, DefaultDataLayout>>(__GE_Defaults_Buffer_Index__);
    }
    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

Sampler GetDefaultSampler() {
    return GetBindless<Sampler>(GetDefaults().defaultSampler);
}

ReadonlyImage2D GetWhiteTexture() {
    return GetBindless<ReadonlyImage2D>(GetDefaults().whiteTexture);
}

ReadonlyImage2D GetBlackTexture() {
    return GetBindless<ReadonlyImage2D>(GetDefaults().blackTexture);
}