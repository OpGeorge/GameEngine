#pragma once

#include "gen_model.hpp"

#include<glm/gtc/matrix_transform.hpp>
#include <memory>

namespace gen {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f,1.f,1.f };
		glm::vec3 rotation;

		// Matrix corresponds to translate * Ry * Rx * Rz * scale 
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};


	class GenGameObject {

	public:
		using id_t = unsigned int;

		static GenGameObject createGameObject() {
			static id_t currentId = 0;
			return GenGameObject{ currentId++ };
		}

		GenGameObject(const GenGameObject&) = delete;
		GenGameObject& operator=(const GenGameObject&) = delete;
		GenGameObject(GenGameObject &&) = default;
		GenGameObject& operator=(GenGameObject&&) = default;





		const id_t getId() { return id; }

		std::shared_ptr<GenModel>model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		GenGameObject(id_t objId) : id{ objId } {}
		id_t id;

	};
}