#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Asset/Asset.h"
#include "GlassEngine/Renderer/Buffer.h"
#include "GlassEngine/Renderer/Material.h"
#include <glm/glm.hpp>

namespace ge::renderer {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 texCoords;
	};

	struct Submesh {
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t materialIndex;
	};

	struct SourceMeshSpec {
		uint32_t maxLODlevel = 4;
	};

	struct LODMesh {
		GEVector<renderer::Vertex> vertices;
		GEVector<uint32_t> indices;
		GEVector<Submesh> submesh;
	};

	class SourceMesh : public Asset {
	public:
		virtual ~SourceMesh() = default;
		GEVector<LODMesh>& GetLODs() { return _lods; }

		ge::mem::Ref<Buffer>& GetVertexBuffer() { return _vertexBuffer; }
		ge::mem::Ref<Buffer>& GetIndexBuffer() { return _indexBuffer; }

		void CreateGPUBuffers();
		static AssetType GetStaticAssetType() { return AssetType::SourceMesh; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	protected:
		GEVector<LODMesh> _lods;
		ge::mem::Ref<Buffer> _vertexBuffer = nullptr;
		ge::mem::Ref<Buffer> _indexBuffer = nullptr;
	};

	class StaticMesh : public SourceMesh {
	public:
		StaticMesh() = default;

		mem::Ref<MaterialTable>& GetMaterialTable() { return _materialTable; }
		void SetMaterialTable(const mem::Ref<MaterialTable>& materialTable) { _materialTable = materialTable; }
		static AssetType GetStaticAssetType() { return AssetType::StaticMesh; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	private:
		mem::Ref<MaterialTable> _materialTable;
	};
}