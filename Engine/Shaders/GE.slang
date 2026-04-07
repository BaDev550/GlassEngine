#pragma warning(disable: 39001)
[__AttributeUsage(_AttributeTargets::Var)]
struct Resource {};

#define UniformBuffer [Resource] ConstantBuffer

#define ReadonlyBuffer [Resource] StructuredBuffer
#define WritableBuffer [Resource] RWStructuredBuffer

#define ReadonlyImage [Resource] Texture2D
#define WritableImage [Resource] RWTexture2D
#define WriteonlyImage [Resource] WTexture2D

#define Sampler [Resource] SamplerState

#if defined(__TARGET_SPIRV__)
#define GetBindless(type, index) __bindless_##type##__[index]
#elif defined(__TARGET_DXIL__)
#define GetBindless(type, index) __loadResourceDescriptorFromHeap<type>(index)
#endif

#if defined(__TARGET_SPIRV__)
[[vk_binding(0, 1)]] Texture1D __bindless_Texture1D__[];
[[vk_binding(0, 1)]] Texture1DArray __bindless_Texture1DArray__[];
[[vk_binding(0, 1)]] Texture2D __bindless_Texture2D__[];
[[vk_binding(0, 1)]] Texture2DArray __bindless_Texture2DArray__[];
[[vk_binding(0, 1)]] Texture3D __bindless_Texture3D__[];
[[vk_binding(0, 1)]] TextureCube __bindless_TextureCube__[];
[[vk_binding(0, 1)]] TextureCubeArray __bindless_TextureCubeArray__[];

[[vk_binding(0, 2)]] RWTexture1D __bindless_RWTexture1D__[];
[[vk_binding(0, 2)]] RWTexture1DArray __bindless_RWTexture1DArray__[];
[[vk_binding(0, 2)]] RWTexture2D __bindless_RWTexture2D__[];
[[vk_binding(0, 2)]] RWTexture2DArray __bindless_RWTexture2DArray__[];
[[vk_binding(0, 2)]] RWTexture3D __bindless_RWTexture3D__[];

[[vk_binding(0, 2)]] WTexture1D __bindless_WTexture1D__[];
[[vk_binding(0, 2)]] WTexture1DArray __bindless_WTexture1DArray__[];
[[vk_binding(0, 2)]] WTexture2D __bindless_WTexture2D__[];
[[vk_binding(0, 2)]] WTexture2DArray __bindless_WTexture2DArray__[];
[[vk_binding(0, 2)]] WTexture3D __bindless_WTexture3D__[];
#endif
