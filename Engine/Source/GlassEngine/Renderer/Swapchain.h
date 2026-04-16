#pragma once

#include "GlassEngine/Renderer/RenderContext.h"
#include "GlassEngine/Renderer/Types.h"
#include <glm/glm.hpp>

namespace ge::renderer {
	struct SwapchainSpec {
		glm::uvec2 extent;
		ImageFormat depthFormat;
		bool vsync;
		bool srgb;
	};

	class Swapchain {
	public:
		[[nodiscard]] static ge::mem::Scope<Swapchain> Create(const SwapchainSpec& desc, RenderContext& renderContext);

		Swapchain(const SwapchainSpec& spec, RenderContext& renderContext) : _spec(spec), _renderContext(renderContext) {}
		virtual ~Swapchain() = default;

		void ReCreateSwapchain(const SwapchainSpec& spec) {
			// TODO (dnm): check spec is valid
			CreateSwapchain(spec);
		}
	protected:
		virtual void CreateSwapchain(const SwapchainSpec& newSpec) = 0;
		SwapchainSpec _spec;
		uint32_t _imageCount{};
		RenderContext& _renderContext;
	};
}