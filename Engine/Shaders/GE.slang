[__AttributeUsage(_AttributeTargets::Var)]
struct Resource {};

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> textureHandles[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> imageHandles[];

[vk::binding(0, 3)]
__DynamicResource<__DynamicResourceKind.General> uniformBufferHandles[];

[vk::binding(0, 4)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];


export T getDescriptorFromHandle<T>(uint handle) where T : IOpaqueDescriptor
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
        else if (T.kind == DescriptorKind.ConstantBuffer)
            return uniformBufferHandles[handle].asOpaqueDescriptor<T>();
        else
            return uniformBufferHandles[handle].asOpaqueDescriptor<T>();
    case hlsl:
        return __loadResourceDescriptorFromHeap<T>(handle);

    default: static_assert(false, "unsupported shader target");
        return {};
    }
}

#define UniformBuffer [Resource] ConstantBuffer

#define ReadonlyBuffer [Resource] StructuredBuffer
#define WritableBuffer [Resource] RWStructuredBuffer

#define ReadonlyImage [Resource] Texture2D
#define WritableImage [Resource] RWTexture2D
#define WriteonlyImage [Resource] WTexture2D

#define Sampler [Resource] SamplerState
