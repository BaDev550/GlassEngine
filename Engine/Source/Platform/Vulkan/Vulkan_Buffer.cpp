#include "gepch.h"
#include "Vulkan_Buffer.h"

namespace ge::renderer {
	static const VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
		if (minOffsetAlignment > 0)
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		return instanceSize;
	}

	Vulkan_Buffer::Vulkan_Buffer(uint64_t size, BufferUsageFlags usage, MemoryPropertiesFlags memoryProperties) 
		: _instanceSize(size),
		_instanceCount(1),
		_usage(utils::EngineBufferUsageFlags(usage)),
		_memoryUsage(utils::EngineMemoryUsageToVMA(memoryProperties))
	{
		// CastChecked gets C2681 build error
		// _context = CastChecked<Vulkan_RenderContext>(&Application::Get()->GetWindow().GetRenderContext());
		_context = static_cast<Vulkan_RenderContext *>(&Application::Get()->GetWindow().GetRenderContext());
		
		_alignmentSize = GetAlignment(size, 1);
		_bufferSize = _alignmentSize * _instanceCount;
		_context->CreateBuffer(_bufferSize, _usage, _memoryUsage, _buffer, _allocation);

		_descriptorInfo.buffer = _buffer;
		_descriptorInfo.offset = 0;
		_descriptorInfo.range = _bufferSize;
	}

	Vulkan_Buffer::~Vulkan_Buffer() {
		Unmap();
	}

	void Vulkan_Buffer::Map(uint64_t size, uint64_t offset) {
		GE_ASSERT(_buffer, "Called memory before buffer was created");
		vmaMapMemory(_context->GetAllocator().GetAllocator(), _allocation, &_data);
	}

	void Vulkan_Buffer::Unmap() {
		GE_ASSERT(_data, "Called unmap to not mapped data");
		vmaUnmapMemory(_context->GetAllocator().GetAllocator(), _allocation);
	}

	void Vulkan_Buffer::Write(void* data, uint64_t size, uint64_t offset) {
		GE_ASSERT(_data, "Cannot write unmapped buffer");
		GE_ASSERT(size < _bufferSize, "Outof range");
		if (size == VK_WHOLE_SIZE) {
			memcpy(_data, data, _bufferSize);
		}
		else {
			char* memOffset = (char*)_data;
			memOffset += offset;
			memcpy(memOffset, data, size);
		}
	}

	void* Vulkan_Buffer::GetData() { return _data; }
}