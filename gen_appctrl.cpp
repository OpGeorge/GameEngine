#include "gen_appctrl.hpp"
#include "gen_buffer.hpp"


#include "gen_camera.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
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
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i< globalDescriptorSets.size(); i++) {
			
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			GenDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0,&bufferInfo).build(globalDescriptorSets[i]);
		
		}

		SimpleRenderSystem simpleRenderSystem{ genDevice, genRenderer.getSwapChainRenderPass(), globalSetLayout ->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{ genDevice, genRenderer.getSwapChainRenderPass(), globalSetLayout ->getDescriptorSetLayout()};

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
				FrameInfo frameInfo{frameIndex,frameTime,commandBuffer,camera,globalDescriptorSets[frameIndex],gameObjects};


				//update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjcetion();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();
				 




				//render
				genRenderer.beginSwachChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjcets(frameInfo);
				pointLightSystem.render(frameInfo);
				genRenderer.endSwachChainRenderPass(commandBuffer);
				genRenderer.endFrame();
			}

		
		}
		vkDeviceWaitIdle(genDevice.device());
	}

	void AppCtrl::loadGameObjects() {
	
		std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/smooth_vase.obj");
		auto smoothVase = GenGameObject::createGameObject();
		smoothVase.model = genModel;
		smoothVase.transform.translation = {-1.0f,.5f, 0.f};
		smoothVase.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(smoothVase.getId(),std::move(smoothVase));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/flat_vase.obj");
		auto flatVase = GenGameObject::createGameObject();
		flatVase.model = genModel;
		flatVase.transform.translation = {1.f,.5f,0.f };
		flatVase.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(flatVase.getId(), std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
		auto coloredCube = GenGameObject::createGameObject();
		coloredCube.model = genModel;
		coloredCube.transform.translation = { 0.f,.0f,0.f };
		coloredCube.transform.scale = glm::vec3(0.5f);
		gameObjects.emplace(coloredCube.getId(),std::move(coloredCube));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
		auto surface = GenGameObject::createGameObject();
		surface.model = genModel;
		surface.transform.translation = { 0.f,.5f,0.f };
		surface.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(surface.getId(),std::move(surface));

		{
			auto pointLight = GenGameObject::makePointLight(0.2f);
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}


		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  //
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = GenGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f,-1.f,0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}

	}

	
	

	
}