#include "gen_appctrl.hpp"

#include "coreV/gen_buffer.hpp"
#include "coreV/wireframe_render_system.hpp"
#include "coreV/simple_render_system.hpp"
#include "coreV/point_light_system.hpp"
#include "coreV/gen_texture_swapper.hpp"

#include "gen_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "gen_obj_movement.hpp"
#include "gen_npc_controller.hpp"




#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <iostream>
#include <queue>



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

        bool textureSwapPending = false;
        std::string pendingTexturePath;

        std::queue<std::tuple<int, GenGameObject*, std::shared_ptr<GenTexture>, std::shared_ptr<GenTexture>>> pendingTextureSwaps;
        std::queue<std::pair<int, std::shared_ptr<GenTexture>>> textureGarbageQueue;

        int currentFrameNumber = 0;

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
                const bool hasRealTexture = obj.texture != nullptr;
                VkDescriptorImageInfo imageInfo = hasRealTexture
                    ? obj.texture->descriptorInfo()
                    : fallbackTexture->descriptorInfo();

                int useTextureFlag = hasRealTexture ? 1 : 0;

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
                    std::cerr << "Descriptor set allocation failed. Pool may be exhausted.\n";

                    // Safely destroy old pool and build a new one
                    globalPool = GenDescriptorPool::Builder(genDevice)
                        .setMaxSets(1000)  // or increase this number
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                        .build();

                    auto retryWriter = GenDescriptorWriter(*globalSetLayout, *globalPool)
                        .writeBuffer(0, &uboInfo)
                        .writeImage(1, &imageInfo)
                        .writeBuffer(2, &flagInfo);

                    if (!retryWriter.build(descriptorSet)) {
                        throw std::runtime_error("Failed to rebuild descriptor set even after pool reset");
                    }
                }

                objectDescriptorSets[frameIndex][id] = descriptorSet;

               

                if (globalDescriptorSets[frameIndex] == VK_NULL_HANDLE) {
                    globalDescriptorSets[frameIndex] = descriptorSet;
                }
            }
        }

        TextureSwapper textureSwapper(
            genDevice,
            *globalSetLayout,
            *globalPool,
            uboBuffers,
            textureToggleBuffers,
            objectDescriptorSets);

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

        std::vector<GenGameObject*> controllableObjects;
        std::vector<GenGameObject*> controllableNPCs;

        for (auto& [id, obj] : gameObjects) {
            if (obj.type == ObjectType::Player) {
                controllableObjects.push_back(&obj);
                std::cout << controllableObjects.size() << "\n";
            }
            if (obj.type == ObjectType::NPC) {
                controllableNPCs.push_back(&obj);
            }
        }
       
        GameObjectMovementController objectController;
 

        //test harcoded target
        static glm::vec3 target = { 2.0f,0.0f,-3.0f };
        NPCMovementController npcController;


        int activeCameraIndex = 0;
        int activeObjectIndex = 0;
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

            npcController.moveToTarget(frameTime, *controllableNPCs[0], target, 0.5f);

            
            static bool switchPressedLastFrame = false;
            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_C) == GLFW_PRESS) {
                
                if (!switchPressedLastFrame) {
                    
                    
                    activeObjectIndex = (activeObjectIndex +1) % controllableObjects.size();
                    std::cout << "Now controlling object index: " << activeObjectIndex << "\n";
                }
                switchPressedLastFrame = true;
            }
            else {
                switchPressedLastFrame = false;
            }

            static bool textureSwapPressedLastFrame = false;

            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_T) == GLFW_PRESS) {
                if (!textureSwapPressedLastFrame && !controllableObjects.empty()) {
                    GenGameObject* activeObj = controllableObjects[activeObjectIndex];
                    auto newTex = getCachedTexture("textures/red.png");
                    if (activeObj->texture != newTex) {
                        pendingTextureSwaps.push({ currentFrameNumber, activeObj, newTex, activeObj->texture });
                        std::cout << "[DELAYED] Queued texture change for object " << activeObj->getId() << "\n";
                    }
                }
                textureSwapPressedLastFrame = true;
            }
            else {
                textureSwapPressedLastFrame = false;
            }

            if (!controllableObjects.empty()) {
                GenGameObject* activeObj = controllableObjects[activeObjectIndex];

                // Always move the object
                objectController.moveInPlaneXZ(genWindow.getGLFWwindow(), frameTime, *activeObj);

                // Only allow Player-type to emit sound and trigger propagation
                if (activeObj->type == ObjectType::Player) {
                    updateNodeColorAndTextureFromPlayer(
                        *activeObj,
                        gameObjects,
                        getCachedTexture("textures/red.png"),
                        getCachedTexture("textures/orange.png"),
                        getCachedTexture("textures/green.png"),
                        currentFrameNumber,
                        pendingTextureSwaps
                    );

                    for (auto& [id, obj] : gameObjects) {
                        if (obj.type != ObjectType::Node || !obj.node || !obj.node->activated) continue;
                        if (!obj.node->hasPropagated &&  obj.node->color == NodeComponent::NodeColor::Red ||
                            obj.node->color == NodeComponent::NodeColor::Orange) {
                            propagationSystem.propagateFromNode(
                                obj,
                                gameObjects,
                                3.5f,
                                getCachedTexture("textures/orange.png"),
                                getCachedTexture("textures/green.png"),
                                currentFrameNumber,
                                pendingTextureSwaps
                            );

                            obj.node->hasPropagated = true;

                        }
                    }
                }
            }
           
            
            if (auto commandBuffer = genRenderer.beginFrame()) {
                
                int frameIndex = genRenderer.getFrameIndex();

                while (!pendingTextureSwaps.empty()) {
                    auto [frameAssigned, objPtr, newTex, oldTex] = pendingTextureSwaps.front();
                    if (currentFrameNumber - frameAssigned >= GenSwapChain::MAX_FRAMES_IN_FLIGHT * 2) {
                        textureSwapper.swapTexture(*objPtr, newTex, frameIndex);
                        textureGarbageQueue.push({ currentFrameNumber, oldTex });
                        pendingTextureSwaps.pop();
                    }
                    else {
                        break;
                    }
                }

                refreshObjectDescriptorsIfNeeded(
                    fallbackTexture,
                    uboBuffers,
                    textureToggleBuffers,
                    objectDescriptorSets,
                    *globalSetLayout,
                    *globalPool);

                while (!textureGarbageQueue.empty()) {
                    const auto& [frameAssigned, tex] = textureGarbageQueue.front();
                    if (currentFrameNumber - frameAssigned >= GenSwapChain::MAX_FRAMES_IN_FLIGHT + 2) {
                        textureGarbageQueue.pop();  // Now it's safe to destroy
                    }
                    else {
                        break;
                    }
                }

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

                currentFrameNumber++;

                


            
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
        flatVase.type = ObjectType::NPC;
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
        sphere.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere.soundDisc->radius = .5f;
        sphere.soundDisc->visible = true;
        sphere.soundDisc->isPlayerControlled = true;
        sphere.transform.translation = { -1.f,-0.5f,-2.5f };
        sphere.transform.scale = glm::vec3(.1f);
        sphere.type = ObjectType::Node;
        sphere.node = std::make_unique<NodeComponent>();
        sphere.node->selfPosition = sphere.transform.translation;
        gameObjects.emplace(sphere.getId(), std::move(sphere));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere2 = GenGameObject::createGameObject();
        sphere2.model = genModel;
        sphere2.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere2.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere2.soundDisc->radius = .5f;
        sphere2.soundDisc->visible = true;
        sphere2.soundDisc->isPlayerControlled = true;
        sphere2.transform.translation = { -3.f,-0.5f,-2.5f };
        sphere2.transform.scale = glm::vec3(.1f);
        sphere2.type = ObjectType::Node;
        sphere2.node = std::make_unique<NodeComponent>();
        sphere2.node->selfPosition = sphere2.transform.translation;

        gameObjects.emplace(sphere2.getId(), std::move(sphere2));

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


    void AppCtrl::refreshObjectDescriptorsIfNeeded(
        std::shared_ptr<GenTexture> fallbackTexture,
        std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets,
        GenDescriptorSetLayout& globalSetLayout,
        GenDescriptorPool& globalPool
    ) {
        for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            for (auto& [id, obj] : gameObjects) {
                if (!obj.textureDirty) continue;

                // Prevent fallback flickering: skip rebuild if texture not yet assigned
                if (!obj.texture) {
                    std::cerr << "[SKIP] Texture is still null for ID " << id << " — deferring descriptor rebuild.\n";
                    continue;
                }

                VkDescriptorSet descriptorSet;
                VkDescriptorBufferInfo uboInfo = uboBuffers[i]->descriptorInfo();

                int useTextureFlag = 1;  // texture is guaranteed valid
                VkDescriptorImageInfo imageInfo = obj.texture->descriptorInfo();

                if (textureToggleBuffers.find(id) == textureToggleBuffers.end()) {
                    std::vector<std::unique_ptr<GenBuffer>> buffers;
                    for (int j = 0; j < GenSwapChain::MAX_FRAMES_IN_FLIGHT; j++) {
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

                textureToggleBuffers[id][i]->writeToBuffer(&useTextureFlag);
                VkDescriptorBufferInfo flagInfo = textureToggleBuffers[id][i]->descriptorInfo();

                auto writer = GenDescriptorWriter(globalSetLayout, globalPool)
                    .writeBuffer(0, &uboInfo)
                    .writeImage(1, &imageInfo)
                    .writeBuffer(2, &flagInfo);

                if (!writer.build(descriptorSet)) {
                    throw std::runtime_error("Failed to rebuild descriptor set");
                }

                objectDescriptorSets[i][id] = descriptorSet;
            }
        }

        for (auto& [id, obj] : gameObjects) {
            if (obj.texture) {
                obj.textureDirty = false;
            }
        }
    }

    std::shared_ptr<GenTexture> AppCtrl::getCachedTexture(const std::string& path) {
        auto it = textureCache.find(path);
        if (it != textureCache.end()) {
            return it->second;
        }

        auto newTexture = std::make_shared<GenTexture>(genDevice, path);
        textureCache[path] = newTexture;
        return newTexture;
    }

    void AppCtrl::updateNodeColorAndTextureFromPlayer(
        GenGameObject& player,
        GenGameObject::Map& gameObjects,
        const std::shared_ptr<GenTexture>& redTexture,
        const std::shared_ptr<GenTexture>& orangeTexture,
        const std::shared_ptr<GenTexture>& greenTexture,
        int currentFrameNumber,
        std::queue<std::tuple<int, GenGameObject*, std::shared_ptr<GenTexture>, std::shared_ptr<GenTexture>>>& pendingTextureSwaps)
    {
        GLFWwindow* window = genWindow.getGLFWwindow();
        if (!player.soundDisc) return;

        // Simple key press detection
        const std::vector<int> moveKeys = { GLFW_KEY_I, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L };
        bool isMovingKeyPressed = false;
        for (int key : moveKeys) {
            if (glfwGetKey(window, key) == GLFW_PRESS) {
                isMovingKeyPressed = true;
                break;
            }
        }

        if (!isMovingKeyPressed) return; // no input, no sound

        // Check modifiers
        bool loud = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        bool quiet = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

        NodeComponent::NodeColor targetColor;
        std::shared_ptr<GenTexture> selectedTexture;
        float soundMultiplier = 1.0f;

        if (loud) {
            targetColor = NodeComponent::NodeColor::Red;
            selectedTexture = redTexture;
            soundMultiplier = 2.0f;
        }
        else if (quiet) {
            targetColor = NodeComponent::NodeColor::Green;
            selectedTexture = greenTexture;
            soundMultiplier = 0.5f;
        }
        else {
            targetColor = NodeComponent::NodeColor::Orange;
            selectedTexture = orangeTexture;
            soundMultiplier = 1.0f;
        }

        glm::vec2 playerCenter = { player.transform.translation.x, player.transform.translation.z };
        float playerRadius = player.soundDisc->radius * soundMultiplier;

        for (auto& [id, obj] : gameObjects) {
            if (obj.type != ObjectType::Node || !obj.node || !obj.soundDisc) continue;

            glm::vec2 nodeCenter = { obj.transform.translation.x, obj.transform.translation.z };
            float nodeRadius = obj.soundDisc->radius;
            float dist = glm::distance(playerCenter, nodeCenter);
            float combinedRadius = playerRadius + nodeRadius;
            if (dist <= combinedRadius) {
                if (obj.node->color != targetColor) {
                    obj.node->color = targetColor;
                    obj.node->activated = true;
                    obj.node->selfPosition = obj.transform.translation;

                    //only apply new texture if color changed from last time
                    if (obj.node->lastColorApplied != targetColor) {
                        obj.node->lastColorApplied = targetColor;

                        obj.texture = selectedTexture;
                        obj.textureDirty = true;

                        std::cout << "[SOUND] Node " << id << " changed to "
                            << static_cast<int>(targetColor) << "\n";
                    }

                }

                
            }
        }
    }


}