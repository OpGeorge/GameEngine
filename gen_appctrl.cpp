#include "gen_appctrl.hpp"
#include "gen_buffer.hpp"

#include "wireframe_render_system.hpp"
#include "gen_camera.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
#include "keyboard_movement_controller.hpp"
#include "skybox_render_system.hpp"

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

      

        const int maxObjects = static_cast<int>(gameObjects.size()) + 10;  // Add padding
        globalPool = GenDescriptorPool::Builder(genDevice)
            .setMaxSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT * maxObjects)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GenSwapChain::MAX_FRAMES_IN_FLIGHT * maxObjects)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GenSwapChain::MAX_FRAMES_IN_FLIGHT * maxObjects)
			.build();

		
	

	
	}

	AppCtrl::~AppCtrl() {
		

	}

    void AppCtrl::run() {
        std::shared_ptr<GenTexture> fallbackTexture = std::make_shared<GenTexture>(genDevice, "textures/white.png");

        std::vector<std::unique_ptr<GenBuffer>> uboBuffers(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>> textureToggleBuffers;

        for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            uboBuffers[i] = std::make_unique<GenBuffer>(
                genDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map();

           
        }

        auto globalSetLayout = GenDescriptorSetLayout::Builder(genDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT> objectDescriptorSets;

        for (int frameIndex = 0; frameIndex < GenSwapChain::MAX_FRAMES_IN_FLIGHT; frameIndex++) {
            for (auto& [id, obj] : gameObjects) {
                VkDescriptorSet descriptorSet;

                // UBO for projection/view
                VkDescriptorBufferInfo uboInfo = uboBuffers[frameIndex]->descriptorInfo();

                // Texture binding logic
                int useTextureFlag = 1;
                bool bindTexture = true;
                VkDescriptorImageInfo imageInfo{};

                if (obj.texture) {
                    imageInfo = obj.texture->descriptorInfo();
                    useTextureFlag = 1;
                }
                else if (obj.soundSphere) {
                    
                    imageInfo = fallbackTexture->descriptorInfo();
                    useTextureFlag = 0;
                }
                else {
                    imageInfo = fallbackTexture->descriptorInfo();
                    useTextureFlag = 1;
                }

                // Allocate per-object texture toggle buffer if needed
                if (textureToggleBuffers.find(id) == textureToggleBuffers.end()) {
                    std::vector<std::unique_ptr<GenBuffer>> buffers;

                    for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
                        auto buffer = std::make_unique<GenBuffer>(
                            genDevice,
                            sizeof(int),
                            1,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        );
                        buffer->map();
                        buffers.push_back(std::move(buffer));
                    }

                    textureToggleBuffers.emplace(id, std::move(buffers));
                }

                // Write the flag for this object for this frame
                textureToggleBuffers[id][frameIndex]->writeToBuffer(&useTextureFlag);
                VkDescriptorBufferInfo flagInfo = textureToggleBuffers[id][frameIndex]->descriptorInfo();

                // Build descriptor set
                auto writer = GenDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &uboInfo)
                    .writeImage(1, &imageInfo)         
                    .writeBuffer(2, &flagInfo);        // tell shader whether to sample

                if (!writer.build(descriptorSet)) {
                    throw std::runtime_error("Failed to allocate descriptor set");
                }

                objectDescriptorSets[frameIndex][id] = descriptorSet;

                if (globalDescriptorSets[frameIndex] == VK_NULL_HANDLE) {
                    globalDescriptorSets[frameIndex] = descriptorSet;
                }
            }
        }

        SimpleRenderSystem simpleRenderSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        WireframeRenderSystem wireframeRenderSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        GenCamera camera{};
        camera.setViewDirection(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto currentTime = std::chrono::high_resolution_clock::now();

        auto viewerObject = GenGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        viewerObject.transform.translation.y = -.5f;

        std::vector<GenGameObject> cameraStand;
        {
            auto cam1 = GenGameObject::createGameObject();
            cam1.type = ObjectType::Camera;
            cam1.transform.translation = { 0.f, -0.3f, 1.f };

            auto cam2 = GenGameObject::createGameObject();
            cam2.type = ObjectType::Camera;
            cam2.transform.translation = { 0.f, -.3f, -2.5f };
            cam2.transform.rotation = { 0.f, -glm::radians(10.f), 0.f };

            auto cam3 = GenGameObject::createGameObject();
            cam3.transform.translation = { -5.f, -.5f, -5.f };
            cam3.type = ObjectType::Camera;

            cameraStand.push_back(std::move(cam1));
            cameraStand.push_back(std::move(cam2));
            cameraStand.push_back(std::move(cam3));
        }

        int activeCameraIndex = 0;
        KeyboardMovementController cameraController{};
        const float MAX_FRAME_TIME = 165.f;

        while (!genWindow.shouldClose()) {
            glfwPollEvents();

            static bool tabPressedLastFrame = false;
            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_TAB) == GLFW_PRESS) {
                if (!tabPressedLastFrame) {
                    activeCameraIndex = (activeCameraIndex + 1) % cameraStand.size();
                }
                tabPressedLastFrame = true;
            }
            else {
                tabPressedLastFrame = false;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.processMouseLookToggle(genWindow.getGLFWwindow());
            cameraController.updateCameraViewFromMouse(genWindow.getGLFWwindow(), cameraStand[activeCameraIndex]);
            cameraController.moveInPlaneXZ(genWindow.getGLFWwindow(), frameTime, cameraStand[activeCameraIndex]);

            camera.setViewYXZ(cameraStand[activeCameraIndex].transform.translation, cameraStand[activeCameraIndex].transform.rotation);
            float aspect = genRenderer.getAspectratio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            logicManager.update(frameTime, gameObjects);

            if (auto commandBuffer = genRenderer.beginFrame()) {
                int frameIndex = genRenderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    objectDescriptorSets[frameIndex],
                    gameObjects
                };

                GlobalUbo ubo{};
                ubo.projection = camera.getProjcetion();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                genRenderer.beginSwachChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                wireframeRenderSystem.render(frameInfo);
                
                genRenderer.endSwachChainRenderPass(commandBuffer);
                genRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(genDevice.device());
    }



	void AppCtrl::loadGameObjects() {



        std::shared_ptr<GenTexture> texture = std::make_shared<GenTexture>(genDevice, "textures/vaseTexture.png");
	
		std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/smooth_vase.obj");
		auto smoothVase = GenGameObject::createGameObject();
		smoothVase.model = genModel;
        smoothVase.texture = texture;
        smoothVase.type = ObjectType::Player;
        
        smoothVase.soundDisc = std::make_unique<SoundDiscComponent>();
        smoothVase.soundDisc->radius = 1.0f;
        smoothVase.soundDisc->visible = true;
        smoothVase.soundDisc->isPlayerControlled = true;

		smoothVase.transform.translation = {-1.0f,-2.0f, 0.f};
		smoothVase.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(smoothVase.getId(),std::move(smoothVase));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/flat_vase.obj");
		auto flatVase = GenGameObject::createGameObject();
		flatVase.model = genModel;
		flatVase.transform.translation = {1.f,0.0f,0.f };
		flatVase.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(flatVase.getId(), std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
		auto coloredCube = GenGameObject::createGameObject();
		coloredCube.model = genModel;
		coloredCube.transform.translation = { 0.f,-.5f,0.f };
		coloredCube.transform.scale = glm::vec3(0.5f);
		gameObjects.emplace(coloredCube.getId(),std::move(coloredCube));

		genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
		auto surface = GenGameObject::createGameObject();
		surface.model = genModel;
		surface.transform.translation = { 0.f,0.0f,0.f };
		surface.transform.scale = glm::vec3(3.f);
		gameObjects.emplace(surface.getId(),std::move(surface));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere = GenGameObject::createGameObject();
        sphere.model = genModel;
        sphere.color = { 1.0f, 0.0f, 0.0f };
        sphere.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere.soundDisc->radius = .5f;
        sphere.soundDisc->visible = true;
        sphere.soundDisc->isPlayerControlled = true;
        sphere.transform.translation = { -1.f,-0.5f,-2.5f };
        sphere.transform.scale = glm::vec3(.1f);
        sphere.type = ObjectType::Sphere;
        gameObjects.emplace(sphere.getId(), std::move(sphere));




		{ // lumina alba din cub
			auto pointLight = GenGameObject::makePointLight(0.2f);
            pointLight.transform.translation = glm::vec3{ .0f,-.5f,.0f };
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
            pointLight.type = ObjectType::Light;
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f,-1.f,0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}

	}

	
	

	
}