#pragma once
#include "GlassEngine/Renderer/Buffer.h"
#include "Vulkan_RenderContext.h"

namespace ge::renderer {
	namespace utils {
		static VmaMemoryUsage EngineMemoryUsageToVMA(const MemoryPropertiesFlags memoryFlags) {
			if (memoryFlags.Has(MemoryPropertiesFlagsBit::Device)) return VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
			if (memoryFlags.Has(MemoryPropertiesFlagsBit::Host_visible)) return VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
			if (memoryFlags.Has(MemoryPropertiesFlagsBit::Host_cohreant)) return VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
		}

		static VkBufferUsageFlags EngineBufferUsageFlags(const BufferUsageFlags usageFlags) {
			VkBufferUsageFlags usage;
			if (usageFlags.Has(BufferUsageFlagsBit::Uniform)) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (usageFlags.Has(BufferUsageFlagsBit::Transfer_src)) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (usageFlags.Has(BufferUsageFlagsBit::Transfer_dst)) usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (usageFlags.Has(BufferUsageFlagsBit::Storage)) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			return usage;
		}
	}

	class Vulkan_Buffer : public Buffer {
	public:
		Vulkan_Buffer(uint64_t size, BufferUsageFlags usage, MemoryPropertiesFlags memoryProperties);
		virtual ~Vulkan_Buffer();

		virtual void Map(uint64_t size = UINT64_MAX, uint64_t offset = 0) override;
		virtual void Unmap() override;
		virtual void Write(void* data, uint64_t size = UINT64_MAX, uint64_t offset = 0) override;
		virtual void* GetData() override;
	private:
		void* _data = nullptr;
		VkBuffer _buffer;
		VmaAllocation _allocation;
		VkBufferUsageFlags _usage;
		uint32_t _instanceCount;
		VkDeviceSize _bufferSize;
		VkDeviceSize _instanceSize;
		VkDeviceSize _alignmentSize;
		VmaMemoryUsage _memoryUsage;
		VkDescriptorBufferInfo _descriptorInfo;
		Vulkan_RenderContext* _context;
	};
}