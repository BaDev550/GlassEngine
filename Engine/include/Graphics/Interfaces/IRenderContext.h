#pragma once
#include <Core.h>
#include <Memory/Memory.h>

#include "IBuffer.h"

struct GLFWwindow;
namespace ge::graphics {
	class IImage;
	class ISampler;
	class IBuffer;

	class GE_API IRenderContext : public mem::RefCounted {
	public:
		virtual ~IRenderContext() = default;
		virtual void Wait() = 0;

		[[nodiscard]] uint32_t GetReadonlyImageHandle(Image& image, ImageSubresource subresource);
		[[nodiscard]] uint32_t GetWritableImageHandle(Image& image, ImageSubresource subresource);
		[[nodiscard]] uint32_t GetSamplerHandle(Sampler& sampler);

		void SetUniformBuffer(mem::Ref<IBuffer>& buffer, uint32_t binding) {
			if (binding > 10) {
				GE_GRAPHICS_ERROR("max binding is 10");
				return;
			}
			RHI_SetUniformBuffer(buffer, binding);
		}
	protected:
		[[nodiscard]] virtual uint32_t RHI_GetReadonlyImageHandle(Image& image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t RHI_GetWritableImageHandle(Image& image, ImageSubresource subresource) = 0;
		[[nodiscard]] virtual uint32_t RHI_GetSamplerHandle(Sampler& sampler) = 0;
		virtual void RHI_RemoveReadonlyImageHandle(uint32_t i) = 0;
		virtual void RHI_RemoveWritableImageHandle(uint32_t i) = 0;
		virtual void RHI_RemoveSamplerHandle(uint32_t i) = 0;
		virtual void RHI_SetUniformBuffer(mem::Ref<IBuffer>& buffer, uint32_t binding) = 0;
	};
}