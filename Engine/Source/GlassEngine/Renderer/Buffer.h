#pragma once
#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "Vulkan_Buffer.h"

namespace ge::renderer {
	enum class BufferUsageFlagsBit {
		None		= 0,
		Uniform		= BIT(0),
		Storage		= BIT(1),
		Transfer_dst = BIT(2),
		Transfer_src = BIT(3)
	};
	using BufferUsageFlags = ge::Flags<BufferUsageFlagsBit>;
	template<> struct ge::FlagTraits<BufferUsageFlagsBit> { static constexpr bool isBitmask = true; };

	enum class MemoryPropertiesFlagsBit {
		None			= 0,
		Host_visible	= BIT(0),
		Host_cohreant	= BIT(1),
		Device			= BIT(2)
	};
	using MemoryPropertiesFlags = ge::Flags<MemoryPropertiesFlagsBit>;
	template<> struct ge::FlagTraits<MemoryPropertiesFlagsBit> { static constexpr bool isBitmask = true; };

	class Buffer : public mem::RefCounted {
	public:
		Buffer() {}
		virtual ~Buffer() = default;

		virtual void Map(uint64_t size = UINT64_MAX, uint64_t offset = 0) {};
		virtual void Unmap() {}
		virtual void Write(void* data, uint64_t size = UINT64_MAX, uint64_t offset = 0) = 0;
		virtual void* GetData() = 0;

		static ge::mem::Ref<Buffer> Create(uint64_t size, BufferUsageFlags usage, MemoryPropertiesFlags memoryProperties);
	};
}