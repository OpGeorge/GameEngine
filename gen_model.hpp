#pragma once

#include "gen_device.hpp"

#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace gen {

	class GenModel {

	public:
		
		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		};

		struct Builder {

			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		GenModel(GenDevice &device,const GenModel::Builder &builder);
		~GenModel();

		GenModel(const GenModel&) = delete;
		GenModel &operator=(const GenModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);


	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		GenDevice& genDevice;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		bool hasIndexBuffer = false;


	};

}