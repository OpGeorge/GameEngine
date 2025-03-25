#pragma once

#include "gen_model.hpp"

#include<glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace gen {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f,1.f,1.f };
		glm::vec3 rotation;

		// Matrix corresponds to translate * Ry * Rx * Rz * scale 
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};


	struct PointLightComponent {

		float lightIntesity = 1.0f;

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

	private:
		GenGameObject(id_t objId) : id{ objId } {}
		id_t id;

	};
}