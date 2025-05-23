#pragma once

#include "coreV/gen_model.hpp"
#include "coreV/gen_texture.hpp"

#include<glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>
#include <optional>



namespace gen {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f,1.f,1.f };
		glm::vec3 rotation;

		// Matrix corresponds to translate * Ry * Rx * Rz * scale 
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	enum class ObjectType {
		Generic,
		Camera,
		Light,
		Player,
		NPC,
		Sphere,
		Node
	};


	struct PointLightComponent {

		float lightIntesity = 1.0f;

	};

	struct SoundSphereComponent {
		float radius = 1.0f;         
		bool visible = false;        
		bool isAudibleProbe = false; 
	};

	struct SoundDiscComponent {
		float radius = 1.0f;
		bool visible = false;
		bool isPlayerControlled = false;
	};


	struct NodeComponent {
		enum class NodeColor {
			White,
			Red,
			Orange,
			Green
		};

		NodeColor color = NodeColor::White;
		int parentId = -1;
		bool activated = false;
		glm::vec3 selfPosition{};
		NodeColor lastColorApplied = NodeColor::White;
		bool hasPropagated = false;
	};
	


	class GenGameObject {

	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, GenGameObject>;


		static GenGameObject createGameObject() {
			static id_t currentId = 0;
			return GenGameObject{ currentId++ };
		}

		static GenGameObject makePointLight(float intesity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		GenGameObject(const GenGameObject&) = delete;
		GenGameObject& operator=(const GenGameObject&) = delete;
		GenGameObject(GenGameObject &&) = default;
		GenGameObject& operator=(GenGameObject&&) = default;





		const id_t getId() { return id; }

		glm::vec3 color{};
		TransformComponent transform{};
		
		//Optional pointer components
		std::shared_ptr<GenModel>model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;
		std::unique_ptr<SoundSphereComponent> soundSphere = nullptr;
		std::unique_ptr<SoundDiscComponent> soundDisc = nullptr;
		std::unique_ptr<NodeComponent> node = nullptr;

		bool textureDirty = false;
		std::shared_ptr<GenTexture> texture{};

		ObjectType type = ObjectType::Generic;

	private:
		GenGameObject(id_t objId) : id{ objId } {}
		id_t id;



	};

	
}