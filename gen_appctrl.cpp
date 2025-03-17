#include "gen_appctrl.hpp"


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




	AppCtrl::AppCtrl() {
		loadGameObjects();
	

	
	}

	AppCtrl::~AppCtrl() {
		

	}

	void AppCtrl::run() {

		SimpleRenderSystem simpleRenderSystem{ genDevice, genRenderer.getSwapChainRenderPass() };

		GenCamera camera{};

		const float  MAX_FRAME_TIME = 165;

		camera.setViewDirection(glm::vec3(0.0f,-0.5f,0.0f), glm::vec3(0.0f, 0.0f, 2.5f));
		
		auto currentTime = std::chrono::high_resolution_clock::now();

		auto viewerObject = GenGameObject::createGameObject();
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

				genRenderer.beginSwachChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjcets(commandBuffer,gameObjects,camera);
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


	std::unique_ptr<GenModel> createCubeModel(GenDevice& device, glm::vec3 offset) {
		GenModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<GenModel>(device, modelBuilder);
	}

	void AppCtrl::loadGameObjects() {
	
		std::shared_ptr<GenModel> genModel = createCubeModel(genDevice, { .0f,.0f,.0f });

		auto cube = GenGameObject::createGameObject();
		cube.model = genModel;
		cube.transform.translation = {.0f,.0f, 2.5f};
		cube.transform.scale = { .5f,.5f,.5f };

		gameObjects.push_back(std::move(cube));

	}

	
	

	
}