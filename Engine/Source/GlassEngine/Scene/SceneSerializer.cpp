#include "gepch.h"
#include "SceneSerializer.h"
#include <set>

namespace ge {
	static void SerializeEntity(file::Writer& out, Entity& entity) {
		EntityID entityId = entity.GetEntityID();
		out.WriteData(reinterpret_cast<const char*>(&entityId), sizeof(EntityID));

		uint32_t entityNameLenght = entity.GetName().size();
		out.WriteData(reinterpret_cast<const char*>(&entityNameLenght), sizeof(uint32_t));
		out.WriteData(entity.GetName().c_str(), entityNameLenght);

		auto& tc = entity.GetComponent<TransformComponent>();
		out.WriteData(reinterpret_cast<const char*>(&tc.position), sizeof(glm::vec3));
		out.WriteData(reinterpret_cast<const char*>(&tc.rotation), sizeof(glm::vec3));
		out.WriteData(reinterpret_cast<const char*>(&tc.scale), sizeof(glm::vec3));

		bool hasStaticMesh = entity.HasComponent<StaticMeshComponent>();
		out.WriteData(reinterpret_cast<const char*>(&hasStaticMesh), sizeof(bool));

		if (hasStaticMesh) {
			auto& smc = entity.GetComponent<StaticMeshComponent>();
			out.WriteData(reinterpret_cast<const char*>(&smc.meshHandle), sizeof(AssetHandle));
			out.WriteData(reinterpret_cast<const char*>(&smc.lodLevel), sizeof(uint32_t));
			out.WriteData(reinterpret_cast<const char*>(&smc.isVisible), sizeof(bool));
		}
	}
	static std::pair<EntityID, GEString> GetEntityNameAndID(file::Reader& reader) {
		EntityID id;
		uint32_t entityNameLenght = 0;
		reader.ReadData(reinterpret_cast<char*>(&id), sizeof(EntityID));
		reader.ReadData(reinterpret_cast<char*>(&entityNameLenght), sizeof(uint32_t));
		GEString entityName;
		entityName.resize(entityNameLenght);
		reader.ReadData(entityName.data(), entityNameLenght);

		return { id, entityName };
	}
	static void DeserializeEntity(file::Reader& reader, Entity& entity) {
		auto& tc = entity.GetComponent<TransformComponent>();
		reader.ReadData(reinterpret_cast<char*>(&tc.position), sizeof(glm::vec3));
		reader.ReadData(reinterpret_cast<char*>(&tc.rotation), sizeof(glm::vec3));
		reader.ReadData(reinterpret_cast<char*>(&tc.scale), sizeof(glm::vec3));

		bool hasStaticMesh = false;
		reader.ReadData(reinterpret_cast<char*>(&hasStaticMesh), sizeof(bool));

		if (hasStaticMesh) {
			auto& smc = entity.AddComponent<StaticMeshComponent>();
			reader.ReadData(reinterpret_cast<char*>(&smc.meshHandle), sizeof(AssetHandle));
			reader.ReadData(reinterpret_cast<char*>(&smc.lodLevel), sizeof(uint32_t));
			reader.ReadData(reinterpret_cast<char*>(&smc.isVisible), sizeof(bool));
		}
	}

	SceneSerializer::SceneSerializer(const mem::Ref<Scene>& scene) : _scene(scene) { }

	bool SceneSerializer::Serialize(const GEString& path)
	{
		file::Writer out(path.ToPath());
		if (!out.IsStreamGood())
			return false;
		uint32_t sceneNameLength = _scene->GetName().size();
		uint32_t entityCount = _scene->GetEntities().size();
		out.WriteData(reinterpret_cast<const char*>(&sceneNameLength), sizeof(uint32_t));
		out.WriteData(_scene->GetName().c_str(), sceneNameLength);
		out.WriteData(reinterpret_cast<const char*>(&entityCount), sizeof(uint32_t));

		for (auto& [id, entity] : _scene->GetEntities()) {
			if (!entity)
				continue;

			SerializeEntity(out, *entity);
		}
		return true;
	}

	bool SceneSerializer::Deserialize(const GEString& path)
	{
		_scene->Clear();
		file::Reader in(path.ToPath());
		if (!in.IsStreamGood())
			return false;

		uint32_t sceneNameSize = 0;
		uint32_t entityCount = 0;
		in.ReadData(reinterpret_cast<char*>(&sceneNameSize), sizeof(uint32_t));
		GEString sceneName;
		sceneName.resize(sceneNameSize);
		in.ReadData(sceneName.data(), sceneNameSize);
		in.ReadData(reinterpret_cast<char*>(&entityCount), sizeof(uint32_t));

		for (uint32_t i = 0; i < entityCount; i++) {
			std::pair<EntityID, GEString> idName = GetEntityNameAndID(in);
			auto entity = _scene->CreateEntityWithID(idName.first, idName.second);
			DeserializeEntity(in, *entity);
		}
		return true;
	}
}