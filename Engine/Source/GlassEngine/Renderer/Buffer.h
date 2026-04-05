#pragma once
#include "GlassEngine/Memory/Ref.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"

namespace ge::renderer {
	enum class BufferUsageFlagsBit : uint8_t {
		None		= 0,
		Uniform		= BIT(0),
		Storage		= BIT(1),
		Transfer_dst = BIT(2),
		Transfer_src = BIT(3)
	};
	using BufferUsageFlags = ge::Flags<BufferUsageFlagsBit>;

	enum class MemoryPropertiesFlagsBit : uint8_t {
		None			= 0,
		Host_visible	= BIT(0),
		Host_cohreant	= BIT(1),
		Device			= BIT(2)
	};
	using MemoryPropertiesFlags = ge::Flags<MemoryPropertiesFlagsBit>;

	class Buffer : public mem::RefCounted {
	public:
		Buffer() {}
		virtual ~Buffer() = default;

		virtual void Map(uint64_t size = UINT64_MAX, uint64_t offset = 0) = 0;
		virtual void Unmap() = 0;
		virtual void Write(void* data, uint64_t size = UINT64_MAX, uint64_t offset = 0) = 0;
		virtual void* GetData() = 0;

		static ge::mem::Ref<Buffer> Create(uint64_t size, BufferUsageFlags usage, MemoryPropertiesFlags memoryProperties);
	};
}

template<> struct ge::FlagTraits<ge::renderer::BufferUsageFlagsBit> { static constexpr bool isBitmask = true; };
template<> struct ge::FlagTraits<ge::renderer::MemoryPropertiesFlagsBit> { static constexpr bool isBitmask = true; };
