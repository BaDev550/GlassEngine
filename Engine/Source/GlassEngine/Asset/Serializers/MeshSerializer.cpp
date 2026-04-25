#include "gepch.h"
#include "MeshSerializer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <assimp/mesh.h>
#include <meshoptimizer.h>

#include "GlassEngine/Renderer/Model.h"

namespace ge {

	AssetType MeshSourceSerializer::ImportFromSource(const ImportAssetData& asset, const std::filesystem::path& source, const std::filesystem::path& targetPath)
	{
		renderer::SourceMeshSpec spec = renderer::SourceMeshSpec();
		if (asset.sourceMeshSpecs != nullptr)
			spec = *asset.sourceMeshSpecs;

		GE_CORE_INFO("Importing mesh from source: {}", source.string());
		GE_PROFILE_SCOPE(std::format("MeshSourceSerializer::ImportFromSource - Assimp Import path: {}", source.string()));
		std::filesystem::path fullSourcePath = std::filesystem::absolute(source);
		std::filesystem::path meshDirectory = fullSourcePath.parent_path();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(source.string(), ASSIMP_FLAGS);
		if (!scene) {
			GE_CORE_ERROR(" -Failed to load model!");
			return GE_INVALID_ASSET_TYPE;
		}

		// import vertices + indices data and optimize
		renderer::LODMesh lod0;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;
		if (scene->HasMeshes()) {
			for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
				aiMesh* aimesh = scene->mMeshes[i];
				renderer::Submesh& submesh = lod0.submesh.emplace_back();
				submesh.vertexOffset = vertexOffset;
				submesh.vertexCount = aimesh->mNumVertices;
				submesh.indexOffset = indexOffset;
				submesh.indexCount = aimesh->mNumFaces * 3;
				submesh.materialIndex = aimesh->mMaterialIndex;

				for (uint32_t v = 0; v < aimesh->mNumVertices; v++) {
					renderer::Vertex vertex{};
					vertex.position = { aimesh->mVertices[v].x, aimesh->mVertices[v].y, aimesh->mVertices[v].z }; // TODO(0x): write a math util for conversations
					if (aimesh->HasNormals())
						vertex.normal = { aimesh->mNormals[v].x, aimesh->mNormals[v].y, aimesh->mNormals[v].z };
					if (aimesh->HasTextureCoords(0))
						vertex.texCoords = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y, aimesh->mTextureCoords[0][v].z };
					lod0.vertices.push_back(vertex);
				}
				for (uint32_t f = 0; f < aimesh->mNumFaces; f++) {
					aiFace face = aimesh->mFaces[f];
					for (uint32_t j = 0; j < face.mNumIndices; j++) {
						lod0.indices.push_back(face.mIndices[j]);
					}
				}
				vertexOffset += aimesh->mNumVertices;
				indexOffset += aimesh->mNumFaces * 3;
			}

			// Optimze the vertices and indiceses so it reduces the filesize and draw time
			{
				GE_PROFILE_SCOPE("MeshSourceSerializer::ImportFromSource::Optimizing");
				meshopt_optimizeVertexCache(lod0.indices.data(), lod0.indices.data(), lod0.indices.size(), lod0.vertices.size());
				meshopt_optimizeOverdraw(lod0.indices.data(), lod0.indices.data(), lod0.indices.size(), &lod0.vertices[0].position.x, lod0.vertices.size(), sizeof(renderer::Vertex), 1.05f);
				meshopt_optimizeVertexFetch(lod0.vertices.data(), lod0.indices.data(), lod0.indices.size(), lod0.vertices.data(), lod0.vertices.size(), sizeof(renderer::Vertex));
			}

			// Load all LOD levels
			GEVector<renderer::LODMesh> allLods;
			{
				allLods.push_back(lod0);

				for (uint32_t level = 1; level < spec.maxLODlevel; level++) {
					renderer::LODMesh& prevLod = allLods[level - 1];
					renderer::LODMesh newLod;
					newLod.vertices = prevLod.vertices;
					newLod.submesh = prevLod.submesh;

					bool simplified = false;
					uint32_t newIndexOffset = 0;

					for (size_t s = 0; s < prevLod.submesh.size(); s++) {
						const auto& prevSubmesh = prevLod.submesh[s];
						auto& newSubmesh = newLod.submesh[s];

						size_t target_index_count = (size_t)(prevSubmesh.indexCount * 0.5f); // Reduce by 50%
						GEVector<uint32_t> submeshIndices(prevSubmesh.indexCount);
						float target_error = 0.01f + (level * 0.03f);
						float error = 0.0f;
						size_t new_indices = meshopt_simplify(
							submeshIndices.data(),
							&prevLod.indices[prevSubmesh.indexOffset],
							prevSubmesh.indexCount,
							&newLod.vertices[0].position.x,
							newLod.vertices.size(),
							sizeof(renderer::Vertex),
							target_index_count,
							target_error,
							meshopt_SimplifyLockBorder,
							&error
						);

						if (new_indices > target_index_count * 1.2f) { // fallback
							new_indices = meshopt_simplifySloppy(
								submeshIndices.data(),
								&prevLod.indices[prevSubmesh.indexOffset],
								prevSubmesh.indexCount,
								&newLod.vertices[0].position.x,
								newLod.vertices.size(),
								sizeof(renderer::Vertex),
								target_index_count,
								target_error,
								&error
							);
						}

						if (new_indices < prevSubmesh.indexCount) {
							submeshIndices.resize(new_indices);
							newLod.indices.insert(newLod.indices.end(), submeshIndices.begin(), submeshIndices.end());
							newSubmesh.indexOffset = newIndexOffset;
							newSubmesh.indexCount = static_cast<uint32_t>(new_indices);
							newIndexOffset += newSubmesh.indexCount;
							simplified = true;
						}
						else {
							newLod.indices.insert(
								newLod.indices.end(),
								&prevLod.indices[prevSubmesh.indexOffset],
								&prevLod.indices[prevSubmesh.indexOffset] + prevSubmesh.indexCount);
							newSubmesh.indexOffset = newIndexOffset;
							newSubmesh.indexCount = prevSubmesh.indexCount;
							newIndexOffset += newSubmesh.indexCount;
						}
					}
					if (!simplified) break;

					meshopt_optimizeVertexCache(newLod.indices.data(), newLod.indices.data(), newLod.indices.size(), newLod.vertices.size());
					allLods.push_back(newLod);
				}
			}

			uint32_t materialCount = scene->mNumMaterials;
			GEVector<AssetHandle> materialHandles(materialCount, GE_INVALID_ASSET_HANDLE);
			if (scene->HasMaterials()) {
				for (uint32_t i = 0; i < materialCount; i++) {
					aiMaterial* aiMat = scene->mMaterials[i];
					aiString aiTexturePath;
					aiString aiMatName;
					aiMat->Get(AI_MATKEY_NAME, aiMatName);
					GEString matName = aiMatName.length > 0 ? aiMatName.C_Str() : std::format("Material_{}", i);
					std::filesystem::path matTargetPath = targetPath.parent_path() / (matName + GE_ASSET_EXTENSION);
					mem::Ref<renderer::Material> mat = mem::Ref<renderer::Material>::Create();
					mem::Ref<renderer::MaterialAsset> matAsset = mem::Ref<renderer::MaterialAsset>::Create(mat);

					bool hasAlbedo = aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath) == AI_SUCCESS;
					if (hasAlbedo) {
						ImportAssetData importData;
						renderer::TextureSpec textureSpec{};
						textureSpec.attachment = false;
						textureSpec.filter = renderer::ImageFilter::Linear;
						textureSpec.format = renderer::ImageFormat::RGBA8;
						importData.textureSpecs = &textureSpec;
						auto texturePath = meshDirectory / aiTexturePath.C_Str();
						auto texture = Application::Get()->GetAssetManager()->GetOrImportAsset(texturePath, importData);
						matAsset->SetAlbedoTexture(texture->_assetHandle);
					}

					bool hasNormal = aiMat->GetTexture(aiTextureType_NORMALS, 0, &aiTexturePath) == AI_SUCCESS;
					if (hasNormal) {
						ImportAssetData importData;
						renderer::TextureSpec textureSpec{};
						textureSpec.attachment = false;
						textureSpec.filter = renderer::ImageFilter::Linear;
						textureSpec.format = renderer::ImageFormat::RGBA8;
						importData.textureSpecs = &textureSpec;
						auto texturePath = meshDirectory / aiTexturePath.C_Str();
						auto texture = Application::Get()->GetAssetManager()->GetOrImportAsset(texturePath, importData);
						matAsset->SetNormalTexture(texture->_assetHandle);
					}
					materialHandles[i] = Application::Get()->GetEditorAssetManager().CreateAsset(matTargetPath, matAsset);
				}
			}

			// Write into .gasset
			file::Writer out(targetPath);
			if (!out.IsStreamGood()) {
				GE_CORE_ERROR("Failed to open/write to path: {}", targetPath.string());
				return GE_INVALID_ASSET_TYPE;
			}
			uint32_t lodCount = static_cast<uint32_t>(allLods.size());
			out.WriteData(STATIC_MESH_MAGIC, 4);
			out.WriteData(reinterpret_cast<const char*>(&lodCount), sizeof(uint32_t));
			out.WriteData(reinterpret_cast<const char*>(&materialCount), sizeof(uint32_t));
			out.WriteData(reinterpret_cast<const char*>(materialHandles.data()), materialCount * sizeof(AssetHandle));
			for (const auto& lod : allLods) {
				uint32_t vertexCount = static_cast<uint32_t>(lod.vertices.size());
				uint32_t indexCount = static_cast<uint32_t>(lod.indices.size());
				uint32_t submesCount = static_cast<uint32_t>(lod.submesh.size());
				out.WriteData(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
				out.WriteData(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
				out.WriteData(reinterpret_cast<const char*>(&submesCount), sizeof(uint32_t));
				out.WriteData(reinterpret_cast<const char*>(lod.vertices.data()), vertexCount * sizeof(renderer::Vertex));
				out.WriteData(reinterpret_cast<const char*>(lod.indices.data()), indexCount * sizeof(uint32_t));
				out.WriteData(reinterpret_cast<const char*>(lod.submesh.data()), submesCount * sizeof(renderer::Submesh));
			}
			return AssetType::StaticMesh;
		}
		return GE_INVALID_ASSET_TYPE;
	}

	void MeshAssetSerializer::SerializeToFile(const std::filesystem::path& target, mem::Ref<Asset>& asset, const AssetMetadata& mtd)
	{
	}

	mem::Ref<Asset> MeshAssetSerializer::DeserializeFromFile(const AssetMetadata& mtd)
	{
		file::Reader in(mtd.path);
		if (!in.IsStreamGood()) {
			GE_CORE_ERROR("MeshSerialize could not open file: {}", mtd.path.string());
			return nullptr;
		}

		char magic[4];
		in.ReadData(magic, 4);
		if (strncmp(magic, STATIC_MESH_MAGIC, 4) != 0) {
			GE_CORE_ERROR("Invalid magic for mesh asset!");
			return nullptr;
		}

		mem::Ref<renderer::StaticMesh> mesh = mem::Ref<renderer::StaticMesh>::Create();
		uint32_t lodCount = 0;
		auto& lods = mesh->GetLODs();

		in.ReadData(reinterpret_cast<char*>(&lodCount), sizeof(uint32_t));

		uint32_t materialCount = 0;
		in.ReadData(reinterpret_cast<char*>(&materialCount), sizeof(uint32_t));
		GEVector<AssetHandle> materialHandles(materialCount);
		in.ReadData(reinterpret_cast<char*>(materialHandles.data()), materialCount * sizeof(AssetHandle));

		auto materialTable = mem::Ref<renderer::MaterialTable>::Create();
		for (uint32_t i = 0; i < materialCount; i++) {
			AssetHandle handle = materialHandles[i];
			mem::Ref<renderer::MaterialAsset> materialAsset = Application::Get()->GetAssetManager()->GetAsset(handle).Cast<renderer::MaterialAsset>();
			materialTable->AddMaterial(i, materialAsset);
		}
		mesh->SetMaterialTable(materialTable);

		lods.resize(lodCount);
		for (uint32_t i = 0; i < lodCount; i++) {
			uint32_t vertexCount, indexCount, submeshCount;
			in.ReadData(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));

			auto& currentLod = lods[i];
			currentLod.vertices.resize(vertexCount);
			currentLod.indices.resize(indexCount);
			currentLod.submesh.resize(submeshCount);
			in.ReadData(reinterpret_cast<char*>(currentLod.vertices.data()), vertexCount * sizeof(renderer::Vertex));
			in.ReadData(reinterpret_cast<char*>(currentLod.indices.data()), indexCount * sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(currentLod.submesh.data()), submeshCount * sizeof(renderer::Submesh));
		}
		mesh->CreateGPUBuffers();
		return mesh;
	}

	mem::Ref<Asset> MeshAssetSerializer::DeserializeFromFile(const GEVector<uint8_t>& buffer)
	{
		file::BufferReader in(buffer);
		char magic[4];
		if (!in.ReadData(magic, 4) || strncmp(magic, STATIC_MESH_MAGIC, 4) != 0) {
			GE_CORE_ERROR("Invalid magic for static mesh asset");
			return nullptr;
		}
		mem::Ref<renderer::StaticMesh> mesh = mem::Ref<renderer::StaticMesh>::Create();
		uint32_t lodCount = 0;
		if (!in.Read(lodCount)) {
			GE_CORE_ERROR("Failed to read LOD count from buffer");
			return nullptr;
		}
		auto& lods = mesh->GetLODs();
		lods.resize(lodCount);

		for (uint32_t i = 0; i < lodCount; i++) {
			uint32_t vertexCount, indexCount, submeshCount;
			in.ReadData(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));

			auto& currentLod = lods[i];
			currentLod.vertices.resize(vertexCount);
			currentLod.indices.resize(indexCount);
			currentLod.submesh.resize(submeshCount);
			in.ReadData(reinterpret_cast<char*>(currentLod.vertices.data()), vertexCount * sizeof(renderer::Vertex));
			in.ReadData(reinterpret_cast<char*>(currentLod.indices.data()), indexCount * sizeof(uint32_t));
			in.ReadData(reinterpret_cast<char*>(currentLod.submesh.data()), submeshCount * sizeof(renderer::Submesh));
		}
		mesh->CreateGPUBuffers();
		return mesh;
	}
}