#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Utilities/Flags.h"
#include "RenderObject.h"
#include "Types.h"

namespace ge::renderer {
	namespace utility {
		[[nodiscard]] constexpr uint32_t GetAlignment(uint32_t instanceSize, uint32_t minOffsetAlignment) {
			if (minOffsetAlignment > 0)
				return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
			return instanceSize;
		}
	}

	struct BufferCreateDesc {
		uint32_t elementSize;
		uint32_t elementCount;
		BufferUsageFlags usageFlags;
		BufferCpuAccess cpuAccess;
		BufferMemoryType memoryType;
	};

	class Buffer : public RenderObject {
	public:
		[[nodiscard]] static ge::mem::Ref<Buffer> Create(const BufferCreateDesc &desc);

		explicit Buffer(const BufferCreateDesc &desc) noexcept : _desc(desc) {
			GE_ASSERT(_desc.elementSize != 0, "BufferCreateDesc::elementSize must be greater than 0");
			GE_ASSERT(_desc.elementCount != 0, "BufferCreateDesc::elementCount must be greater than 0");

			if (_desc.usageFlags.Has(BufferUsageFlagsBits::Uniform)) _desc.elementSize = utility::GetAlignment(_desc.elementSize, 256);
		}
		virtual ~Buffer() = default;

		template <typename T = uint8_t>
		[[nodiscard]] T* GetMappedPtr() const noexcept { return reinterpret_cast<T*>(_mappedPtr); }
		[[nodiscard]] uint32_t GetAlighnedElementSize() const noexcept { return _alighened_element_size; }
		[[nodiscard]] const auto& GetDescRef() const noexcept { return _desc; }
		[[nodiscard]] auto GetDesc() const noexcept { return _desc; }
	protected:
		BufferCreateDesc _desc;
		uint32_t _alighened_element_size;
		void *_mappedPtr;
	};
}