#pragma once

#include <Core.h>
#include <Flags.h>
#include <Memory/Memory.h>

namespace ge::graphics {
	enum class BufferUsageFlagsBits : uint16_t {
		None = 0,
		Uniform = BIT(0),
		Readonly = BIT(1),
		Writable = BIT(2),
		TransferDst = BIT(3),
		TransferSrc = BIT(4),
		Vertex = BIT(5),
		Index = BIT(6),
		Indirect = BIT(7),
	};
	using BufferUsageFlags = Flags<BufferUsageFlagsBits>;

	enum class BufferCpuAccess : uint8_t {
		None = 0,
		Write,
		ReadWrite
	};

	enum class BufferMemoryType : uint8_t {
		Auto = 0,
		DeviceMemory,
		SystemMemory
	};

	struct BufferSpec {
		uint32_t elementSize;
		uint32_t elementCount;
		BufferUsageFlags usageFlags;
		BufferCpuAccess cpuAccess;
		BufferMemoryType memoryType;
	};

	class GE_API IBuffer : public mem::RefCounted {
	public:
		virtual ~IBuffer() = default;

		void Write(void* data, size_t size, size_t offset = SIZE_MAX) {
			if (data && _mappedPtr && size <= (_specs.elementSize * _specs.elementCount)) {
				std::memcpy(_mappedPtr, data, size);
			}
		}
	protected:
		void* _mappedPtr = nullptr;
		BufferSpec _specs;
	};
}
template <>
struct ge::FlagTraits<ge::graphics::BufferUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};