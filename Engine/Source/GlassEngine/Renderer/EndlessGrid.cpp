#include "gepch.h"
#include "EndlessGrid.h"
#include "RenderAPI.h"
#include "Renderer.h"
#include "ShaderLibrary.h"

namespace ge::renderer {
	EndlessGrid::EndlessGrid(mem::Ref<Framebuffer>& framebuffer) {
		std::vector<glm::vec3> vertices = {
			glm::vec3(1.0f,  1.0f,  0.0f),
			glm::vec3(-1.0f, -1.0f,  0.0f),
			glm::vec3(-1.0f,  1.0f,  0.0f),
			glm::vec3(-1.0f, -1.0f,  0.0f),
			glm::vec3(1.0f,  1.0f,  0.0f),
			glm::vec3(1.0f, -1.0f,  0.0f)
		};

		PipelineSpec spec{};
		spec.shader = Renderer3D::GetShaderLibrary().GetShader("grid");
		spec.targetFramebuffer = framebuffer;
		spec.blendSpec.blendEnabled = true;
		spec.depthStencilSpec.depthTestEnable = false;
		_pipeline = Pipeline::Create(spec);

		BufferSpec bufferSpec{};
		bufferSpec.cpuAccess = BufferCpuAccess::Write;
		bufferSpec.elementCount = vertices.size();
		bufferSpec.elementSize = vertices.size() * sizeof(glm::vec3);
		bufferSpec.memoryType = BufferMemoryType::DeviceMemory;
		bufferSpec.usageFlags = BufferUsageFlagsBits::Vertex;
		_gridBuffer = Buffer::Create(bufferSpec);
		std::memcpy(_gridBuffer->GetMappedPtr(), vertices.data(), vertices.size());
	}

	void EndlessGrid::Draw() {
		Renderer3D::DrawVertex(_pipeline, _vertexCount, 1, _gridBuffer);
	}
}