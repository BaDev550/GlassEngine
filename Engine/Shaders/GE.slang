#ifdef __GE_DXIL__
static const constexpr uint DxilCameraBufferIndex = 0;
static const constexpr uint DxilLightBufferIndex = 1;
#endif

// TODO: better name
class CameraData {
    float4x4 proj;
    float4x4 view;
};

// TODO: better name
class LightData {
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

#ifdef __GE_SPIRV__
[vk::binding(0, 0)]
ConstantBuffer<CameraData> __EngineGlobalCameraBuffer__;

[vk::binding(1, 0)]
ConstantBuffer<LightData> __EngineGlobalLightBuffer__;
#endif

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> __TextureHandles__[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> __ImageHandles__[];

[vk::binding(0, 3)]
__DynamicResource<__DynamicResourceKind.Sampler> __SamplerHandles__[];

T GetBindless<T>(uint handle)
    where T : IOpaqueDescriptor, IBindlessDescriptor {
    __target_switch
    {
    case spirv:
        if (T.kind == DescriptorKind.Sampler)
            return __SamplerHandles__[handle].asOpaqueDescriptor<T>();
        else if (T.kind == DescriptorKind.Texture) {
            if (T.This.descriptorAccess == T.This.descriptorAccess::Read) {
                return __TextureHandles__[handle].asOpaqueDescriptor<T>();
            }
            else {
                return __ImageHandles__[handle].asOpaqueDescriptor<T>();
            }
        }
        // unreachable
        return __ImageHandles__[handle].asOpaqueDescriptor<T>();

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
#ifdef __GE_SPIRV__
    return __EngineGlobalCameraBuffer__.asOpaqueDescriptor<UniformBuffer<CameraData, DefaultDataLayout>>();
#elifdef (__GE_DXIL__) 
    return __loadResourceDescriptorFromHeap<UniformBuffer<CameraData, DefaultDataLayout>>(DxilCameraBufferIndex);
#endif
}

LightData GetLightData() {
#ifdef __GE_SPIRV__
    return __EngineGlobalLightBuffer__.asOpaqueDescriptor<UniformBuffer<LightData, DefaultDataLayout>>();
#elifdef(__GE_DXIL__)
    return __loadResourceDescriptorFromHeap<UniformBuffer<LightData, DefaultDataLayout>>(DxilLightBufferIndex);
#endif
}