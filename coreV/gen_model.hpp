#pragma once

#include "gen_device.hpp"
#include "gen_buffer.hpp"


#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace gen {

	class GenModel {

	public:
		
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}

		};

		struct Builder {

			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);

		};

		GenModel(GenDevice &device,const GenModel::Builder &builder);
		~GenModel();

		GenModel(const GenModel&) = delete;
		GenModel &operator=(const GenModel&) = delete;

		static std::unique_ptr<GenModel> createModelFromFile(GenDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);


	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		GenDevice& genDevice;

		std::unique_ptr<GenBuffer> vertexBuffer;
		uint32_t vertexCount;

		std::unique_ptr<GenBuffer> indexBuffer; 
		uint32_t indexCount;

		bool hasIndexBuffer = false;


	};

}