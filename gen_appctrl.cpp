#include "gen_appctrl.hpp"
#include "gen_buffer.hpp"


#include "gen_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"



#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>


namespace gen {

	struct GlobalUbo {

		glm::mat4 projectionView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f,1.f,1.f,.02f };
		glm::vec3 lightPotision{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };

		//glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f,-3.f,-1.f });
	};


	AppCtrl::AppCtrl() {
		globalPool = GenDescriptorPool::Builder(genDevice)
			.setMaxSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GenSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	

	
	}

	AppCtrl::~AppCtrl() {
		

	}

	void AppCtrl::run() {

		
		std::vector<std::unique_ptr<GenBuffer>> uboBuffers(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<GenBuffer>(genDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  );

			uboBuffers[i]->map();
		}

		auto globalSetLayout = GenDescriptorSetLayout::Builder(genDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i< globalDescriptorSets.size(); i++) {
			
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			GenDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0,&bufferInfo).build(globalDescriptorSets[i]);
		
		}

		SimpleRenderSystem simpleRenderSystem{ genDevice, genRenderer.getSwapChainRenderPass(), globalSetLayout ->getDescriptorSetLayout()};

		GenCamera camera{};

		const float  MAX_FRAME_TIME = 165;

		camera.setViewDirection(glm::vec3(0.0f,-0.5f,0.0f), glm::vec3(0.0f, 0.0f, 2.5f));
		
		auto currentTime = std::chrono::high_resolution_clock::now();

		auto viewerObject = GenGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		
		while (!genWindow.shouldClose()) {
			
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();

			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

			currentTime = newTime;

			frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			cameraController.moveInPlaneXZ(genWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);


			float aspect = genRenderer.getAspectratio();
			//camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = genRenderer.beginFrame()) {

				int frameIndex = genRenderer.getFrameIndex();
				FrameInfo farmeInfo{frameIndex,frameTime,commandBuffer,camera,globalDescriptorSets[frameIndex]};


				//update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjcetion() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();
				 




				//render
				genRenderer.beginSwachChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjcets(farmeInfo,gameObjects);
				genRenderer.endSwachChainRenderPass(commandBuffer);
				genRenderer.endFrame();
			}

		
		}
		vkDeviceWaitIdle(genDevice.device());
	}

	//std::unique_ptr<GenModel> createCubeModel(GenDevice& device, glm::vec3 offset) {

	//	GenModel::Builder modelBuilder{};
	//	modelBuilder.vertices = {

	//		// left face (white)
	//		{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
	//		{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
	//		{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
	//		{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
	//		{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
	//		{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

	//		// right face (yellow)
	//		{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
	//		{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
	//		{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
	//		{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
	//		{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
	//		{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

	//		// top face (orange, remember y axis points down)
	//		{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
	//		{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	//		{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	//		{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
	//		{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
	//		{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

	//		// bottom face (red)
	//		{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
	//		{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
	//		{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
	//		{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
	//		{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
	//		{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

	//		// nose face (blue)
	//		{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
	//		{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	//		{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	//		{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
	//		{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
	//		{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

	//		// tail face (green)
	//		{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	//		{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	//		{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	//		{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	//		{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	//		{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

	//	};
	//	for (auto& v : modelBuilder.vertices) {
	//		v.position += offset;
	//	}
	//	return std::make_unique<GenModel>(device, modelBuilder);
	//}


	

	void AppCtrl::loadGameObjects() {
	
		std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/smooth_vase.obj");
		auto smoothVase = GenGameObject::createGameObject();
		smoothVase.model = genModel;
		smoothVase.transform.translation = {-1.0f,.5f, 0.f};
		smoothVase.transform.scale = glm::vec3(3.f);
		gameObjects.push_back(std::move(smoothVase));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/flat_vase.obj");
		auto flatVase = GenGameObject::createGameObject();
		flatVase.model = genModel;
		flatVase.transform.translation = {1.f,.5f,0.f };
		flatVase.transform.scale = glm::vec3(3.f);
		gameObjects.push_back(std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
		auto coloredCube = GenGameObject::createGameObject();
		coloredCube.model = genModel;
		coloredCube.transform.translation = { 0.f,.0f,0.f };
		coloredCube.transform.scale = glm::vec3(0.5f);
		gameObjects.push_back(std::move(coloredCube));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
		auto surface = GenGameObject::createGameObject();
		surface.model = genModel;
		surface.transform.translation = { 0.f,.5f,0.f };
		surface.transform.scale = glm::vec3(3.f);
		gameObjects.push_back(std::move(surface));


	}

	
	

	
}