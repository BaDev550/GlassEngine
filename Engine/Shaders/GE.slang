module GE;
static const constexpr uint DxilCameraBufferIndex = 0;
static const constexpr uint DxilLightBufferIndex = 1;

// TODO: better name
public class CameraData {
};

// TODO: better name
public class LightData {
};

public interface IBindlessDescriptor {};

public __generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias UniformBuffer = ConstantBuffer<T, L>;

public __generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias ReadonlyBuffer = StructuredBuffer<T, L>;

public __generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias WritableBuffer = RWStructuredBuffer<T, L>;

public typealias ReadonlyImage1D = Texture1D;
public typealias WritableImage1D = RWTexture1D;
public typealias WriteonlyImage1D = WTexture1D;

public typealias ReadonlyImage1DArray = Texture1DArray;
public typealias WritableImage1DArray = RWTexture1DArray;
public typealias WriteonlyImage1DArray = WTexture1DArray;

public typealias ReadonlyImage2D = Texture2D;
public typealias WritableImage2D = RWTexture2D;
public typealias WriteonlyImage2D = WTexture2D;

public typealias ReadonlyImage2DArray = Texture2DArray;
public typealias WritableImage2DArray = RWTexture2DArray;
public typealias WriteonlyImage2DArray = WTexture2DArray;

public typealias ReadonlyImage3D = Texture3D;
public typealias WritableImage3D = RWTexture3D;
public typealias WriteonlyImage3D = WTexture3D;

public typealias ReadonlyImageCube = TextureCube;

public typealias Sampler = SamplerState;

public extension ReadonlyImage1D : IBindlessDescriptor {}
public extension WritableImage1D : IBindlessDescriptor {}
public extension WriteonlyImage1D : IBindlessDescriptor {}

public extension ReadonlyImage1DArray : IBindlessDescriptor {}
public extension WritableImage1DArray : IBindlessDescriptor {}
public extension WriteonlyImage1DArray : IBindlessDescriptor {}

public extension ReadonlyImage2D : IBindlessDescriptor {}
public extension WritableImage2D : IBindlessDescriptor {}
public extension WriteonlyImage2D : IBindlessDescriptor {}

public extension ReadonlyImage2DArray : IBindlessDescriptor {}
public extension WritableImage2DArray : IBindlessDescriptor {}
public extension WriteonlyImage2DArray : IBindlessDescriptor {}

public extension ReadonlyImage3D : IBindlessDescriptor {}
public extension WritableImage3D : IBindlessDescriptor {}
public extension WriteonlyImage3D : IBindlessDescriptor {}

public extension ReadonlyImageCube : IBindlessDescriptor {}

public extension Sampler : IBindlessDescriptor {}

[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind.General> __EngineGlobalCameraBuffer__;

[vk::binding(1, 0)]
__DynamicResource<__DynamicResourceKind.General> __EngineGlobalLightBuffer__;

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> __TextureHandles__[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> __ImageHandles__[];

[vk::binding(0, 3)]
__DynamicResource<__DynamicResourceKind.Sampler> __SamplerHandles__[];

public extern T GetBindless<T>(uint handle)
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

// TODO: better name
public CameraData GetCamaraData() {
    __target_switch
    {
    case spirv:
        return __EngineGlobalCameraBuffer__.asOpaqueDescriptor<UniformBuffer<CameraData, DefaultDataLayout>>();

    case hlsl:
        return __loadResourceDescriptorFromHeap<UniformBuffer<CameraData, DefaultDataLayout>>(DxilCameraBufferIndex);

    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

// TODO: better name
public LightData GetLightData() {
    __target_switch
    {
    case spirv:
        return __EngineGlobalLightBuffer__.asOpaqueDescriptor<UniformBuffer<LightData, DefaultDataLayout>>();

    case hlsl:
        return __loadResourceDescriptorFromHeap<UniformBuffer<LightData, DefaultDataLayout>>(DxilLightBufferIndex);

    default: static_assert(false, "unsupported shader target");
        return {};
    }
}