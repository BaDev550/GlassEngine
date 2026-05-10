#pragma once

namespace ge::renderer {
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
	using BufferUsageFlags = ge::Flags<BufferUsageFlagsBits>;

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
		IBuffer(BufferSpec spec) : _specs(specs) {}
		virtual ~IBuffer() = default;

		void Write(void* data, size_t size, size_t offset) {

		}
	private:
		void* _mappedPtr = nullptr;
		BufferSpec _specs;
	};
}
template <>
struct ge::FlagTraits<ge::renderer::BufferUsageFlagsBits> {
	static constexpr bool is_bitmask = true;
};