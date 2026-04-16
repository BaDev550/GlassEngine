[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> textureHandles[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> imageHandles[];

[vk::binding(0, 3)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];


export T GetBindless<T>(uint handle) where T : IOpaqueDescriptor
{
    __target_switch
    {
    case spirv:
        if (T.kind == DescriptorKind.Sampler)
            return samplerHandles[handle].asOpaqueDescriptor<T>();
        else if (T.kind == DescriptorKind.Texture) {
            if (T.This.descriptorAccess == T.This.descriptorAccess::Read) {
                return textureHandles[handle].asOpaqueDescriptor<T>();
            }
            else {
                return imageHandles[handle].asOpaqueDescriptor<T>();
            }
        }
        return imageHandles[handle].asOpaqueDescriptor<T>();

    case hlsl:
        return __loadResourceDescriptorFromHeap<T>(handle);

    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

__generic<T>
typealias UniformBuffer = ConstantBuffer<T>;

__generic<T>
typealias ReadonlyBuffer = StructuredBuffer<T>;
__generic<T>
typealias WritableBuffer = RWStructuredBuffer<T>;

typealias ReadonlyImage = Texture2D;
typealias WritableImage = RWTexture2D;
typealias WriteonlyImage = WTexture2D;

typealias Sampler = SamplerState;
