#pragma once

#include "GlassEngine/Memory/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include "Types.h"

namespace ge::renderer {
	struct SamplerSpec {
		float anisotropy = 1.f;
		float maxLod = 1000.f;
		float minLod = 0.f;
		float lodBias = 0.f;
		SamplerAddressMode addressMode = SamplerAddressMode::Repeat;
		FilterType filterType = FilterType::Linear;
		FilterType mipmapfilterType = FilterType::Linear;
		CompareOp compareOp = CompareOp::None;
	};

	class Sampler : public RenderObject {
	public:
		[[nodiscard]] static ge::mem::Ref<Sampler> Create(const SamplerSpec& desc);
		explicit Sampler(const SamplerSpec& desc) : _desc(desc) {} 

		[[nodiscard]] const auto& GetSpecRef() const noexcept { return _desc; }
		[[nodiscard]] auto GetSpec() const noexcept { return _desc; }
		[[nodiscard]] uint32_t GetHandle();
	protected:
		SamplerSpec _desc;
	private:
		uint32_t _handle{ static_cast<uint32_t>(-1) };
	};
}