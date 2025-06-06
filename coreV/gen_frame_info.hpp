#pragma once

#include "../gen_camera.hpp"
#include "../gen_game_object.hpp"
#include <vulkan/vulkan.h>


namespace gen {

#define MAX_LIGHTS 100

	struct PointLight {

		glm::vec4 position{};
		glm::vec4 color{};


	};

	struct GlobalUbo {

		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f,1.f,1.f,.02f };
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
		alignas(16) glm::vec3 padding;

		//glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f,-3.f,-1.f });
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		GenCamera& camera;
		VkDescriptorSet globalDescriptorSet;  // for lighting, etc.
		std::unordered_map<GenGameObject::id_t, VkDescriptorSet>& objectDescriptorSets;  // for textured objects
		GenGameObject::Map& gameObjcets;
	};
	


}