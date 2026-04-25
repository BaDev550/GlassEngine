#include "Model.h"

namespace ge::renderer {
	void SourceMesh::CreateGPUBuffers()
	{
		if (_lods.empty()) {
			GE_CORE_ERROR("Cannot create GPU buffers for SourceMesh that is empty");
			return;
		}

		if (!_vertexBuffer) {
			BufferSpec vertexBufferSpec{};
			vertexBufferSpec.cpuAccess = BufferCpuAccess::Write;
			vertexBufferSpec.elementCount = static_cast<uint32_t>(_lods[0].vertices.size());
			vertexBufferSpec.elementSize = sizeof(Vertex);
			vertexBufferSpec.memoryType = BufferMemoryType::DeviceMemory;
			vertexBufferSpec.usageFlags = BufferUsageFlagsBits::Vertex;
			_vertexBuffer = Buffer::Create(vertexBufferSpec);
			std::memcpy(_vertexBuffer->GetMappedPtr(), _lods[0].vertices.data(), _lods[0].vertices.size() * sizeof(Vertex));
		}

		if (!_indexBuffer) {
			BufferSpec indexBufferSpec{};
			indexBufferSpec.cpuAccess = BufferCpuAccess::Write;
			indexBufferSpec.elementCount = static_cast<uint32_t>(_lods[0].indices.size());
			indexBufferSpec.elementSize = sizeof(uint32_t);
			indexBufferSpec.memoryType = BufferMemoryType::DeviceMemory;
			indexBufferSpec.usageFlags = BufferUsageFlagsBits::Index;
			_indexBuffer = Buffer::Create(indexBufferSpec);
			std::memcpy(_indexBuffer->GetMappedPtr(), _lods[0].indices.data(), _lods[0].indices.size() * sizeof(uint32_t));
		}
	}
}