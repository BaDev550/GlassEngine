#include "Model.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

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

	uint32_t LODManager::GetLODindex(const GEVector<LODMesh>& lods, const ge::mem::Ref<Camera>& cam, const glm::vec3& objectPosition) const
	{
		if (lods.empty()) return 0;
		if (lods.size() == 1) return 0;

		glm::vec3 camPos = cam->GetPosition();
		float distSquared = glm::distance(camPos, objectPosition);

		for (size_t i = 0; i < _lodLevels.size(); ++i) {
			float thresholdSquared = _lodLevels[i] * _lodLevels[i];

			if (distSquared < thresholdSquared) {
				return std::min(static_cast<uint32_t>(i), static_cast<uint32_t>(lods.size() - 1));
			}
		}
		return static_cast<uint32_t>(lods.size() - 1);
	}
}