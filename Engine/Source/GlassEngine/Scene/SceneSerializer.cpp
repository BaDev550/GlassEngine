#include "gepch.h"
#include "SceneSerializer.h"
#include <set>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/binary.h>

namespace YAML {
	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::u8vec4> {
		static Node encode(const glm::u8vec4& rhs) {
			Node node;
			node.push_back(static_cast<int>(rhs.x));
			node.push_back(static_cast<int>(rhs.y));
			node.push_back(static_cast<int>(rhs.z));
			node.push_back(static_cast<int>(rhs.w));
			return node;
		}

		static bool decode(const Node& node, glm::u8vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = static_cast<uint8_t>(node[0].as<int>());
			rhs.y = static_cast<uint8_t>(node[1].as<int>());
			rhs.z = static_cast<uint8_t>(node[2].as<int>());
			rhs.w = static_cast<uint8_t>(node[3].as<int>());
			return true;
		}
	};

	template<>
	struct convert<ge::UUID> {
		static Node encode(const ge::UUID& uuid) {
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}
		static bool decode(const Node& node, ge::UUID& uuid) {
			uuid = node[0].as<uint64_t>();
			return true;
		}
	};

	template<>
	struct convert<GEString> {
		static Node encode(const GEString& str) {
			Node node;
			node.push_back(str);
			return node;
		}
		static bool decode(const Node& node, GEString& str) {
			str = node[0].as<std::string>();
			return true;
		}
	};
}
namespace ge {
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& vec) {
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec) {
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec) {
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::u8vec4& vec) {
		out << YAML::Flow;
		out << YAML::BeginSeq
			<< static_cast<int>(vec.x)
			<< static_cast<int>(vec.y)
			<< static_cast<int>(vec.z)
			<< static_cast<int>(vec.w)
			<< YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const GEString& string) {
		out << (std::string)string;
		return out;
	}

#define SERIALIZE_COMPONENT_START(component) if (entity.HasComponent<component>()) { \
	out << YAML::Key << #component; \
	out << YAML::BeginMap; \
	component& comp_##component = entity.GetComponent<component>();
#define SERIALIZE_COMPONENT_END out << YAML::EndMap; }

#define DESERIALIZE_COMPONENT_START(component) auto node_##component = node[#component]; if(node_##component) { auto& comp_##component = entity.AddOrReplaceComponent<component>();
#define DESERIALIZE_COMPONENT_END }

	static void SerializeEntity(YAML::Emitter& out, Entity& entity) {
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetEntityID();

		SERIALIZE_COMPONENT_START(IdentityComponent)
			out << YAML::Key << "Name" << YAML::Value << comp_IdentityComponent.name;
		SERIALIZE_COMPONENT_END

		SERIALIZE_COMPONENT_START(TransformComponent)
			out << YAML::Key << "Position" << YAML::Value << comp_TransformComponent.position;
			out << YAML::Key << "Rotation" << YAML::Value << comp_TransformComponent.rotation;
			out << YAML::Key << "Scale" << YAML::Value << comp_TransformComponent.scale;
		SERIALIZE_COMPONENT_END

		SERIALIZE_COMPONENT_START(StaticMeshComponent)
			out << YAML::Key << "MeshHandle" << YAML::Value << comp_StaticMeshComponent.meshHandle;
			out << YAML::Key << "IsVisible" << YAML::Value << comp_StaticMeshComponent.isVisible;
			out << YAML::Key << "LODLevel" << YAML::Value << comp_StaticMeshComponent.lodLevel;
			SERIALIZE_COMPONENT_END

		SERIALIZE_COMPONENT_START(PointLightComponent)
			out << YAML::Key << "Color" << YAML::Value << comp_PointLightComponent.handle.color;
			out << YAML::Key << "Intensity" << YAML::Value << comp_PointLightComponent.handle.intensity;
			out << YAML::Key << "Radius" << YAML::Value << comp_PointLightComponent.handle.radius;
		SERIALIZE_COMPONENT_END

		SERIALIZE_COMPONENT_START(DirectionalLightComponent)
			out << YAML::Key << "Color" << YAML::Value << comp_DirectionalLightComponent.handle.color;
			out << YAML::Key << "Intensity" << YAML::Value << comp_DirectionalLightComponent.handle.intensity;
		SERIALIZE_COMPONENT_END

		out << YAML::EndMap;
	}
	
	static void DeserializeEntity(YAML::Node node, Entity& entity) {
		DESERIALIZE_COMPONENT_START(TransformComponent)
			comp_TransformComponent.position = node_TransformComponent["Position"].as<glm::vec3>();
			comp_TransformComponent.rotation = node_TransformComponent["Rotation"].as<glm::vec3>();
			comp_TransformComponent.scale = node_TransformComponent["Scale"].as<glm::vec3>();
		DESERIALIZE_COMPONENT_END

		DESERIALIZE_COMPONENT_START(StaticMeshComponent)
			comp_StaticMeshComponent.meshHandle = node_StaticMeshComponent["MeshHandle"].as<uint64_t>();
			comp_StaticMeshComponent.isVisible = node_StaticMeshComponent["IsVisible"].as<bool>();
			comp_StaticMeshComponent.lodLevel = node_StaticMeshComponent["LODLevel"].as<uint32_t>();
		DESERIALIZE_COMPONENT_END

		DESERIALIZE_COMPONENT_START(PointLightComponent)
			comp_PointLightComponent.handle.color = node_PointLightComponent["Color"].as<glm::u8vec4>();
			comp_PointLightComponent.handle.intensity = node_PointLightComponent["Intensity"].as<float>();
			comp_PointLightComponent.handle.radius = node_PointLightComponent["Radius"].as<float>();
		DESERIALIZE_COMPONENT_END

		DESERIALIZE_COMPONENT_START(DirectionalLightComponent)
			comp_DirectionalLightComponent.handle.color = node_DirectionalLightComponent["Color"].as<glm::u8vec4>();
			comp_DirectionalLightComponent.handle.intensity = node_DirectionalLightComponent["Intensity"].as<float>();
		DESERIALIZE_COMPONENT_END
	}

	SceneSerializer::SceneSerializer(const mem::Ref<Scene>& scene) : _scene(scene) { }

	bool SceneSerializer::Serialize(const GEString& path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << _scene->GetName();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (auto& [id, entity] : _scene->GetEntities()) {
			if (!entity)
				continue;

			SerializeEntity(out, *entity);
		}
		YAML::EndSeq;
		YAML::EndMap;

		std::ofstream file(path);
		file << out.c_str();
		return true;
	}

	bool SceneSerializer::Deserialize(const GEString& path)
	{
		_scene->Clear();
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		GEString sceneName = data["Scene"].as<std::string>();
		auto entities = data["Entities"];
		if (entities) {
			for (auto entity : entities) {
				UUID uuid = entity["Entity"].as<uint64_t>();
				GEString name;
				auto identityComponent = entity["IdentityComponent"];
				if (identityComponent) {
					name = identityComponent["Name"].as<std::string>();
				}
				Entity* deserializedEntity = _scene->CreateEntityWithID(uuid, name);

				if (deserializedEntity)
					DeserializeEntity(entity, *deserializedEntity);
			}
		}
		return true;
	}
}