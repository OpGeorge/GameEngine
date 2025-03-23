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
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f,-3.f,-1.f });
	};


	AppCtrl::AppCtrl() {
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

				int frameIndex = genRenderer.getFrameIndex();
				FrameInfo farmeInfo{frameIndex,frameTime,commandBuffer,camera};


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

		auto gameObj = GenGameObject::createGameObject();
		gameObj.model = genModel;
		gameObj.transform.translation = {.0f,.0f, 2.5f};
		gameObj.transform.scale = glm::vec3(3.f);

		gameObjects.push_back(std::move(gameObj));

	}

	
	

	
}