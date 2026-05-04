#pragma once
#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Utilities/Logger.h"
#include "Types.h"

struct GLFWwindow;

namespace ge::renderer {
	class Image;
	class Sampler;
	class Buffer;

	class RenderContext {
	public:
		virtual ~RenderContext() = default;
		virtual void Init() = 0;
		virtual void Wait() = 0;

		[[nodiscard]] uint32_t GetReadonlyImageHandle(Image& image, ImageSubresource subresource);
		[[nodiscard]] uint32_t GetWritableImageHandle(Image& image, ImageSubresource subresource);
		[[nodiscard]] uint32_t GetSamplerHandle(Sampler& sampler);

		virtual void RemoveReadonlyImageHandle(uint32_t i) = 0;
		virtual void RemoveWritableImageHandle(uint32_t i) = 0;
		virtual void RemoveSamplerHandle(uint32_t i) = 0;

		void SetUniformBuffer(ge::mem::Ref<Buffer>& buffer, uint32_t binding) {
			if (binding > 10) {
				GE_GRAPHICS_ERROR("max binding is 10");
				return;
			}
			ISetUniformBuffer(buffer, binding);
		}
		//TODO (0x): better name
		[[nodiscard]] virtual bool UmaOrRebar() = 0;

		static ge::mem::Scope<RenderContext> Create(GLFWwindow* window);
	protected:
		[[nodiscard]] virtual uint32_t IGetReadonlyImageHandle(Image& image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t IGetWritableImageHandle(Image& image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t IGetSamplerHandle(Sampler& sampler) = 0;
		virtual void ISetUniformBuffer(ge::mem::Ref<Buffer>& buffer, uint32_t binding) = 0;
	};
}