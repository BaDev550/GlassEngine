#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include "Types.h"

namespace ge::renderer {
	struct SamplerSpec {
		float anisotropy;
		float maxLod;
		float minLod;
		float lodBias;
		SamplerAddressMode addressMode;
		FilterType filterType;
		FilterType mipmapfilterType;
		CompareOp compareOp;
	};

	class Sampler : public RenderObject {
	public:
		[[nodiscard]] static ge::mem::Ref<Sampler> Create(const SamplerSpec& desc);
		explicit Sampler(const SamplerSpec& desc) : _desc(desc) {} 

		[[nodiscard]] uint32_t GetHandle();
	protected:
		SamplerSpec _desc;
	private:
		uint32_t _handle{ static_cast<uint32_t>(-1) };
	};
}