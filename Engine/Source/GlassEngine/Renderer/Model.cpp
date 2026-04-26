#include "Model.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

namespace ge::renderer {
	void SourceMesh::CreateGPUBuffers()
	{
		if (_lods.empty()) {
			GE_CORE_ERROR("Cannot create GPU buffers for SourceMesh that is empty");
			return;
		}

		uint32_t totalVertices = 0;
		uint32_t totalIndices = 0;
		for (const auto& lod : _lods) {
			totalVertices += static_cast<uint32_t>(lod.vertices.size());
			totalIndices += static_cast<uint32_t>(lod.indices.size());
		}

		GEVector<Vertex> globalVertices;
		globalVertices.reserve(totalVertices);
		GEVector<uint32_t> globalIndices;
		globalIndices.reserve(totalIndices);

		for (auto& lod : _lods) {
			uint32_t currentVertexOffset = static_cast<uint32_t>(globalVertices.size());
			uint32_t currentIndexOffset = static_cast<uint32_t>(globalIndices.size());
			globalVertices.insert(globalVertices.end(), lod.vertices.begin(), lod.vertices.end());
			globalIndices.insert(globalIndices.end(), lod.indices.begin(), lod.indices.end());
			for (auto& submesh : lod.submesh) {
				submesh.vertexOffset += currentVertexOffset;
				submesh.indexOffset += currentIndexOffset;
			}
		}

		if (!_vertexBuffer) {
			BufferSpec vertexBufferSpec{};
			vertexBufferSpec.cpuAccess = BufferCpuAccess::Write;
			vertexBufferSpec.elementCount = totalVertices;
			vertexBufferSpec.elementSize = sizeof(Vertex);
			vertexBufferSpec.memoryType = BufferMemoryType::DeviceMemory;
			vertexBufferSpec.usageFlags = BufferUsageFlagsBits::Vertex;
			_vertexBuffer = Buffer::Create(vertexBufferSpec);
			std::memcpy(_vertexBuffer->GetMappedPtr(), globalVertices.data(), globalVertices.size() * sizeof(Vertex));
		}

		if (!_indexBuffer) {
			BufferSpec indexBufferSpec{};
			indexBufferSpec.cpuAccess = BufferCpuAccess::Write;
			indexBufferSpec.elementCount = totalIndices;
			indexBufferSpec.elementSize = sizeof(uint32_t);
			indexBufferSpec.memoryType = BufferMemoryType::DeviceMemory;
			indexBufferSpec.usageFlags = BufferUsageFlagsBits::Index;
			_indexBuffer = Buffer::Create(indexBufferSpec);
			std::memcpy(_indexBuffer->GetMappedPtr(), globalIndices.data(), globalIndices.size() * sizeof(uint32_t));
		}
	}

	uint32_t LODManager::GetLODindex(const GEVector<LODMesh>& lods, const ge::mem::Ref<Camera>& cam, const glm::vec3& objectPosition) const
	{
		if (lods.size() <= 1) return 0;

		float distSquared = glm::length2(cam->GetPosition() - objectPosition);
		float farPlane = cam->GetFarPlane();

		for (size_t i = 0; i < _lodFractions.size(); ++i) {
			float threshold = _lodFractions[i] * farPlane;
			float thresholdSquared = threshold * threshold;
			if (distSquared < thresholdSquared) {
				return std::min(static_cast<uint32_t>(i), static_cast<uint32_t>(lods.size() - 1));
			}
		}

		return static_cast<uint32_t>(lods.size() - 1);
	}
}