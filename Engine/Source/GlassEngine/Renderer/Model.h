#pragma once
#include "GlassEngine/Core/Core.h"
#include "GlassEngine/Asset/Asset.h"
#include "GlassEngine/Renderer/Buffer.h"
#include "GlassEngine/Renderer/Material.h"
#include <glm/glm.hpp>

namespace ge::renderer {
	class Camera;
	struct Vertex {
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 texCoords;
		glm::vec4 tangents;
	};

	struct Submesh {
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t materialIndex;
	};

	struct SourceMeshSpec {
		bool flipUVs = false;
		bool loadMaterials = true;
		uint32_t maxLODlevel = 4;
	};

	struct LODMesh {
		GEVector<renderer::Vertex> vertices;
		GEVector<uint32_t> indices;
		GEVector<Submesh> submesh;
	};

	class LODManager {
	public:
		uint32_t GetLODindex(const GEVector<LODMesh>& lods, const ge::mem::Ref<Camera>& cam, const glm::vec3& objectPosition) const;
	private:
		GEVector<float> _lodFractions = { 0.05f, 0.1f, 0.25f, 0.5f };
	};

	class SourceMesh : public Asset {
	public:
		virtual ~SourceMesh() = default;
		GEVector<LODMesh>& GetLODs() { return _lods; }
		LODManager& GetLODManager() { return _lodManager; }

		ge::mem::Ref<Buffer>& GetVertexBuffer() { return _vertexBuffer; }
		ge::mem::Ref<Buffer>& GetIndexBuffer() { return _indexBuffer; }

		void CreateGPUBuffers();
		static AssetType GetStaticAssetType() { return AssetType::SourceMesh; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	protected:
		GEVector<LODMesh> _lods;
		LODManager _lodManager;
		ge::mem::Ref<Buffer> _vertexBuffer = nullptr;
		ge::mem::Ref<Buffer> _indexBuffer = nullptr;
	};

	class StaticMesh : public SourceMesh {
	public:
		StaticMesh() = default;

		ge::mem::Ref<MaterialTable>& GetMaterialTable() { return _materialTable; }
		void SetMaterialTable(const ge::mem::Ref<MaterialTable>& materialTable) { _materialTable = materialTable; }
		static AssetType GetStaticAssetType() { return AssetType::StaticMesh; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
	private:
		ge::mem::Ref<MaterialTable> _materialTable;
	};
}