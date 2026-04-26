#pragma once

#include "GlassEngine/Renderer/Pipeline.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	class Vulkan_Pipeline : public Pipeline {
	public:
		Vulkan_Pipeline(const PipelineSpec& specs);
		~Vulkan_Pipeline();

		void Invalidate();
		[[nodiscard]] VkPipeline GetPipeline() const noexcept { return _pipeline; }

		virtual void SetDebugName(GEString name) const noexcept override;
	private:
		VkPipeline _pipeline;
	};
}